/*
 * battery.cpp
 *
 *  Created on: 4 avr. 2020
 *      Author: Christophe
 */
#include "Arduino.h"

#include "battery.h"

#ifndef __AVR_ATtiny85__
#define DEBUG
// #define SCOPE
// #define MUTE
#endif

/**
 * Battery measurement and battery charge compute.
 */
int batteryLevel(uint8_t pin) {
	float batteryVoltage = integratedBatteryVoltage(pin);

	// Percentage
	int batteryLevel;
	if (batteryVoltage <= 2.8) {
		batteryLevel = 0;
	} else if (batteryVoltage < 4.1) {
		batteryLevel = 100 * (batteryVoltage - 2.8) / (4.1 - 2.8);
	} else {
		batteryLevel = 100;
	}

	/*
	int batteryLevel;
	if (batteryVoltage <= 3) {
		batteryLevel = 0;
	} else if (batteryVoltage <= 3.3) {
		batteryLevel = 5;
	} else if (batteryVoltage <= 3.6) {
		batteryLevel = 10;
	} else if (batteryVoltage <= 3.7) {
		batteryLevel = 20;
	} else if (batteryVoltage <= 3.75) {
		batteryLevel = 30;
	} else if (batteryVoltage <= 3.79) {
		batteryLevel = 40;
	} else if (batteryVoltage <= 3.83) {
		batteryLevel = 50;
	} else if (batteryVoltage <= 3.87) {
		batteryLevel = 60;
	} else if (batteryVoltage <= 3.92) {
		batteryLevel = 70;
	} else if (batteryVoltage <= 3.97) {
		batteryLevel = 60;
	} else if (batteryVoltage <= 4.10) {
		batteryLevel = 90;
	} else {
		batteryLevel = 100;
	}
	*/

#ifdef SCOPE
	int16_t buffer[3];     //Buffer to hold the packet, note it is 16bit data type

	buffer[0] = 0xCDAB;      //Packet Header, indicating start of packet.
	buffer[1] = 2;           //Size of data payload in bytes.
	buffer[2] = (int16_t) 1000 * batteryVoltage;    //Channel 1 data. 16bit signed integer

	Serial.write((uint8_t * ) buffer, sizeof(buffer));
#endif

#ifdef DEBUG
	// Serial.print("battery: ");
	Serial.print(batteryVoltage);
	Serial.print(" - ");
	Serial.println(batteryLevel);
	delay(50);
#endif

	return batteryLevel;
	// return (int) random(100);
}


#define INTEGRATED_INITIALIZATION_VALUES (100)
#define INTEGRATED_VALUES (15)

float integratedVoltage = 0;
bool initialized = false;


/**
 *
 */
float integratedBatteryVoltageInitialization(uint8_t pin) {
	for (int count = 0; count < INTEGRATED_INITIALIZATION_VALUES; count++) {
		integratedVoltage += ((analogRead(pin) * 5.0 / 1023) - integratedVoltage) / INTEGRATED_VALUES;
	}
	return integratedVoltage;
}

/**
 *
 */
float integratedBatteryVoltage(uint8_t pin) {
	delay(5);

	if (!initialized) {
		initialized = true;
		return integratedBatteryVoltageInitialization(pin);
	}

	for (int count = 0; count < INTEGRATED_VALUES; count++) {
		integratedVoltage += ((analogRead(pin) * 5.0 / 1023) - integratedVoltage) / INTEGRATED_VALUES;
	}
	return integratedVoltage;
}

#define AVERAGE_READ_COUNT (5)

/**
 *
 */
float averageBatteryVoltage(uint8_t pin) {
	int valueSum = 0;
	int min = 1024;
	int max = -1;

	for(int i = 0; i < AVERAGE_READ_COUNT; i++) {
		int value = analogRead(pin);
		valueSum += value;
		if (value > max) {
			max = value;
		} else if (value < min) {
			min = value;
		}
	}
	return 5.0 * (valueSum - min - max + 0.5) / ((AVERAGE_READ_COUNT - 2) * 1023);
}

#define ROLLING_AVERAGE_READ_COUNT (16)

float voltages[ROLLING_AVERAGE_READ_COUNT];
int voltageRollingIndex = 0;
float averageVoltage;

/**
 *
 */
float rollingAverageBatteryVoltageInitialization(uint8_t pin) {
	averageVoltage = 0.0;
	for(; voltageRollingIndex < ROLLING_AVERAGE_READ_COUNT; ) {
		float value = (analogRead(pin) + 0.5) * 5.0 / 1024.0;

		averageVoltage += (value - voltages[voltageRollingIndex]) / ROLLING_AVERAGE_READ_COUNT;
		voltages[voltageRollingIndex++] = value;
	}

	return voltageRollingIndex;
}

/**
 *
 */
float rollingAverageBatteryVoltage(uint8_t pin) {
	delay(5);

	if (!initialized) {
		rollingAverageBatteryVoltageInitialization(pin);
		initialized = true;
	}

	float value = (analogRead(pin) + 0.5) * 5.0 / 1024.0;

	if (voltageRollingIndex == ROLLING_AVERAGE_READ_COUNT) {
		voltageRollingIndex = 0;
	}
	averageVoltage += (value - voltages[voltageRollingIndex]) / ROLLING_AVERAGE_READ_COUNT;
	voltages[voltageRollingIndex++] = value;

#ifdef DEBUG
	for(int i= 0; i < ROLLING_AVERAGE_READ_COUNT;) {
		Serial.print(voltages[i++]);
		Serial.print(" ");
	}
	Serial.println();
#endif

	return averageVoltage;
}

