/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution

  Updated by Terry Moore for MCCI to match the BME280 datasheet Oct 2016
 ***************************************************************************/
#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>
#include "Adafruit_BME280.h"


/***************************************************************************
 PRIVATE FUNCTIONS
 ***************************************************************************/


Adafruit_BME280::Adafruit_BME280()
  : _cs(-1), _mosi(-1), _miso(-1), _sck(-1)
{ }

Adafruit_BME280::Adafruit_BME280(int8_t cspin)
  : _cs(cspin), _mosi(-1), _miso(-1), _sck(-1)
{ }

Adafruit_BME280::Adafruit_BME280(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin)
  : _cs(cspin), _mosi(mosipin), _miso(misopin), _sck(sckpin)
{ }


bool Adafruit_BME280::begin(uint8_t a) {
  return Adafruit_BME280::begin(a, OPERATING_MODE::Normal);
}

bool Adafruit_BME280::begin(uint8_t a, Adafruit_BME280::OPERATING_MODE mode) {
  _i2caddr = a;
  _mode = mode;

  if (_cs == -1) {
    // i2c
    Wire.begin();
  } else {
    digitalWrite(_cs, HIGH);
    pinMode(_cs, OUTPUT);

    if (_sck == -1) {
      // hardware SPI
      SPI.begin();
    } else {
      // software SPI
      pinMode(_sck, OUTPUT);
      pinMode(_mosi, OUTPUT);
      pinMode(_miso, INPUT);
    }
  }

  bool fFound;
  fFound = false;
  for (unsigned i = CHIP_ID_READ_COUNT; i > 0; --i)
	{
	if (read8(BME280_REGISTER_CHIPID) == CHIP_ID)
		{
		fFound = true;
		break;
		}
	delay(REGISTER_READ_DELAY);
	}

  if (! fFound)
    return false;

  // reset the device using soft-reset
  // this makes sure the IIR is off, etc.
  write8(BME280_REGISTER_SOFTRESET, 0xB6);

  // wait 3ms for chip to wake up.
  delay(3);

  // if chip is still reading coefficients, delay
  while (isReadingCalibration())
        /* wait */;

  readCoefficients();

  //need to set CONFIG register?

  OPERATING_MODE initialMode;
  if (_mode == OPERATING_MODE::Normal) {
    _osrs_h = OVERSAMPLE_MODE::x16;
    _osrs_t = OVERSAMPLE_MODE::x16;
    _osrs_p = OVERSAMPLE_MODE::x16;
    initialMode = OPERATING_MODE::Normal;
  } else {
    _osrs_h = OVERSAMPLE_MODE::x16;
    _osrs_t = OVERSAMPLE_MODE::x16;
    _osrs_p = OVERSAMPLE_MODE::x16;
    initialMode = OPERATING_MODE::Sleep;
  }

  //Set before CONTROL_meas (DS 5.4.3)
  write8(BME280_REGISTER_CONTROLHUMID,
	static_cast<unsigned>(this->_osrs_h)
	);

  // treat sleep mode and forced mode the same: start out in sleep
  write8(BME280_REGISTER_CONTROL, modeRegisterBits(
                                        _osrs_t,
                                        _osrs_p,
                                        initialMode
                                        ));

  // for normal mode: delay until we have a measurement; otherwise when
  // we start up, we'll get a bogus initial measurement.
  if (initialMode == OPERATING_MODE::Normal)
	{
	uint32_t const n = this->getMeasurementDelay();
	// Serial.print("BME280: delay "); Serial.print(n); Serial.println(" ms");
	delay(n);
	}

  return true;
}

uint8_t Adafruit_BME280::spixfer(uint8_t x) {
  if (_sck == -1)
    return SPI.transfer(x);

  // software spi
  //Serial.println("Software SPI");
  uint8_t reply = 0;
  for (int i=7; i>=0; i--) {
    reply <<= 1;
    digitalWrite(_sck, LOW);
    digitalWrite(_mosi, x & (1<<i));
    digitalWrite(_sck, HIGH);
    if (digitalRead(_miso))
      reply |= 1;
  }
  return reply;
}

