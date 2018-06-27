/*

  This is a library for the BH1750FVI Digital Light Sensor
  breakout board.

  The board uses I2C for communication. 2 pins are required to
  interface to the device.

  Datasheet: http://rohmfs.rohm.com/en/products/databook/datasheet/ic/sensor/light/bh1750fvi-e.pdf

  Written by Christopher Laws, March, 2013.

*/

#ifndef BH1750_h
#define BH1750_h

#if (ARDUINO >= 100)
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

#include "Wire.h"

#include <stdint.h>

// Uncomment, to enable debug messages
// #define BH1750_DEBUG

// No active state
#define BH1750_POWER_DOWN 0x00

// Wating for measurment command
#define BH1750_POWER_ON 0x01

// Reset data register value - not accepted in POWER_DOWN mode
#define BH1750_RESET 0x07

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE  0x10

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE_2  0x11

// Start measurement at 4lx resolution. Measurement time is approx 16ms.
#define BH1750_CONTINUOUS_LOW_RES_MODE  0x13

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE  0x20

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE_2  0x21

// Start measurement at 1lx resolution. Measurement time is approx 120ms (+ 20%)
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_LOW_RES_MODE  0x23

#define BH1750_LOW_RES_DELAY_MAX        24
#define BH1750_HIGH_RES_DELAY_MAX       180

#define BH1750_SENSITIVITY_TYP          0x45

class BH1750 {

  public:
    BH1750 (byte addr = 0x23);
    void begin (uint8_t mode = BH1750_CONTINUOUS_HIGH_RES_MODE);
    void configure (uint8_t mode);
    uint16_t readLightLevel(void);

    static constexpr uint8_t limit_measurement_adj(uint8_t uMeasAdjRaw)
        {
        return (uMeasAdjRaw < 0x1F) ? 0x1F :
               (uMeasAdjRaw < 0xFE) ? uMeasAdjRaw :
                                      0xFE;
        }

    static constexpr uint8_t measurement_command_hi(uint8_t uMeasAdj)
        {
        return 0x40 + (limit_measurement_adj(uMeasAdj) >> 5);
        }

    static constexpr uint8_t measurement_command_low(uint8_t uMeasAdj)
        {
        return 0x20 + (limit_measurement_adj(uMeasAdj) & 0x1F);
        }

    static constexpr uint32_t measurement_adj_denom(void)
        {
        return 256;
        }

    static constexpr uint32_t measurement_adj_num(uint8_t uMeasAdj)
        {
        // round
        return (uMeasAdj * measurement_adj_denom() * 2 + BH1750_SENSITIVITY_TYP) / (2 * BH1750_SENSITIVITY_TYP);
        }

    static constexpr uint32_t measurement_time(uint8_t cmd)
        {
        return (cmd == BH1750_CONTINUOUS_LOW_RES_MODE) ? BH1750_LOW_RES_DELAY_MAX :
               (cmd == BH1750_ONE_TIME_LOW_RES_MODE)   ? BH1750_LOW_RES_DELAY_MAX :
                                                         BH1750_HIGH_RES_DELAY_MAX;
        }

    uint32_t getMeasurementMillis(void) const
        {
        return measurement_time(this->m_mode) * measurement_adj_num(this->m_uMeasAdj) / measurement_adj_denom() + 1;
        }

    bool setMeasurementAdj(float factor);
    bool setMeasurementAdj(uint8_t uAdjRaw);

private:
    void send(uint8_t cmd) const;
    uint32_t m_lux_scale_num;
    uint32_t m_lux_scale_denom = 12;
    int BH1750_I2CADDR;
    uint8_t m_mode;
    uint8_t m_uMeasAdj = BH1750_SENSITIVITY_TYP;
};

#endif
