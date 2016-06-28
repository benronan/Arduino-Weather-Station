// 
// 
// 

#include "BME280.h"

TBME280::TBME280() {
	this->_temperature = 0.0;
	this->_pressure = 0.0;
	this->_humidity = 0.0;
}

double TBME280::Temperature() {
	return this->_temperature;
}

double TBME280::Pressure() {
	return this->_pressure;
}

double TBME280::Humidity() {
	return this->_humidity;
}

void TBME280::Initialize() {
	uint8_t osrs_t = 1;             //Temperature oversampling x 1
	uint8_t osrs_p = 1;             //Pressure oversampling x 1
	uint8_t osrs_h = 1;             //Humidity oversampling x 1
	uint8_t mode = 3;               //Normal mode
	uint8_t t_sb = 5;               //Tstandby 1000ms -- 1 second between readings
	uint8_t filter = 0;             //Filter off 
	uint8_t spi3w_en = 0;           //3-wire SPI Disable

	uint8_t ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode;
	uint8_t config_reg = (t_sb << 5) | (filter << 2) | spi3w_en;
	uint8_t ctrl_hum_reg = osrs_h;

	Wire.begin();

	WriteReg(0xF2, ctrl_hum_reg);
	WriteReg(0xF4, ctrl_meas_reg);
	WriteReg(0xF5, config_reg);
	ReadCalibrationData();
}

void TBME280::WriteReg(uint8_t reg_address, uint8_t data) {
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(reg_address);
	Wire.write(data);
	Wire.endTransmission();
}

void TBME280::Read() {

	unsigned long int hum_raw, temp_raw, pres_raw;
	signed long int temp_cal;
	unsigned long int press_cal, hum_cal;

	ReadData(temp_raw, pres_raw, hum_raw);
	temp_cal = TranslateT(temp_raw);
	press_cal = TranslateP(pres_raw);
	hum_cal = TranslateH(hum_raw);

	this->_temperature = (double)temp_cal / 100.0;
	this->_pressure = (double)press_cal / 100.0;
	this->_humidity = (double)hum_cal / 1024.0;

}

void TBME280::ReadCalibrationData() {
	uint8_t data[32];
	uint8_t i = 0;
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(0x88);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS, 24);
	while (Wire.available()) {
		data[i] = Wire.read();
		i++;
	}

	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(0xA1);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS, 1);
	data[i] = Wire.read();
	i++;

	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(0xE1);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS, 7);
	while (Wire.available()) {
		data[i] = Wire.read();
		i++;
	}
	dig_T1 = (data[1] << 8) | data[0];
	dig_T2 = (data[3] << 8) | data[2];
	dig_T3 = (data[5] << 8) | data[4];
	dig_P1 = (data[7] << 8) | data[6];
	dig_P2 = (data[9] << 8) | data[8];
	dig_P3 = (data[11] << 8) | data[10];
	dig_P4 = (data[13] << 8) | data[12];
	dig_P5 = (data[15] << 8) | data[14];
	dig_P6 = (data[17] << 8) | data[16];
	dig_P7 = (data[19] << 8) | data[18];
	dig_P8 = (data[21] << 8) | data[20];
	dig_P9 = (data[23] << 8) | data[22];
	dig_H1 = data[24];
	dig_H2 = (data[26] << 8) | data[25];
	dig_H3 = data[27];
	dig_H4 = (data[28] << 4) | (0x0F & data[29]);
	dig_H5 = (data[30] << 4) | ((data[29] >> 4) & 0x0F);
	dig_H6 = data[31];
}

void TBME280::ReadData(unsigned long int &t, unsigned long int &p, unsigned long int &h) {
	int i = 0;
	uint32_t data[8];
	Wire.beginTransmission(BME280_ADDRESS);
	Wire.write(0xF7);
	Wire.endTransmission();
	Wire.requestFrom(BME280_ADDRESS, 8);
	while (Wire.available()) {
		data[i] = Wire.read();
		i++;
	}
	p = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
	t = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
	h = (data[6] << 8) | data[7];
}

double TBME280::TranslateT(unsigned long int &value) {
	signed long int var1, var2, T;
	var1 = ((((value >> 3) - ((signed long int)dig_T1 << 1))) * ((signed long int)dig_T2)) >> 11;
	var2 = (((((value >> 4) - ((signed long int)dig_T1)) * ((value >> 4) - ((signed long int)dig_T1))) >> 12) * ((signed long int)dig_T3)) >> 14;

	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

double TBME280::TranslateP(unsigned long int &value) {
	signed long int var1, var2;
	unsigned long int P;
	var1 = (((signed long int)t_fine) >> 1) - (signed long int)64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((signed long int)dig_P6);
	var2 = var2 + ((var1*((signed long int)dig_P5)) << 1);
	var2 = (var2 >> 2) + (((signed long int)dig_P4) << 16);
	var1 = (((dig_P3 * (((var1 >> 2)*(var1 >> 2)) >> 13)) >> 3) + ((((signed long int)dig_P2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1))*((signed long int)dig_P1)) >> 15);
	if (var1 == 0)
	{
		return 0;
	}
	P = (((unsigned long int)(((signed long int)1048576) - value) - (var2 >> 12))) * 3125;
	if (P < 0x80000000)
	{
		P = (P << 1) / ((unsigned long int) var1);
	}
	else
	{
		P = (P / (unsigned long int)var1) * 2;
	}
	var1 = (((signed long int)dig_P9) * ((signed long int)(((P >> 3) * (P >> 3)) >> 13))) >> 12;
	var2 = (((signed long int)(P >> 2)) * ((signed long int)dig_P8)) >> 13;
	P = (unsigned long int)((signed long int)P + ((var1 + var2 + dig_P7) >> 4));
	return P;
}

double TBME280::TranslateH(unsigned long int &value) {
	signed long int v_x1;

	v_x1 = (t_fine - ((signed long int)76800));
	v_x1 = (((((value << 14) - (((signed long int)dig_H4) << 20) - (((signed long int)dig_H5) * v_x1)) +
		((signed long int)16384)) >> 15) * (((((((v_x1 * ((signed long int)dig_H6)) >> 10) *
			(((v_x1 * ((signed long int)dig_H3)) >> 11) + ((signed long int) 32768))) >> 10) + ((signed long int)2097152)) *
			((signed long int) dig_H2) + 8192) >> 14));
	v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((signed long int)dig_H1)) >> 4));
	v_x1 = (v_x1 < 0 ? 0 : v_x1);
	v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);
	return (unsigned long int)(v_x1 >> 12);
}

void TBME280::Reset() {
	WriteReg(0xE0, 0xB6);
}

//#pragma pack(push,1)
//struct TCalibrationData {
//	union {
//		uint16_t T[3];
//		struct {
//			uint16_t dig_T1;
//			int16_t dig_T2;
//			int16_t dig_T3;
//		};
//	};//T
//	union {
//		uint16_t P[9];
//		struct {
//			uint16_t dig_P1;
//			int16_t dig_P2;
//			int16_t dig_P3;
//			int16_t dig_P4;
//			int16_t dig_P5;
//			int16_t dig_P6;
//			int16_t dig_P7;
//			int16_t dig_P8;
//			int16_t dig_P9;
//		};
//	};//P
//	union {
//		uint8_t H[9];
//		struct {
//			int8_t  dig_H1;
//			int16_t dig_H2;
//			int8_t  dig_H3;
//			int16_t dig_H4;
//			int16_t dig_H5;
//			int8_t  dig_H6;
//		};
//	};//H
//};
//#pragma pack(pop)