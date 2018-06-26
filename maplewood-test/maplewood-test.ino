/*

Module:  maplewood-test.ino

Function:
        Test app for Catena-4470 as used at Maplewood.

Copyright notice and License:
        See LICENSE file accompanying this project.

Author:
        Terry Moore, MCCI Corporation	April 2018

*/

// line number should be l# - 2, for ArduinoUnit compatibility
#line 2 "maplewood-test.ino"

#include <Arduino.h>
#include <Catena4470.h>
#include <Catena_led.h>
#include <SPI.h>
#include <wiring_private.h>
#include <Catena_Flash_at25sf081.h>
#include <Catena_Guids.h>
#include <RTCZero.h>

#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <ModbusRtu.h>
#include <MCCI-WattNode-Modbus-Registers.h>

#include <ArduinoUnit.h>

using namespace McciCatena;
using Catena = Catena4470;

Catena gCatena;
cFlash_AT25SF081 gFlash;

// declare lorawan (so we can provision)
Catena::LoRaWAN gLoRaWAN;

// declare the LED object
StatusLed gLed (Catena::PIN_STATUS_LED);

BH1750 gBH1750;
Adafruit_BME280 gBME280;

// SCK is D12, which is SERCOM1.3
// MOSI is D11, which is SERCOM1.0
// MISO is D10, which is SERCOM1.2
// Knowing that, you might be able to make sens of the following:
SPIClass gSPI2(&sercom1, 10, 12, 11, SPI_PAD_0_SCK_3, SERCOM_RX_PAD_2);

/**
 *  Modbus object declaration
 *  u8id : node id = 0 for host, = 1..247 for device
 *  u8txenpin : 0 for RS-232 and USB-FTDI 
 *               or any pin number > 1 for RS-485
 */
Modbus gModbusHost(0, A4); // this is host; RX485.
ModbusSerial<decltype(Serial1)> gModbusSerial(&Serial1);

bool fFlashDone = false;

bool flash_init(void);
void setup_flash(void);
void setup_platform(void);

void setup()
	{
	gCatena.begin();

        // Test::exclude("*");
        // Test::include("*modbus*");
        // fFlashDone = true;

	setup_platform();

	//** set up for flash work **
	setup_flash();
	}

void loop()
	{
        gCatena.poll();
	Test::run();
	}

void setup_platform()
	{
	while (! Serial)
		/* wait for USB attach */
		yield();

        Serial.print(
                "\n" 
                "-------------------------------------------------------------------------------\n"
                "This is the Maplewood test program.\n"
                "Enter 'help' for a list of commands.\n"
                "(remember to select 'Line Ending: Newline' at the bottom of the monitor window.)\n"
                "--------------------------------------------------------------------------------\n"
                "\n"
                );

        gLed.begin();
        gCatena.registerObject(&gLed);
        gLed.Set(LedPattern::FastFlash);

	Catena::UniqueID_string_t CpuIDstring;

        gCatena.SafePrintf(
		"CPU Unique ID: %s\n",
                gCatena.GetUniqueIDstring(&CpuIDstring)
                );
	}

//-----------------------------------------------------
// platform tests
//-----------------------------------------------------

test(2_platform_05_check_platform_guid)
	{
        const CATENA_PLATFORM * const pPlatform = gCatena.GetPlatform();
	const MCCIADK_GUID_WIRE m101Guid = GUID_HW_CATENA_4470_M101(WIRE);

	assertTrue(pPlatform != nullptr, "gCatena.GetPlatform() failed -- this is normal on first boot");
	assertEqual(
		memcmp(&m101Guid, &pPlatform->Guid, sizeof(m101Guid)), 0,
		"platform guid mismatch"
		);
	}

