/*
 * bal_prototype.h
 *
 *  Created on: 26 mars 2020
 *      Author: Christophe
 */
#ifndef BAL_PROTOTYPE_H_
#define BAL_PROTOTYPE_H_

void setup_pin_change_interrupt();

int batteryMeasurement();

void sendBatteryLevel(int battery);
void sendFrontDoorOpening();
void sendBackDoorOpening();
void sendWindowOpening();
void sendAlarm();

void send(int id, int value);

#ifndef __AVR_ATtiny85__
void blink(int pin);
#endif

#endif /* BAL_PROTOTYPE_H_ */