/**************************************************************************/
/*!
    @brief  Writes an 8 bit value over I2C/SPI
*/
/**************************************************************************/
void Adafruit_BME280::write8(byte reg, byte value)
{
  if (_cs == -1) {
    Wire.beginTransmission((uint8_t)_i2caddr);
    Wire.write((uint8_t)reg);
    Wire.write((uint8_t)value);
    Wire.endTransmission();
  } else {
    if (_sck == -1)
      SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg & ~0x80); // write, bit 7 low
    spixfer(value);
    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      SPI.endTransaction();              // release the SPI bus
  }
}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value over I2C
*/
/**************************************************************************/
uint8_t Adafruit_BME280::read8(byte reg)
{
  uint8_t value;

  if (_cs == -1) {
    Wire.beginTransmission((uint8_t)_i2caddr);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)_i2caddr, (byte)1);
    value = Wire.read();

  } else {
    if (_sck == -1)
      SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg | 0x80); // read, bit 7 high
    value = spixfer(0);
    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      SPI.endTransaction();              // release the SPI bus
  }
  return value;
}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit value over I2C
*/
/**************************************************************************/
uint16_t Adafruit_BME280::read16(byte reg)
{
  uint16_t value;

  if (_cs == -1) {
    Wire.beginTransmission((uint8_t)_i2caddr);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)_i2caddr, (byte)2);
    value = (Wire.read() << 8) | Wire.read();

  } else {
    if (_sck == -1)
      SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg | 0x80); // read, bit 7 high
    value = (spixfer(0) << 8) | spixfer(0);
    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      SPI.endTransaction();              // release the SPI bus
  }

  return value;
}

uint16_t Adafruit_BME280::read16_LE(byte reg) {
  uint16_t temp = read16(reg);
  return (temp >> 8) | (temp << 8);

}

/**************************************************************************/
/*!
    @brief  Reads a signed 16 bit value over I2C
*/
/**************************************************************************/
int16_t Adafruit_BME280::readS16(byte reg)
{
  return (int16_t)read16(reg);

}

int16_t Adafruit_BME280::readS16_LE(byte reg)
{
  return (int16_t)read16_LE(reg);

}


/**************************************************************************/
/*!
    @brief  Reads a 24 bit value over I2C or SPI
*/
/**************************************************************************/

uint32_t Adafruit_BME280::read24(byte reg)
{
  uint32_t value;

  if (_cs == -1) {
    Wire.beginTransmission((uint8_t)_i2caddr);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)_i2caddr, (byte)3);
    
    value = Wire.read();
    value <<= 8;
    value |= Wire.read();
    value <<= 8;
    value |= Wire.read();

  } else {
    if (_sck == -1)
      SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg | 0x80); // read, bit 7 high
    
    value = spixfer(0);
    value <<= 8;
    value |= spixfer(0);
    value <<= 8;
    value |= spixfer(0);

    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      SPI.endTransaction();              // release the SPI bus
  }

  return value;
}

/**************************************************************************/
/*!
    @brief  Reads two 24 bit values over I2C or SPI
*/
/**************************************************************************/

void Adafruit_BME280::read24x24(byte reg, uint32_t *pv1, uint32_t *pv2)
{
  uint32_t value;

  if (_cs == -1) {
    Wire.beginTransmission((uint8_t)_i2caddr);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)_i2caddr, (byte)6);

    value = Wire.read();
    value <<= 8;
    value |= Wire.read();
    value <<= 8;
    value |= Wire.read();
    *pv1 = value;

    value = Wire.read();
    value <<= 8;
    value |= Wire.read();
    value <<= 8;
    value |= Wire.read();
    *pv2 = value;
  } else {
    if (_sck == -1)
      SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg | 0x80); // read, bit 7 high

    value = spixfer(0);
    value <<= 8;
    value |= spixfer(0);
    value <<= 8;
    value |= spixfer(0);
    *pv1 = value;

    value = spixfer(0);
    value <<= 8;
    value |= spixfer(0);
    value <<= 8;
    value |= spixfer(0);
    *pv2 = value;

    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      SPI.endTransaction();              // release the SPI bus
  }
}

/**************************************************************************/
/*!
   @brief  Reads a 24-bit value followed by a 16-bit value over I2C or SPI
*/
/**************************************************************************/