test(2_platform_10_check_syseui)
	{
        const Catena::EUI64_buffer_t *pSysEUI = gCatena.GetSysEUI();
	static const Catena::EUI64_buffer_t ZeroEUI = { 0 };
	bool fNonZeroEUI;

	assertTrue(pSysEUI != nullptr);
	fNonZeroEUI = memcmp(pSysEUI, &ZeroEUI, sizeof(ZeroEUI)) != 0;
	assertTrue(fNonZeroEUI, "SysEUI is zero. This is normal on first boot.");

	for (unsigned i = 0; i < sizeof(pSysEUI->b); ++i)
		{
		gCatena.SafePrintf("%s%02x", i == 0 ? "  SysEUI: " : "-", pSysEUI->b[i]);
		}
	gCatena.SafePrintf("\n");
	}

test(2_platform_20_lorawan_begin)
	{
	bool fPassed = gLoRaWAN.begin(&gCatena);

	// always register
	gCatena.registerObject(&gLoRaWAN);

	assertTrue(fPassed, "gLoRaWAN.begin() failed");
	}

test(2_platform_30_init_lux) 
	{
	uint32_t flags = gCatena.GetPlatformFlags();

	assertNotEqual(flags & Catena::fHasLuxRohm, 0, "No lux sensor in platform flags?");

	gBH1750.begin();
	gBH1750.configure(BH1750_CONTINUOUS_HIGH_RES_MODE_2);
	}

test(2_platform_40_init_bme)
	{
	uint32_t flags = gCatena.GetPlatformFlags();

	assertNotEqual(flags & Catena::fHasBme280, 0, "No BME280 sensor in platform flags?");
	assertTrue(
		gBME280.begin(BME280_ADDRESS, Adafruit_BME280::OPERATING_MODE::Sleep),
		"BME280 sensor failed begin(): check wiring"
		);
	}

constexpr uint8_t kModbusPowerOn = A3;

void modbusPowerOn(void)
	{
        pinMode(kModbusPowerOn, OUTPUT);
        digitalWrite(kModbusPowerOn, HIGH);
	}

test(2_platform_80_modbus_init)
	{
	uint32_t flags = gCatena.GetPlatformFlags();

	assertNotEqual(flags & Catena::fHasRS485, 0);

	modbusPowerOn();
	gModbusHost.begin(&gModbusSerial, 19200);
	
	gModbusHost.setTimeOut(1000);
	gModbusHost.setTxEnableDelay(50);
	}

testing(3_platform_50_lux)
	{
	assertTestPass(2_platform_30_init_lux);

	const uint32_t interval = 2000;
	const uint32_t ntries = 10;
	static uint32_t lasttime, thistry;
	uint32_t now = millis();

	if ((int32_t)(now - lasttime) < interval)
		/* skip */;
	else
		{
		lasttime = now;

		uint16_t light;

		light = gBH1750.readLightLevel();
		assertLess(light, 0xFFFF / 1.2, "Oops: light value pegged: " << light);
		gCatena.SafePrintf("BH1750:  %u lux\n", light);

		if (++thistry >= ntries)
			pass();
 		}
	}

testing(3_platform_60_bme)
	{
	assertTestPass(2_platform_40_init_bme);

	const uint32_t interval = 2000;
	const uint32_t ntries = 10;
	static uint32_t lasttime, thistry;
	uint32_t now = millis();

	if ((int32_t)(now - lasttime) < interval)
		/* skip */;
	else
		{
		lasttime = now;

	       	Adafruit_BME280::Measurements m = gBME280.readTemperaturePressureHumidity();
		Serial.print("BME280:  T: "); Serial.print(m.Temperature);
		Serial.print("  P: "); Serial.print(m.Pressure);
		Serial.print("  RH: "); Serial.print(m.Humidity); Serial.println("%");

		assertMore(m.Temperature, 10, "Temperature in lab must be > 10 deg C: " << m.Temperature);
		assertLess(m.Temperature, 40, "Temperature in lab must be < 40 deg C: " << m.Temperature);
		assertMore(m.Pressure, 800 * 100);
		assertLess(m.Pressure, 1100 * 100);
		assertMore(m.Humidity, 5);
		assertLess(m.Humidity, 100);

		if (++thistry >= ntries)
			pass();
		}
	}

