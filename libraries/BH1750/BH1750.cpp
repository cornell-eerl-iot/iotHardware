/*

  This is a library for the BH1750FVI Digital Light Sensor
  breakout board.

  The board uses I2C for communication. 2 pins are required to
  interface to the device.

  Written by Christopher Laws, March, 2013.

*/

#include "BH1750.h"

// Define milliseconds delay for ESP8266 platform
#if defined(ESP8266)

  #include <pgmspace.h>
  #define _delay_ms(ms) delayMicroseconds((ms) * 1000)

// Use _delay_ms from utils for AVR-based platforms
#elif defined(__avr__)
  #include <util/delay.h>

// Use Wiring's delay for compability with another platforms
#else
  #define _delay_ms(ms) delay(ms)
#endif


// Legacy Wire.write() function fix
#if (ARDUINO >= 100)
  #define __wire_write(d) Wire.write(d)
#else
  #define __wire_write(d) Wire.send(d)
#endif


// Legacy Wire.read() function fix
#if (ARDUINO >= 100)
  #define __wire_read() Wire.read()
#else
  #define __wire_read() Wire.receive()
#endif


/**
 * Constructor
 * @params addr Sensor address (0x76 or 0x72, see datasheet)
 *
 * On most sensor boards, it was 0x76
 */
BH1750::BH1750(byte addr) {

  BH1750_I2CADDR = addr;

}


/**
 * Begin I2C and configure sensor
 * @param mode Measurment mode
 */
void BH1750::begin(uint8_t mode) {

  // Initialize I2C
  Wire.begin();

  // power it up
  this->configure(BH1750_POWER_ON);

  // reset it
  this->configure(BH1750_RESET);

  // Configure sensor in specified mode
  configure(mode);

}


/**
 * Configurate BH1750 with specified working mode
 * @param mode Measurment mode
 */
void BH1750::configure(uint8_t mode) {

  // Check, is measurment mode exist
  switch (mode)
        {
    case BH1750_CONTINUOUS_HIGH_RES_MODE:
    case BH1750_CONTINUOUS_HIGH_RES_MODE_2:
    case BH1750_CONTINUOUS_LOW_RES_MODE:
    case BH1750_ONE_TIME_HIGH_RES_MODE:
    case BH1750_ONE_TIME_HIGH_RES_MODE_2:
    case BH1750_ONE_TIME_LOW_RES_MODE:
    case BH1750_POWER_ON:
    case BH1750_POWER_DOWN:
            // Send mode to sensor
      this->send(mode);

      // Wait few moments for waking up
      if (this->m_mode == BH1750_POWER_DOWN)
              _delay_ms(10);
      break;

    case BH1750_RESET:
        this->send(BH1750_POWER_ON);
        _delay_ms(10);
        this->send(mode);
        this->m_mode = BH1750_POWER_ON;
        break;

    default:
        // Invalid measurement mode
        #ifdef BH1750_DEBUG
                Serial.println(F("BH1750: Invalid measurment mode"));
        #endif
        return;
        }

  // save mode and scaling value.
  switch (mode)
        {
  case BH1750_ONE_TIME_LOW_RES_MODE:
  case BH1750_CONTINUOUS_LOW_RES_MODE:
        this->m_lux_scale_num = 10;
        this->m_lux_scale_denom = 12;
        this->m_mode = mode;
        break;

  case BH1750_ONE_TIME_HIGH_RES_MODE:
  case BH1750_CONTINUOUS_HIGH_RES_MODE:
        this->m_lux_scale_num = 10;
        this->m_lux_scale_denom = 12;
        this->m_mode = mode;
        break;

  case BH1750_ONE_TIME_HIGH_RES_MODE_2:
  case BH1750_CONTINUOUS_HIGH_RES_MODE_2:
        this->m_lux_scale_num = 5;
        this->m_lux_scale_denom = 12;
        this->m_mode = mode;
        break;

  default:
        this->m_mode = mode;
        break;
        }
}

void BH1750::send(uint8_t cmd) const
        {
        Wire.beginTransmission(BH1750_I2CADDR);
        __wire_write(cmd);
        Wire.endTransmission();
        }

/**
 * Read light level from sensor
 * @return  Lightness in lux (0 ~ 65535)
 */
uint16_t BH1750::readLightLevel(void) {

  // Measurment result will be stored here
  uint16_t level;

  // Read two bytes from sensor
  Wire.requestFrom(BH1750_I2CADDR, 2);

  // Read two bytes, which are low and high parts of sensor value
  level = __wire_read();
  level <<= 8;
  level |= __wire_read();

  // Send raw value if debug enabled
  #ifdef BH1750_DEBUG
    Serial.print(F("[BH1750] Raw value: 0x"));
    Serial.println(level, HEX);
  #endif

  uint32_t scale_level_num = uint32_t(level) * measurement_adj_num(this->m_uMeasAdj) + (measurement_adj_denom() >> 1);

  // Convert raw value to lux
  uint32_t level_num = scale_level_num * this->m_lux_scale_num + (this->m_lux_scale_denom >> 1);

  uint32_t level_result = level_num / (this->m_lux_scale_denom * measurement_adj_denom());

  // Send converted value, if debug enabled
  #ifdef BH1750_DEBUG
    Serial.print(F("[BH1750] Converted value: "));
    Serial.println(level_result);
  #endif

  return level_result;
  }

  bool BH1750::setMeasurementAdj(float adj)
        {
        if (adj > 4 || adj < 0.1)
                return false;
        uint8_t uAdjRaw = uint8_t(adj * BH1750_SENSITIVITY_TYP + 0.5);
        return setMeasurementAdj(uAdjRaw);
        }

  bool BH1750::setMeasurementAdj(uint8_t uAdjRaw)
        {
        uint8_t uAdj = limit_measurement_adj(uAdjRaw);
        if (uAdjRaw != uAdj)
                return false;

        this->m_uMeasAdj = uAdj;
        if (this->m_mode == BH1750_POWER_DOWN)
                this->configure(BH1750_POWER_ON);

        this->send(measurement_command_hi(uAdj));
        this->send(measurement_command_low(uAdj));
        return true;
        }