void Adafruit_BME280::read24x16(byte reg, uint32_t *pv1, uint16_t *pv2)
{
  uint32_t value;

  if (_cs == -1) {
    Wire.beginTransmission((uint8_t)_i2caddr);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)_i2caddr, (byte)5);

    value = Wire.read();
    value <<= 8;
    value |= Wire.read();
    value <<= 8;
    value |= Wire.read();
    *pv1 = value;

    value = Wire.read();
    value <<= 8;
    value |= Wire.read();
    *pv2 = value;
  } else {
    if (_sck == -1)
      SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    spixfer(reg | 0x80); // read, bit 7 high

    value = spixfer(0);
    value <<= 8;
    value |= spixfer(0);
    value <<= 8;
    value |= spixfer(0);
    *pv1 = (uint16_t) value;

    value = spixfer(0);
    value <<= 8;
    value |= spixfer(0);
    *pv2 = value;

    digitalWrite(_cs, HIGH);
    if (_sck == -1)
      SPI.endTransaction();              // release the SPI bus
  }
}

/**************************************************************************/
/*!
    @brief  Reads two 24-bit values followed by a 16-bit value over I2C or SPI
*/
/**************************************************************************/

void Adafruit_BME280::read24x24x16(byte reg, uint32_t *pv1, uint32_t *pv2, uint16_t *pv3)
        {
        uint32_t value;

        if (_cs == -1) {
                Wire.beginTransmission((uint8_t)_i2caddr);
                Wire.write((uint8_t)reg);
                Wire.endTransmission();
                Wire.requestFrom((uint8_t)_i2caddr, (byte) 8);

                value = Wire.read();
                value <<= 8;
                value |= Wire.read();
                value <<= 8;
                value |= Wire.read();
                *pv1 = value;

                value = Wire.read();
                value <<= 8;
                value |= Wire.read();
                value <<= 8;
                value |= Wire.read();
                *pv2 = value;

                value = Wire.read();
                value <<= 8;
                value |= Wire.read();
                *pv3 = (uint16_t) value;
                } else {
                if (_sck == -1)
                        SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
                digitalWrite(_cs, LOW);
                spixfer(reg | 0x80); // read, bit 7 high

                value = spixfer(0);
                value <<= 8;
                value |= spixfer(0);
                value <<= 8;
                value |= spixfer(0);
                *pv1 = value;

                value = spixfer(0);
                value <<= 8;
                value |= spixfer(0);
                value <<= 8;
                value |= spixfer(0);
                *pv2 = value;

                value = spixfer(0);
                value <<= 8;
                value |= spixfer(0);
                *pv3 = (uint16_t) value;

                digitalWrite(_cs, HIGH);
                if (_sck == -1)
                        SPI.endTransaction();              // release the SPI bus
                }
        }


/**************************************************************************/
/*!
    @brief  Reads the factory-set coefficients
*/
/**************************************************************************/
void Adafruit_BME280::readCoefficients(void)
{
    _bme280_calib.dig_T1 = read16_LE(BME280_REGISTER_DIG_T1);
    _bme280_calib.dig_T2 = readS16_LE(BME280_REGISTER_DIG_T2);
    _bme280_calib.dig_T3 = readS16_LE(BME280_REGISTER_DIG_T3);

    _bme280_calib.dig_P1 = read16_LE(BME280_REGISTER_DIG_P1);
    _bme280_calib.dig_P2 = readS16_LE(BME280_REGISTER_DIG_P2);
    _bme280_calib.dig_P3 = readS16_LE(BME280_REGISTER_DIG_P3);
    _bme280_calib.dig_P4 = readS16_LE(BME280_REGISTER_DIG_P4);
    _bme280_calib.dig_P5 = readS16_LE(BME280_REGISTER_DIG_P5);
    _bme280_calib.dig_P6 = readS16_LE(BME280_REGISTER_DIG_P6);
    _bme280_calib.dig_P7 = readS16_LE(BME280_REGISTER_DIG_P7);
    _bme280_calib.dig_P8 = readS16_LE(BME280_REGISTER_DIG_P8);
    _bme280_calib.dig_P9 = readS16_LE(BME280_REGISTER_DIG_P9);

    _bme280_calib.dig_H1 = read8(BME280_REGISTER_DIG_H1);
    _bme280_calib.dig_H2 = readS16_LE(BME280_REGISTER_DIG_H2);
    _bme280_calib.dig_H3 = read8(BME280_REGISTER_DIG_H3);
    _bme280_calib.dig_H4 = (read8(BME280_REGISTER_DIG_H4) << 4) | (read8(BME280_REGISTER_DIG_H4+1) & 0xF);
    _bme280_calib.dig_H5 = (read8(BME280_REGISTER_DIG_H5+1) << 4) | (read8(BME280_REGISTER_DIG_H5) >> 4);
    _bme280_calib.dig_H6 = (int8_t)read8(BME280_REGISTER_DIG_H6);
}