testing(3_platform_70_vBat)
	{
	const uint32_t interval = 2000;
	const uint32_t ntries = 10;
	static uint32_t lasttime, thistry;
	uint32_t now = millis();

	if ((int32_t)(now - lasttime) < interval)
		/* skip */;
	else
		{
		lasttime = now;

		float vBat = gCatena.ReadVbat();
		Serial.print("Vbat:   "); Serial.println(vBat);

		assertMore(vBat, 3.1);
		assertLess(vBat, 4.5);
		if (++thistry >= ntries)
			pass();
		}
	}

const uint16_t kModel = 201;

testing(3_platform_80_modbus)
	{
	assertTestPass(2_platform_80_modbus_init);
	static enum : unsigned { stInitial, stDelay, stQuery, stPoll, stCheck, stNextDev, stNextTry } state = stInitial;
	static unsigned iDevice;
	static const uint8_t myDevices[] = { 1, 2 };
	static uint32_t lastUptime[sizeof(myDevices)];
	static uint32_t lastQueryStart;
	static modbus_t telegram;
	constexpr unsigned nRegisters = 
				unsigned(WattNodeModbus::Register::Model_i16) -
				unsigned(WattNodeModbus::Register::SerialNumber_i32) + 1
				;
	static uint16_t modbusRegisters[nRegisters];

        const uint32_t interval = 500;
        const uint32_t ntries = 10;
        static uint32_t lasttime, thistry;
        uint32_t now = millis();

	switch (state)
		{
        case stInitial:
                lasttime = now;
                state = stDelay;
                break;

	// waiting for something to do
	case stDelay:
                if (! fFlashDone)
                        /* skip */;
		else if ((int32_t)(now - lasttime) < interval)
			/* skip */;
		else
			{
			lasttime = now;
			state = stQuery;
			}
		break;

	case stQuery:
                // Serial.println(" stQuery: launch query");
		telegram.u8id = myDevices[iDevice];
		telegram.u8fct = MB_FC_READ_REGISTERS;
		telegram.u16RegAdd = uint16_t(WattNodeModbus::Register::SerialNumber_i32) - 1;
		telegram.u16CoilsNo = nRegisters;
		telegram.au16reg = modbusRegisters;

		gModbusHost.setLastError(ERR_SUCCESS);
		assertEqual(gModbusHost.getLastError(), ERR_SUCCESS);

		assertEqual(gModbusHost.query(telegram), 0);
		state = stPoll;
		break;

	// polling
	case stPoll:
		{
		auto iStatus = gModbusHost.poll();
		
		if (iStatus == 0)
			/* return */;
		else if (iStatus == 1)
			{
                        // Serial.println(" poll() succeeded");
			assertEqual(gModbusHost.getLastError(), ERR_SUCCESS,
				"non-zero error after successful poll(): " << gModbusHost.getLastError()
				);
			state = stCheck;
			}
		else
			{
                        // Serial.println(" poll() failed");
			assertNotEqual(gModbusHost.getLastError(), ERR_SUCCESS,
				"gModbusHost.poll() failed but error not set"
				);
			assertEqual(iStatus, -1, "iStatus not -1?");
			assertNotEqual(iStatus, -1, "poll() failed: " << gModbusHost.getLastError());
			}
		}
		break;

	// looking at results
	case stCheck:
		{
		uint32_t uSerial = modbusRegisters[0] + (modbusRegisters[1] << 16);
		uint32_t uUptime = modbusRegisters[2] + (modbusRegisters[3] << 16);
		uint16_t const uModel = modbusRegisters[nRegisters - 1];

		gCatena.SafePrintf(" device %02x: sn=%-8u uptime=%-8u model=%u\n",
				myDevices[iDevice], uSerial, uUptime, uModel
				);

		assertEqual(uModel, kModel, "Wrong model number: " << uModel);
		assertNotEqual(uUptime, 0);

		if (thistry > 0)
			assertLessOrEqual(lastUptime[iDevice], uUptime);

		lastUptime[iDevice] = uUptime;
		state = stNextDev;
		}

	// selecting next device
	case stNextDev:
		if (++iDevice >= sizeof(myDevices))
			{
			iDevice = 0;
			state = stNextTry;
			}
		else
			state = stQuery;

	// deciding if we're done
	case stNextTry:
		if (++thistry >= ntries)
			pass();
		else
			state = stDelay;
		break;

	default:
		assertTrue(false, "this shouldn't be possible");
		}
	}

