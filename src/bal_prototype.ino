#include "Arduino.h"

#include <avr/wdt.h>        // Supplied Watch Dog Timer Macros
#include <avr/sleep.h>      // Supplied AVR Sleep Macros
#include <avr/interrupt.h>

#include "bal_prototype.h"
#include "watchdog.h"
#include "battery.h"
#include "X10RF.h"

#ifndef __AVR_ATtiny85__
#define DEBUG
// #define SCOPE
// #define MUTE
#endif

//
#ifndef __AVR_ATtiny85__
#define PIN_LED_BLUE (2)
#define PIN_LED_RED (3)
#endif

// Digital output (433MHz transmitter)
#ifdef __AVR_ATtiny85__
#define PIN_TX_DATA (4)
#else
#define PIN_TX_DATA (12)
#endif

// Digital input pullup
#ifdef __AVR_ATtiny85__
#define PIN_REED_WINDOW (0)
#define PIN_REED_FRONT_DOOR (1)
#define PIN_REED_BACK_DOOR (2)
#define PIN_ALARM_SWITCH (3)
#else
#define PIN_REED_WINDOW (8)
#define PIN_REED_FRONT_DOOR (9)
#define PIN_REED_BACK_DOOR (10)
#define PIN_ALARM_SWITCH (17)
#endif

// Analog read (battery measurement)
#define PIN_BATTERY (A3)


// #define BATTERY_MEASUREMENT_INTERVAL_S (8L)
// Battery measurement interval in seconds (1 hours)
#define BATTERY_MEASUREMENT_INTERVAL_S (1L * 60 * 60)
// Battery measurement interval in seconds (24 hours)
// #define BATTERY_MEASUREMENT_INTERVAL_S (24L * 60 * 60)
// Number of watchdog timeouts between two battery messages sent (1h/8s)
#define WDT_COUNT_BATTERY_SEND (BATTERY_MEASUREMENT_INTERVAL_S / 8L);


// Counters
volatile bool watchdogTimeout = false;

volatile bool frontDoorOpening = false;
volatile bool backDoorOpening = false;
volatile bool windowOpening = false;
volatile bool alarmSwitch = false;

unsigned int watchdogTimeoutCount = 0;


// Number of watchdog timeouts between two messages sent (eq. 64s)
const byte WDT_COUNT_SEND = 8;

byte frontDoorOpeningWdtCount = 0;
byte backDoorOpeningWdtCount = 0;
byte windowOpeningWdtCount = 0;
byte alarmSwitchWdtCount = 0;


// Stop sending message after
const byte MAX_SEND = 10;

byte frontDoorOpeningCount = 0;
byte backDoorOpeningCount = 0;
byte windowOpeningCount = 0;
byte alarmSwitchCount = 0;


// X10
#define X10_REPEAT_COUNT (5)

#define RFXSENSOR_WINDOW_ID (1)
#define RFXSENSOR_FRONT_DOOR_ID (2)
#define RFXSENSOR_BACK_DOOR_ID (3)
#define RFXSENSOR_BATTERY_ID (4)
#define RFXSENSOR_ALARM_ID (5)

x10rf x10 = x10rf(PIN_TX_DATA, 0, X10_REPEAT_COUNT);


/**
 * SETUP.
 */
void setup() {
#ifdef DEBUG
	pinMode(PIN_LED_BLUE, OUTPUT);
	pinMode(PIN_LED_RED, OUTPUT);

	digitalWrite(PIN_LED_BLUE, LOW);
	digitalWrite(PIN_LED_RED, LOW);

//blink(PIN_LED_BLUE);
	blink(PIN_LED_RED);
#endif

	pinMode(PIN_REED_WINDOW, INPUT_PULLUP);
	pinMode(PIN_REED_BACK_DOOR, INPUT_PULLUP);
	pinMode(PIN_REED_FRONT_DOOR, INPUT_PULLUP);

	pinMode(PIN_ALARM_SWITCH, INPUT);
	// pinMode(PIN_BATTERY, INPUT);

	pinMode(PIN_TX_DATA, OUTPUT);
	digitalWrite(PIN_TX_DATA, LOW);

#ifdef DEBUG
	randomSeed(analogRead(0));
#endif

	setup_pin_change_interrupt();

	x10.begin();

	send(RFXSENSOR_FRONT_DOOR_ID, 0);
	send(RFXSENSOR_BACK_DOOR_ID, 0);
	send(RFXSENSOR_WINDOW_ID, 0);
	send(RFXSENSOR_ALARM_ID, 0);

	setup_watchdog();

#ifdef DEBUG
	Serial.begin(115200);
	delay(50);
	Serial.println("Ready...");
	delay(50);
#endif
#ifdef SCOPE
	Serial.begin(115200);
	delay(50);
#endif
}