/**************************************************************************/
/*!
    @brief return true if a measurement is in progress
*/
/**************************************************************************/
bool Adafruit_BME280::isMeasuring(void)
{
  uint8_t const rStatus = read8(BME280_REGISTER_STATUS);

  return (rStatus & (1 << 3)) != 0;
}

/**************************************************************************/
/*!
    @brief return true if chip is busy reading cal data
*/
/**************************************************************************/
bool Adafruit_BME280::isReadingCalibration(void)
{
  uint8_t const rStatus = read8(BME280_REGISTER_STATUS);

  return (rStatus & (1 << 0)) != 0;
}


/**************************************************************************/
/*!
    @brief start a measurement (for forced mode).
*/
/**************************************************************************/
void Adafruit_BME280::startMeasurement(void)
{
  if (_mode == OPERATING_MODE::Normal)
    return;

  /* observe that the BME280 is idle */
  while (isMeasuring())
        /* wait */;

  /* trigger a measurement */
  write8(BME280_REGISTER_CONTROL, 
        modeRegisterBits(
          _osrs_t,
          _osrs_p,
          OPERATING_MODE::Forced
          ));

  /*
  || wait for measurement to complete; how long is a function
  || of the measurement type.
  */
  delay(this->getMeasurementDelay());

  /* put the device back to sleep in sleep mode */
  if (_mode == OPERATING_MODE::Forced)
        return;

  write8(BME280_REGISTER_CONTROL,
        modeRegisterBits(
          _osrs_t,
          _osrs_p,
          OPERATING_MODE::Sleep
          ));
}
/**************************************************************************/
/*!
    @brief read a temperature (only) from the BME280 in Celsius
*/
/**************************************************************************/
float Adafruit_BME280::readTemperature(void)
{
  startMeasurement();

  /* get data. the rightshift is OK, because read24() is unsigned */
  const int32_t adc_T = read24(BME280_REGISTER_TEMPDATA) >> 4;

  /*
  || we sometimes want to do this with daa that we've fetched as a group,
  || so we have a separate function
  */
  return compensateTemperatureFloat(adc_T);
}

/**************************************************************************/
/*!
    @brief convert a uncompensated temperature value to Celsius
*/
/**************************************************************************/
float Adafruit_BME280::compensateTemperatureFloat(int32_t adc_T)
{
  return compensateTemperatureInt32(adc_T) / 100.0;
}

int32_t Adafruit_BME280::compensateTemperatureInt32(int32_t adc_T)
{
#if 0
  int32_t var1, var2;

  /* x1 in Bosch API terminology */
  var1  = ( ( ((adc_T>>3) - ((int32_t)_bme280_calib.dig_T1 <<1)) ) *
	      ((int32_t)_bme280_calib.dig_T2)
          ) >> 11;

  /* x2 in Bosch terminology */
  var2  = (((((adc_T>>4) - ((int32_t)_bme280_calib.dig_T1)) *
	     ((adc_T>>4) - ((int32_t)_bme280_calib.dig_T1))) >> 12) *
	   ((int32_t)_bme280_calib.dig_T3)) >> 14;

  t_fine = var1 + var2;
#else
  // code from https://github.com/embeddedadventures/BME280
  double var1, var2;

  var1 = (((double)adc_T)/16384.0 - ((double)_bme280_calib.dig_T1)/1024.0) * ((double)_bme280_calib.dig_T2);
  var2 = ((((double)adc_T)/131072.0 - ((double)_bme280_calib.dig_T1)/8192.0) *
     (((double)adc_T)/131072.0 - ((double) _bme280_calib.dig_T1)/8192.0)) * ((double)_bme280_calib.dig_T3);

  t_fine = (int32_t)(var1 + var2);
#endif

  return (t_fine * 5 + 128) >> 8;
}