testing(3_platform_99)
	{
	if (checkTestDone(3_platform_50_lux) &&
	    checkTestDone(3_platform_60_bme) &&
	    checkTestDone(3_platform_70_vBat) &&
            checkTestDone(3_platform_80_modbus))
		{
		assertTestPass(3_platform_50_lux);
		assertTestPass(3_platform_60_bme);
		assertTestPass(3_platform_70_vBat);
                assertTestPass(3_platform_80_modbus);
                pass();
		}
	}

//-----------------------------------------------------
//      Network tests
//-----------------------------------------------------

// make sure we're provisioned.
testing(4_lora_00_provisioned)
	{
	if (! checkTestDone(3_platform_99))
		return;

	assertTrue(gLoRaWAN.IsProvisioned(), "Not provisioned yet");
	pass();
	}

// Send a confirmed uplink message
Arduino_LoRaWAN::SendBufferCbFn uplinkDone;

uint8_t noncePointer;
bool gfSuccess;
bool gfTxDone;
void *gpCtx;

uint8_t uplinkBuffer[] = { /* port */ 0x10, 0xCA, 0xFE, 0xBA,0xBE };

const uint32_t kLoRaSendTimeout = 40 * 1000;

uint32_t gTxStartTime;

testing(4_lora_10_senduplink)
	{
	if (! checkTestDone(4_lora_00_provisioned))
		return;

	// send a confirmed message.
	if (! checkTestPass(4_lora_00_provisioned))
		{
		skip();
		return;
		}

	assertTrue(gLoRaWAN.SendBuffer(uplinkBuffer, sizeof(uplinkBuffer), uplinkDone, (void *) &noncePointer, true), "SendBuffer failed");
	gTxStartTime = millis();
	pass();
	}

void uplinkDone(void *pCtx, bool fSuccess)
	{
	gfTxDone = true;
	gfSuccess = fSuccess;
	gpCtx = pCtx;
	}

testing(4_lora_20_uplink_done)
	{
	if (checkTestSkip(4_lora_10_senduplink))
		{
		skip();
		return;
		}

	if (!checkTestDone(4_lora_10_senduplink))
		return;

	if (!checkTestPass(4_lora_10_senduplink))
		{
		skip();
		return;
		}

	if (! gfTxDone)
		{
		assertLess((int32_t)(millis() - gTxStartTime), kLoRaSendTimeout, "LoRa transmit timed out");

		return;
		}

	assertTrue(gfSuccess, "Message uplink failed");
	assertTrue(gpCtx == (void *)&noncePointer, "Context pointer was wrong on callback");
	pass();
	}


//-----------------------------------------------------
//      Flash tests
//-----------------------------------------------------
void setup_flash()
	{	
	gSPI2.begin();

	// these *must* be after gSPI2.begin(), because the SPI
	// library resets the pins to defaults as part of the begin()
	// method.
	pinPeripheral(10, PIO_SERCOM);
	pinPeripheral(12, PIO_SERCOM);
	pinPeripheral(11, PIO_SERCOM);
	}

void logMismatch(
	uint32_t addr, 
	uint8_t expect,
	uint8_t actual
	)
	{
	gCatena.SafePrintf(
		"mismatch address %#x: expect %#02x got %02x\n",
		addr, expect, actual
		);
	}

uint32_t vNext(uint32_t v)
	{
	return v * 31413 + 3;
	}

// choose a sector.
const uint32_t sectorAddress = gFlash.DEVICE_SIZE_BYTES - gFlash.SECTOR_SIZE;

union sectorBuffer_t {
	uint8_t b[gFlash.SECTOR_SIZE];
	uint32_t dw[gFlash.SECTOR_SIZE / sizeof(uint32_t)];
	} sectorBuffer;