/**
 * MAIN LOOP.
 */
void loop() {
	system_sleep();

	if (watchdogTimeout) {
		if (frontDoorOpeningWdtCount > 0) {
			frontDoorOpeningWdtCount--;
		}
		if (backDoorOpeningWdtCount > 0) {
			backDoorOpeningWdtCount--;
		}
		if (windowOpeningWdtCount > 0) {
			windowOpeningWdtCount--;
		}
		if (alarmSwitchWdtCount > 0) {
			alarmSwitchWdtCount--;
		}
		if (watchdogTimeoutCount > 0) {
			watchdogTimeoutCount--;
		}
		else {
			watchdogTimeoutCount = WDT_COUNT_BATTERY_SEND;
#ifdef SCOPE
			batteryLevel(PIN_BATTERY);
#else
#ifdef DEBUG
			batteryLevel(PIN_BATTERY);
#else
			int level = batteryLevel(PIN_BATTERY);
			sendBatteryLevel(level);
#endif
#endif
		}
		watchdogTimeout = false;
	}
	else {
		if (frontDoorOpening) {
			sendFrontDoorOpening();
		}
		if (backDoorOpening) {
			sendBackDoorOpening();
		}
		if (windowOpening) {
			sendWindowOpening();
		}
		if (alarmSwitch) {
			sendAlarm();
		}
	}
}


/**
 * Watchdog Interrupt Service...
 * Is executed when watchdog timed out.
 */
ISR(WDT_vect) {
  watchdogTimeout = true;  // set global flag
}


/**
 * Pin Change Interrupts (PCINT)
 */
ISR(PCINT0_vect) {
	// digitalWrite(PIN_LED_RED, digitalRead(PIN_REED_FRONT_DOOR) | digitalRead(PIN_REED_BACK_DOOR) | digitalRead(PIN_REED_WINDOW));
	if (!frontDoorOpening && digitalRead(PIN_REED_FRONT_DOOR) == HIGH) {
		frontDoorOpening = true;
	}
	if (!backDoorOpening && digitalRead(PIN_REED_BACK_DOOR) == HIGH) {
		backDoorOpening = true;
	}
	if (!windowOpening && digitalRead(PIN_REED_WINDOW) == HIGH) {
		windowOpening = true;
	}
#ifdef __AVR_ATtiny85__
	if (!alarmSwitch && digitalRead(PIN_ALARM_SWITCH) == LOW) {
		alarmSwitch = true;
	}
#endif
}

#ifndef __AVR_ATtiny85__
ISR(PCINT1_vect) {
	digitalWrite(PIN_LED_BLUE, !digitalRead(PIN_ALARM_SWITCH));
	if (!alarmSwitch && digitalRead(PIN_ALARM_SWITCH) == LOW) {
		alarmSwitch = true;
	}
}
#endif

/**
 *
 */
void setup_pin_change_interrupt() {
	cli();
#ifdef __AVR_ATtiny85__
	// Turn on Pin Change interrupts (Tell Attiny85 we want to use pin change interrupts (can be any pin))
  sbi(PCMSK, PCINT0);
  sbi(PCMSK, PCINT1);
  sbi(PCMSK, PCINT2);
  sbi(PCMSK, PCINT3);
  sbi(GIMSK, PCIE);
#else
  // PCINT0 for pin 8, 9, 10 and 11
  //	*digitalPinToPCMSK(8) |= bit(digitalPinToPCMSKbit(8));  // enable pin
  //	*digitalPinToPCMSK(9) |= bit(digitalPinToPCMSKbit(9));  // enable pin
  //	*digitalPinToPCMSK(10) |= bit(digitalPinToPCMSKbit(10));  // enable pin
  //	*digitalPinToPCMSK(11) |= bit(digitalPinToPCMSKbit(11));  // enable pin
	// bit(digitalPinToPCICRbit(8)); // clear any outstanding interrupt
	// bit(digitalPinToPCICRbit(8)); // enable interrupt for the group
  sbi(PCMSK0, PCINT0); // PIN 8
  sbi(PCMSK0, PCINT1);
  sbi(PCMSK0, PCINT2);
  sbi(PCMSK0, PCINT3);
  sbi(PCIFR, PCIF0);
  sbi(PCICR, PCIE0);
	//  PCMSK0 |= 0b00001111;
	//	PCIFR  |= 0b00000001;
	//	PCICR  |= 0b00000001;

// 	*digitalPinToPCMSK(PIN_BATTERY) |= bit(digitalPinToPCMSKbit(PIN_BATTERY));  // PCMSK1 3 (PCINT11)
// 	PCIFR  |= bit(digitalPinToPCICRbit(PIN_BATTERY)); // PCIF1
// 	PCIFR  |= bit(digitalPinToPCICRbit(PIN_BATTERY)); // PCIF1
  sbi(PCMSK1, PCINT11); // PIN A3 (17)
  sbi(PCIFR, PCIF1);
  sbi(PCICR, PCIE1);
#endif
  sei(); // Enable the Interrupts
}


