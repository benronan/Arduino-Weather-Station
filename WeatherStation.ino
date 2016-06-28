/*
 Name:		WeatherStation.ino
 Created:	1/8/2016 11:12:13 AM
 Author:	e28ek3b
*/


#include <Wire.h>
#include "BME280.h"

TBME280 weather;

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	Serial.println("Initializing BME280");
	weather.Initialize();
	Serial.println("Initialization complete");
}

// the loop function runs over and over again until power down or reset
void loop() {
	double t, p, h = 0.0;

	Serial.println();
	Serial.println();
	Serial.println("Reading...");

	weather.Read();

	t = weather.Temperature();
	p = weather.Pressure();
	h = weather.Humidity();

	Serial.println("Temperature: " + String(t));
	Serial.println("Pressure: " + String(p));
	Serial.println("Humidity: " + String(h));

	delay(2000);
}
