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
#ifndef __BME280_H__
#define __BME280_H__

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Adafruit_Sensor.h>
#include <Wire.h>

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
    #define BME280_ADDRESS                (0x77)
/*=========================================================================*/

/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/
    enum
    {
      BME280_REGISTER_DIG_T1              = 0x88,
      BME280_REGISTER_DIG_T2              = 0x8A,
      BME280_REGISTER_DIG_T3              = 0x8C,

      BME280_REGISTER_DIG_P1              = 0x8E,
      BME280_REGISTER_DIG_P2              = 0x90,
      BME280_REGISTER_DIG_P3              = 0x92,
      BME280_REGISTER_DIG_P4              = 0x94,
      BME280_REGISTER_DIG_P5              = 0x96,
      BME280_REGISTER_DIG_P6              = 0x98,
      BME280_REGISTER_DIG_P7              = 0x9A,
      BME280_REGISTER_DIG_P8              = 0x9C,
      BME280_REGISTER_DIG_P9              = 0x9E,

      BME280_REGISTER_DIG_H1              = 0xA1,
      BME280_REGISTER_DIG_H2              = 0xE1,
      BME280_REGISTER_DIG_H3              = 0xE3,
      BME280_REGISTER_DIG_H4              = 0xE4,
      BME280_REGISTER_DIG_H5              = 0xE5,
      BME280_REGISTER_DIG_H6              = 0xE7,

      BME280_REGISTER_CHIPID             = 0xD0,
      BME280_REGISTER_VERSION            = 0xD1,
      BME280_REGISTER_SOFTRESET          = 0xE0,

      BME280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0

      BME280_REGISTER_CONTROLHUMID       = 0xF2,
      BME280_REGISTER_STATUS             = 0xF3,
      BME280_REGISTER_CONTROL            = 0xF4,
      BME280_REGISTER_CONFIG             = 0xF5,
      BME280_REGISTER_PRESSUREDATA       = 0xF7,
      BME280_REGISTER_TEMPDATA           = 0xFA,
      BME280_REGISTER_HUMIDDATA          = 0xFD,
    };

/*=========================================================================*/

/*=========================================================================
    CALIBRATION DATA
    -----------------------------------------------------------------------*/
    typedef struct
    {
      uint16_t dig_T1;
      int16_t  dig_T2;
      int16_t  dig_T3;

      uint16_t dig_P1;
      int16_t  dig_P2;
      int16_t  dig_P3;
      int16_t  dig_P4;
      int16_t  dig_P5;
      int16_t  dig_P6;
      int16_t  dig_P7;
      int16_t  dig_P8;
      int16_t  dig_P9;

      uint8_t  dig_H1;
      int16_t  dig_H2;
      uint8_t  dig_H3;
      int16_t  dig_H4;
      int16_t  dig_H5;
      int8_t   dig_H6;
    } bme280_calib_data;
/*=========================================================================*/

/*
class Adafruit_BME280_Unified : public Adafruit_Sensor
{
  public:
    Adafruit_BME280_Unified(int32_t sensorID = -1);

    bool  begin(uint8_t addr = BME280_ADDRESS);
    void  getTemperature(float *temp);
    void  getPressure(float *pressure);
    float pressureToAltitude(float seaLevel, float atmospheric, float temp);
    float seaLevelForAltitude(float altitude, float atmospheric, float temp);
    void  getEvent(sensors_event_t*);
    void  getSensor(sensor_t*);

  private:
    uint8_t   _i2c_addr;
    int32_t   _sensorID;
};

*/

class Adafruit_BME280
{
  public:
    enum class OPERATING_MODE { Normal, Forced, Sleep };
    enum class OVERSAMPLE_MODE { Skip = 0, x1, x2, x4, x8, x16 };
    enum PARAM
	{
	CHIP_ID_READ_COUNT = 5,
	CHIP_ID = 0x60,
	REGISTER_READ_DELAY = 1, /* ms */
	};

