// BME280.h

#ifndef _BME280_h
#define _BME280_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Wire.h>

#define BME280_ADDRESS 0x76

class TBME280 {
private:
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;
	int8_t  dig_H1;
	int16_t dig_H2;
	int8_t  dig_H3;
	int16_t dig_H4;
	int16_t dig_H5;
	int8_t  dig_H6;
	signed long int t_fine;

	double _temperature;
	double _pressure;
	double _humidity;

	void WriteReg(uint8_t reg_address, uint8_t data);

	void ReadCalibrationData();
	void ReadData(unsigned long int &t, unsigned long int &p, unsigned long int &h);
	double TranslateT(unsigned long int &v);
	double TranslateP(unsigned long int &v);
	double TranslateH(unsigned long int &v);

public:
	TBME280();

	void Initialize();
	void Read();
	
	void Reset();

	double Temperature();
	double Pressure();
	double Humidity();
	
};

#endif

