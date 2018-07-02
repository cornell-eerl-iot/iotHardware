/**************************************************************************/
/*!
    @file     Adafruit_FRAM_I2C.cpp
    @author   KTOWN (Adafruit Industries)
    @license  BSD (see license.txt)

    Driver for the Adafruit I2C FRAM breakout.

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0 - First release
    v1.1 - Adapt for FM24CL16B

*/
/**************************************************************************/
//#include <avr/pgmspace.h>
//#include <util/delay.h>
#include <stdlib.h>
#include <math.h>

#include "Adafruit_FRAM_I2C.h"

/*========================================================================*/
/*                            CONSTRUCTORS                                */
/*========================================================================*/

/**************************************************************************/
/*!
    Constructor
*/
/**************************************************************************/
Adafruit_FRAM_I2C::Adafruit_FRAM_I2C(void) 
{
  this->m_framInitialized = false;
}

void Adafruit_FRAM_I2C::prepIO(void) const
	{
	// this->m_pWire->setClock(1000000);
	}

uint8_t Adafruit_FRAM_I2C::getI2cAddr(uint16_t framAddr) const
	{
	return this->m_i2c_addr + ((framAddr >> 8) & 0x7);
	}

/*========================================================================*/
/*                           PUBLIC FUNCTIONS                             */
/*========================================================================*/

/**************************************************************************/
/*!
    Initializes I2C and configures the chip (call this function before
    doing anything else)
*/
/**************************************************************************/
boolean Adafruit_FRAM_I2C::begin(uint8_t addr, TwoWire *pWire)
{
  /* scrub and save the address */
  if (addr == 0) // address of 0 is never valid on i2c.
	addr = MB85RC_DEFAULT_ADDRESS;

  this->m_i2c_addr = addr & ~0x7;
  this->m_pWire = pWire;

  pWire->begin();
  
  /* Make sure we're actually connected ... do a begin/end on each address */
  this->prepIO();

  for (uint8_t i = 0; i < 8; ++i)
	{
	pWire->beginTransmission(this->m_i2c_addr + i);
	uint8_t uError = pWire->endTransmission();

	if (uError != 0)
		// device didn't ack
		return false;
	}

  /* Everything seems to be properly initialised and connected */
  this->m_framInitialized = true;

  return true;
}

/**************************************************************************/
/*!
    @brief  Writes a byte at the specific FRAM address
    
    @params[in] framAddr
                The address to write to in FRAM memory (reduced modulo 2k)
    @params[in] value
                The 8-bit value to write at framAddr
*/
/**************************************************************************/
void Adafruit_FRAM_I2C::write8 (uint16_t framAddr, uint8_t value) 
{
  const uint8_t i2c_addr = this->getI2cAddr(framAddr);

  this->prepIO();
  this->m_pWire->beginTransmission(i2c_addr);
  this->m_pWire->write(framAddr & 0xFF);
  this->m_pWire->write(value);
  this->m_pWire->endTransmission();
}

/**************************************************************************/
/*!
    @brief  Writes a buffer to the specific FRAM address
    
    @params[in] framAddr
                The address to write to in FRAM memory (reduced mod 2k)
    @params[in] pBuffer
		Address of data in memory.
    @params[in] nBuffer
                number of bytes to write.
*/
/**************************************************************************/
void Adafruit_FRAM_I2C::write(
	uint16_t framAddr, 
	const uint8_t *pBuffer,
	size_t nBuffer
	)
{
  const uint8_t i2c_addr = this->getI2cAddr(framAddr);

  this->prepIO();

  this->m_pWire->beginTransmission(i2c_addr);
  this->m_pWire->write(framAddr & 0xFF);
  this->m_pWire->write(pBuffer, nBuffer);
  this->m_pWire->endTransmission();
}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value from the specified FRAM address

    @params[in] i2cAddr
                The I2C address of the FRAM memory chip (1010+A2+A1+A0)
    @params[in] framAddr
                The 16-bit address to read from in FRAM memory

    @returns    The 8-bit value retrieved at framAddr
*/
/**************************************************************************/
uint8_t Adafruit_FRAM_I2C::read8 (uint16_t framAddr)
{
  const uint8_t i2c_addr = this->getI2cAddr(framAddr);

  this->prepIO();
  this->m_pWire->beginTransmission(i2c_addr);
  this->m_pWire->write(framAddr & 0xFF);
  this->m_pWire->endTransmission();

  this->m_pWire->requestFrom(i2c_addr, (uint8_t)1);
  while (! this->m_pWire->available())
	/* loop */;

  return this->m_pWire->read();
}

/**************************************************************************/
/*!
    @brief  Reads a buffer from the specified FRAM address

    @params[in] framAddr
                The 16-bit address to read from in FRAM memory
    @params[in] pBuffer
		The base address of the buffer to be filled
    @params[in] nBuffer
		Number of bytes to read.

    @returns    The 8-bit value retrieved at framAddr
*/
/**************************************************************************/
uint8_t Adafruit_FRAM_I2C::read (uint16_t framAddr, uint8_t *pBuffer, size_t nBuffer)
{
  uint8_t const i2c_addr = this->getI2cAddr(framAddr);

  this->prepIO();
  this->m_pWire->beginTransmission(i2c_addr);
  this->m_pWire->write(framAddr & 0xFF);
  this->m_pWire->endTransmission();

  // we can only read 255 bytes at a time.
  if (nBuffer > 0xFF)
	nBuffer = 0xFF;

  this->m_pWire->requestFrom(i2c_addr, (uint8_t) nBuffer);
  uint8_t const save_nBuffer = nBuffer;

  while (this->m_pWire->available() && nBuffer > 0)
	{
	*pBuffer++ = this->m_pWire->read();
	--nBuffer;
	}

  return save_nBuffer - nBuffer;
}

/**************************************************************************/
/*!
    @brief  Reads the Manufacturer ID and the Product ID frm the IC

    @params[out]  manufacturerID
                  The 12-bit manufacturer ID (Fujitsu = 0x00A)
    @params[out]  productID
                  The memory density (bytes 11..8) and proprietary
                  Product ID fields (bytes 7..0). Should be 0x510 for
                  the MB85RC256V.
*/
/**************************************************************************/
bool Adafruit_FRAM_I2C::getDeviceID(DeviceInfo& Info)
{
  uint8_t a[3] = { 0, 0, 0 };
  uint8_t results;
  
  this->m_pWire->beginTransmission(MB85RC_SLAVE_ID >> 1);
  this->m_pWire->write(this->m_i2c_addr << 1);
  results = this->m_pWire->endTransmission(false);

  this->m_pWire->requestFrom(MB85RC_SLAVE_ID >> 1, 3);
  a[0] = this->m_pWire->read();
  a[1] = this->m_pWire->read();
  a[2] = this->m_pWire->read();

  /* Shift values to separate manuf and prod IDs */
  /* See p.10 of http://www.fujitsu.com/downloads/MICRO/fsa/pdf/products/memory/fram/MB85RC256V-DS501-00017-3v0-E.pdf */
  Info.uMfg = (a[0] << 4) + (a[1]  >> 4);
  Info.uProduct = ((a[1] & 0x0F) << 8) + a[2];

  return true;
}