    /*
    || these times all use a consistent formula: (us * 4 + 125)/250,
    || because we're converting microseconds to 16ths of a second, 
    || and rounding. Since 1/16th is 62.5, we multiply by 4 (not 2)
    || in order to be able to round correctly. They are taken from the
    || Bosch Sensortec sample code and don't match the datasheet exactly;
    || in particular, the datasheet uses 525us for the setup constants,
    || and 2300us for the MEASURE_PER_OSRS_MAX time.
    */
    enum TIMES : unsigned
	{
	T_INIT_MAX		= (1250 * 4 + 125) / 250,
	T_MEASURE_PER_OSRS_MAX  = (2314 * 4 + 125) / 250,
	T_SETUP_PRESSURE_MAX	=  (625 * 4 + 125) / 250,
	T_SETUP_HUMIDITY_MAX	=  (625 * 4 + 125) / 250,
	};

    struct Measurements { float Temperature; float Pressure; float Humidity; };
    Adafruit_BME280(void);
    Adafruit_BME280(int8_t cspin);
    Adafruit_BME280(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin);

    bool  begin(uint8_t addr = BME280_ADDRESS);
    bool  begin(uint8_t addr, OPERATING_MODE mode);
    float readTemperature(void);

    float readPressure(void);
    float readHumidity(void);
    Measurements readTemperaturePressureHumidity(void);
    float readAltitude(float seaLevel);
    float seaLevelForAltitude(float altitude, float atmospheric);

  private:
    inline uint8_t modeRegisterBits(
                OVERSAMPLE_MODE osrs_t,
                OVERSAMPLE_MODE osrs_p,
                OPERATING_MODE mode
                )
        {
        unsigned result;

        switch (mode) {
           case OPERATING_MODE::Sleep: result = 0; break;
           case OPERATING_MODE::Normal: result = 3; break;
           case OPERATING_MODE::Forced: result = 1; break;
           default:        result = 3; break;
        }
        result |= (static_cast<unsigned>(osrs_t) << 5) |
                  (static_cast<unsigned>(osrs_p) << 2);

        return result;
        }
    uint32_t  getMeasurementDelay(void) const;
    float     compensateHumidityFloat(int32_t adc_T, int32_t adc_H);
    int32_t   compensateHumidityInt32(int32_t adc_T, int32_t adc_H);
    float     compensatePressureFloat(int32_t adc_T, int32_t adc_P);
    int32_t   compensatePressureInt32(int32_t adc_T, int32_t adc_P);
    float     compensateTemperatureFloat(int32_t adcVal);
    int32_t   compensateTemperatureInt32(int32_t adcVal);
    bool      isMeasuring(void);
    bool      isReadingCalibration(void);
    void      readCoefficients(void);
    void      startMeasurement(void);
    uint8_t   spixfer(uint8_t x);

    void      write8(byte reg, byte value);
    uint8_t   read8(byte reg);
    uint16_t  read16(byte reg);
    uint32_t  read24(byte reg);
    void      read24x24(byte reg, uint32_t *pv1, uint32_t *pv2);
    void      read24x16(byte reg, uint32_t *pv1, uint16_t *pv2);
    void      read24x24x16(byte reg, uint32_t *pv1, uint32_t *pv2, uint16_t *pv3);
    int16_t   readS16(byte reg);
    uint16_t  read16_LE(byte reg); // little endian
    int16_t   readS16_LE(byte reg); // little endian

    uint8_t   _i2caddr;
    int32_t   _sensorID;
    int32_t   t_fine;

    int8_t _cs, _mosi, _miso, _sck;

    OPERATING_MODE _mode;
    OVERSAMPLE_MODE _osrs_t;
    OVERSAMPLE_MODE _osrs_p;
    OVERSAMPLE_MODE _osrs_h;

    bme280_calib_data _bme280_calib;

};

#endif