/**************************************************************************/
/*!
    @brief Measure and return pressure in hPa (or millibars)
*/
/**************************************************************************/
float Adafruit_BME280::readPressure(void) {
  uint32_t u_adc_T, u_adc_P;
  int32_t adc_T, adc_P;
  startMeasurement();

  read24x24(BME280_REGISTER_PRESSUREDATA, &u_adc_P, &u_adc_T);

  /* get data. the rightshift is OK, because values are unsigned */
  adc_T = u_adc_T >> 4;
  adc_P = u_adc_P >> 4;

  /* update t_fine */
  (void)compensateTemperatureInt32(adc_T);

  return compensatePressureFloat(adc_T, adc_P);
}

/**************************************************************************/
/*!
    @brief Calculate pressure in hPa (or millibars)
*/
/**************************************************************************/
float Adafruit_BME280::compensatePressureFloat(int32_t adc_T, int32_t adc_P)
{
  return compensatePressureInt32(adc_T, adc_P);
}

/**************************************************************************/
/*!
    @brief Calculate pressure in Pascals (as integer).
*/
/**************************************************************************/
int32_t  Adafruit_BME280::compensatePressureInt32(int32_t adc_T, int32_t adc_P)
{
#if 0
  int32_t var1, var2;
  uint32_t p;

  var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
  var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)_bme280_calib.dig_P6);
  var2 = var2 + ((var1*((int32_t)_bme280_calib.dig_P5))<<1);
  var2 = (var2>>2)+(((int32_t)_bme280_calib.dig_P4)<<16);
  var1 = (((_bme280_calib.dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)_bme280_calib.dig_P2) * var1)>>1))>>18;
  var1 =((((32768+var1))*((int32_t)_bme280_calib.dig_P1))>>15);
  if (var1 == 0)
    {
    return 0; // avoid exception caused by division by zero
    }
  p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
  if (p < 0x80000000)
    {
    p = (p << 1) / ((uint32_t)var1);
    }
  else
    {
    p = (p / (uint32_t)var1) * 2;
    }
  var1 = (((int32_t)_bme280_calib.dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
  var2 = (((int32_t)(p>>2)) * ((int32_t)_bme280_calib.dig_P8))>>13;
  p = (uint32_t)((int32_t)p + ((var1 + var2 + _bme280_calib.dig_P7) >> 4));
#else
  // code from https://github.com/embeddedadventures/BME280
  double var1, var2, p;

  var1 = ((double)t_fine/2.0) - 64000.0;
  var2 = var1 * var1 * ((double)_bme280_calib.dig_P6) / 32768.0;
  var2 = var2 + var1 * ((double)_bme280_calib.dig_P5) * 2.0;
  var2 = (var2/4.0)+(((double)_bme280_calib.dig_P4) * 65536.0);
  var1 = (((double)_bme280_calib.dig_P3) * var1 * var1 / 524288.0 + ((double)_bme280_calib.dig_P2) * var1) / 524288.0;
  var1 = (1.0 + var1 / 32768.0)*((double)_bme280_calib.dig_P1);
  if (var1 == 0.0) {
    return 0; // avoid exception caused by division by zero
  }
  p = 1048576.0 - (double)adc_P;
  p = (p - (var2 / 4096.0)) * 6250.0 / var1;
  var1 = ((double)_bme280_calib.dig_P9) * p * p / 2147483648.0;
  var2 = p * ((double)_bme280_calib.dig_P8) / 32768.0;
  p = p + (var1 + var2 + ((double)_bme280_calib.dig_P7)) / 16.0;
#endif
  return p;
}

Adafruit_BME280::Measurements Adafruit_BME280::readTemperaturePressureHumidity()
{
   Measurements result;

   startMeasurement();
   uint32_t u_adc_T;
   uint32_t u_adc_P;
   uint16_t u_adc_H;
   read24x24x16(BME280_REGISTER_PRESSUREDATA, &u_adc_P, &u_adc_T, &u_adc_H);

   int32_t adc_P = u_adc_P >>= 4;
   int32_t adc_T = u_adc_T >>= 4;

   result.Temperature = compensateTemperatureFloat(adc_T);
   result.Pressure = compensatePressureFloat(adc_T, adc_P);
   result.Humidity = compensateHumidityFloat(adc_T, u_adc_H);
   return result;
}

/**************************************************************************/
/*!
    @brief Return relative humidity
*/
/**************************************************************************/
float Adafruit_BME280::readHumidity(void) {
  startMeasurement();

  uint32_t u_adc_T;
  uint16_t u_adc_H;

  read24x16(BME280_REGISTER_TEMPDATA, &u_adc_T, &u_adc_H);

  /* update t_fine */
  int32_t const adc_T = u_adc_T >> 4;
  (void)compensateTemperatureInt32(adc_T);

  return compensateHumidityFloat(adc_T, u_adc_H);
}

float Adafruit_BME280::compensateHumidityFloat(int32_t adc_T, int32_t adc_H)
{
  float h = compensateHumidityInt32(adc_T, adc_H);
  return  h / 1024.0;
}

int32_t Adafruit_BME280::compensateHumidityInt32(int32_t adc_T, int32_t adc_H)
{
  int32_t v_x1_u32r;

  v_x1_u32r = (t_fine - ((int32_t)76800));

  v_x1_u32r = (((((adc_H << 14) - (((int32_t)_bme280_calib.dig_H4) << 20) -
		  (((int32_t)_bme280_calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
	       (((((((v_x1_u32r * ((int32_t)_bme280_calib.dig_H6)) >> 10) *
		    (((v_x1_u32r * ((int32_t)_bme280_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
		  ((int32_t)2097152)) * ((int32_t)_bme280_calib.dig_H2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
			     ((int32_t)_bme280_calib.dig_H1)) >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  return (v_x1_u32r>>12);
}

/**************************************************************************/
/*!
    Calculates the altitude (in meters) from the specified atmospheric
    pressure (in hPa), and sea-level pressure (in hPa).

    @param  seaLevel      Sea-level pressure in hPa
    @param  atmospheric   Atmospheric pressure in hPa
*/
/**************************************************************************/
float Adafruit_BME280::readAltitude(float seaLevel)
{
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  float atmospheric = readPressure() / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}

/**************************************************************************/
/*!
    Calculates the pressure at sea level (in hPa) from the specified altitude 
    (in meters), and atmospheric pressure (in hPa).  
    @param  altitude      Altitude in meters
    @param  atmospheric   Atmospheric pressure in hPa
*/
/**************************************************************************/
float Adafruit_BME280::seaLevelForAltitude(float altitude, float atmospheric)
{
  // Equation taken from BMP180 datasheet (page 17):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064
  
  return atmospheric / pow(1.0 - (altitude/44330.0), 5.255);
}

/**************************************************************************/
/*!
    Calculate the required delay time for the current measurement mode.
*/
/**************************************************************************/

uint32_t Adafruit_BME280::getMeasurementDelay(void) const
	{
	uint32_t basecount;
	uint32_t ms;

	/*
	|| (1 << n) >> 1 is the same as (1 << (n-1)) except
	|| that it works right, portably, when n is 0, returning
	|| zero.
	*/
	basecount = T_MEASURE_PER_OSRS_MAX *
			(((1u << static_cast<unsigned>(this->_osrs_t)) >> 1) +
			 ((1u << static_cast<unsigned>(this->_osrs_p)) >> 1) +
			 ((1u << static_cast<unsigned>(this->_osrs_h)) >> 1));

	if (this->_osrs_p != OVERSAMPLE_MODE::Skip)
		basecount += T_SETUP_PRESSURE_MAX;

	if (this->_osrs_h != OVERSAMPLE_MODE::Skip)
		basecount += T_SETUP_HUMIDITY_MAX;

	/* this will be 114 ms for 16/16/16 */
	ms = (T_INIT_MAX + basecount + 15) / 16;
	return ms;
	}