/**
 * Send back door opening count, and reset others one.
 */
void sendBackDoorOpening() {
#ifdef DEBUG
	Serial.print("sendBackDoorOpening - ");
	Serial.print(backDoorOpeningWdtCount);
	Serial.print(" - ");
	Serial.println(backDoorOpeningCount);
	delay(50);
#endif

	if (backDoorOpeningWdtCount > 0) {
		backDoorOpening = false;
		return;
	}
	backDoorOpeningWdtCount	= WDT_COUNT_SEND;

	if (backDoorOpeningCount > MAX_SEND) {
		backDoorOpening = false;
		return;
	}
	backDoorOpeningCount++;

	// SEND back_door_opening_count
	send(RFXSENSOR_BACK_DOOR_ID, backDoorOpeningCount);

	// RESET
	if (frontDoorOpeningCount > 0) {
		send(RFXSENSOR_FRONT_DOOR_ID, frontDoorOpeningCount = 0);
	}
	if (windowOpeningCount > 0) {
		send(RFXSENSOR_WINDOW_ID, windowOpeningCount = 0);
	}
	if (alarmSwitchCount > 0) {
		send(RFXSENSOR_ALARM_ID, alarmSwitchCount = 0);
	}

	backDoorOpening = false;
}

/**
 * Send front door opening count.
 */
void sendFrontDoorOpening() {
#ifdef DEBUG
	Serial.print("sendFrontDoorOpening - ");
	Serial.print(frontDoorOpeningWdtCount);
	Serial.print(" - ");
	Serial.println(frontDoorOpeningCount);
	delay(50);
#endif

	if (frontDoorOpeningWdtCount > 0) {
		frontDoorOpening = false;
		return;
	}
	frontDoorOpeningWdtCount	= WDT_COUNT_SEND;

	if (frontDoorOpeningCount > MAX_SEND) {
		frontDoorOpening = false;
		return;
	}
	frontDoorOpeningCount++;

	// SEND front_door_opening_count
	send(RFXSENSOR_FRONT_DOOR_ID, frontDoorOpeningCount);

	// RESET backdoor
	if (backDoorOpeningCount > 0) {
		send(RFXSENSOR_BACK_DOOR_ID, backDoorOpeningCount = 0);
	}

	frontDoorOpening = false;
}

/**
 * Send front window opening count.
 */
void sendWindowOpening() {
#ifdef DEBUG
	Serial.print("sendWindowOpening - ");
	Serial.print(windowOpeningWdtCount);
	Serial.print(" - ");
	Serial.println(windowOpeningCount);
	delay(50);
#endif

	if (windowOpeningWdtCount > 0) {
		windowOpening = false;
		return;
	}
	windowOpeningWdtCount	= WDT_COUNT_SEND;

	if (windowOpeningCount > MAX_SEND) {
		windowOpening = false;
		return;
	}
	windowOpeningCount++;

	send(RFXSENSOR_WINDOW_ID, windowOpeningCount);

	// RESET backdoor
	if (backDoorOpeningCount > 0) {
		send(RFXSENSOR_BACK_DOOR_ID, backDoorOpeningCount = 0);
	}

	windowOpening = false;
}


/**
 * Send alarmSwith (case opening).
 */
void sendAlarm() {
#ifdef DEBUG
	Serial.print("sendAlarm - ");
	Serial.print(alarmSwitchWdtCount);
	Serial.print(" - ");
	Serial.println(alarmSwitchCount);
	delay(50);
#endif

	if (alarmSwitchWdtCount > 0) {
		alarmSwitch = false;
		return;
	}
	alarmSwitchWdtCount	= WDT_COUNT_SEND;

	if (alarmSwitchCount > MAX_SEND) {
		alarmSwitch = false;
		return;
	}
	alarmSwitchCount++;

	// SEND
	send(RFXSENSOR_ALARM_ID, 1);

	alarmSwitch = false;
}


/**
 * Send battery level (in pourcentage).
 */
void sendBatteryLevel(int battery) {
	send(RFXSENSOR_BATTERY_ID, battery);
}


/**
 * Send RFXsensor value.
 */
void send(int id, int value) {
#ifdef DEBUG
	Serial.println("send...");
	delay(50);
#endif

#ifndef MUTE
	x10.RFXsensor(id, 't', 't', value);
	delay(50);
#endif
}


#ifdef DEBUG
void blink(int pin) {
	for (int count = 0; count < 5; count++) {
		digitalWrite(pin, HIGH);
		delay(25);
		digitalWrite(pin, LOW);
		delay(25);
	}
}
#endif