test(1_flash_00init)
	{
	assertTrue(flash_init(), "flash_init()");
	pass();
	}

test(1_flash_01erase)
	{
	// erase the sector.
	assertTestPass(1_flash_00init);
	assertTrue(gFlash.eraseSector(sectorAddress));
	pass();
	}

uint32_t flashBlankCheck(
	uint32_t a = sectorAddress,
	sectorBuffer_t &buffer = sectorBuffer
	)
	{
	// make sure the sector is blank
	memset(buffer.b, 0, sizeof(buffer));
	gFlash.read(a, buffer.b, sizeof(buffer.b));
	unsigned errs = 0;
	for (auto i = 0; i < sizeof(buffer.b); ++i)
		{
		if (buffer.b[i] != 0xFF)
			{
			logMismatch(a + i, 0xFF, buffer.b[i]);
			++errs;
			}
		}
	return errs;
	}

test(1_flash_02blankcheck)
	{
	auto errs = flashBlankCheck();

	assertEqual(errs, 0, "mismatch errors: " << errs);
	pass();
	}

void initBuffer(
	uint32_t v,
	sectorBuffer_t &buffer = sectorBuffer
	)
	{
	for (auto i = 0; 
	     i < sizeof(buffer.dw) / sizeof(buffer.dw[0]); 
	     ++i, v = vNext(v))
	     	{
		buffer.dw[i] = v;
		}
	}

const uint32_t vStart = 0x55555555u;

test(1_flash_03writepattern)
	{
	// write a pattern
	initBuffer(vStart, sectorBuffer);

	assertTrue(gFlash.program(sectorAddress, sectorBuffer.b, sizeof(sectorBuffer.b)),
		"Failed to program sector " << sectorAddress
		);
	}

test(1_flash_04readpattern)
	{
	// read the buffer
	for (auto i = 0; i < sizeof(sectorBuffer.b); ++i)
		sectorBuffer.b[i] = ~sectorBuffer.b[i];

	gFlash.read(sectorAddress, sectorBuffer.b, sizeof(sectorBuffer.b));

	union 	{
		uint8_t b[sizeof(uint32_t)];
		uint32_t dw;
		} vTest, v;
	v.dw = vStart;

	auto errs = 0;
	for (auto i = 0; 
	     i < sizeof(sectorBuffer.dw) / sizeof(sectorBuffer.dw[0]); 
	     ++i, v.dw = vNext(v.dw))
		{
		vTest.dw = sectorBuffer.dw[i];

		for (auto j = 0; j < sizeof(v.b); ++j)
			{
			if (v.b[j] != vTest.b[j])
				{
				++errs;
				logMismatch(sectorAddress + sizeof(uint32_t) * i + j, v.b[j], vTest.b[j]);
				}
			}
		}
	assertEqual(errs, 0, "mismatch errors: " << errs);
	pass();
	}

test(1_flash_05posterase)
	{
	assertTrue(gFlash.eraseSector(sectorAddress));
	pass();
	}

testing(1_flash_99done)
        {
        if (checkTestDone(1_flash_05posterase))
                {
                fFlashDone = true;
                assertTestPass(1_flash_05posterase);
                pass();
                }
        }

// boilerplate setup code
bool flash_init(void)
	{
        bool fFlashFound;

	gCatena.SafePrintf("Init FLASH\n");

	if (gFlash.begin(&gSPI2, 5))
		{
		uint8_t ManufacturerId;
		uint16_t DeviceId;

		gFlash.readId(&ManufacturerId, &DeviceId);
		gCatena.SafePrintf(
			"FLASH found, ManufacturerId=%02x, DeviceId=%04x\n",
			ManufacturerId, DeviceId
			);
		gFlash.powerDown();
		fFlashFound = true;
		}
	else
		{
		gCatena.SafePrintf("No FLASH found\n");
		fFlashFound = false;
		}
	return fFlashFound;
	}

//-------------------------------------------------
//      Sensor tests
//-------------------------------------------------


