/*
 * battery.h
 *
 *  Created on: 4 avr. 2020
 *      Author: Christophe
 */

#ifndef BATTERY_H_
#define BATTERY_H_


int batteryLevel(uint8_t pin);


float integratedBatteryVoltageInitialization(uint8_t pin);

float integratedBatteryVoltage(uint8_t pin);

float averageBatteryVoltage(uint8_t pin);

float rollingAverageBatteryVoltageInitialization(uint8_t pin);

float rollingAverageBatteryVoltage(uint8_t pin);


#endif /* BATTERY_H_ */
