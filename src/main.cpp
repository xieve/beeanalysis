#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#define NOTHING 0
#define FRONT 1
#define BOTH 2
#define BACK 3

const int in1 = 10;
const int in2 = 11;
const int led1 = 13;
const int led2 = 12;
const int firstIRLED = 22;

long int in1cnt = 0;
long int in2cnt = 0;

int start_state = NOTHING;
int laststate = NOTHING;
int error = 0;

bool in1state = false;
bool in2state = false;

unsigned long milliscounter = 0;
int timeout = 1000; // how many millis until we time out

unsigned long pctimer = 0;
int pctimout = 1000; // how many millis until we print to screen

const char* modeConvert(int var)
{
	if (var == NOTHING) return ("NOTHING");
	else if (var == BACK) return ("BACK");
	else if (var == BOTH) return ("BOTH");
	else if (var == FRONT) return ("FRONT");
}

void setup()
{
	pinMode(in1, INPUT);
	pinMode(in2, INPUT);
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	Serial.begin(9600);
	if (!SD.begin())
	{
		Serial.println("initialization failed!");
	}
	File logfile = SD.open("log.csv", FILE_WRITE);
	if (logfile)
	{
		logfile.println("in,out");
		logfile.close();
	}
	pctimer = millis() + pctimout;
}

void loop()
{
	// can cause problems when millis overflows after "long" runtimes
	if (millis() > pctimer)
	{
		pctimer = millis() + pctimout;
		File logfile = SD.open("log.csv", FILE_WRITE);
		if (logfile)
		{
			logfile.print(in1cnt);
			logfile.print(",");
			logfile.println(in2cnt);
			logfile.close();
		}
		else
		{
			Serial.print("Not on SD: ");
		}
		Serial.print("time: ");
		Serial.print(millis()/1000);
		Serial.print("; in: ");
		Serial.print(in1cnt);
		Serial.print("; out: ");
		Serial.print(in2cnt);
		Serial.print("; in1state: ");
		Serial.print(in1state?"true":"false");
		Serial.print("; in2state: ");
		Serial.print(in2state?"true":"false");
		Serial.print("; laststate: ");
		Serial.print(modeConvert(laststate));
		Serial.print("; start_state: ");
		Serial.print(modeConvert(start_state));
		if (error == 1)
		{
			Serial.print(" Error 1: Did not notice escape");
			error = 0;
		}
		Serial.println("");
	}
	digitalWrite(firstIRLED, HIGH);
	delay(10);

	for (int curSlot; (curSlot < 24); curSlot++)
	{
		//sensorcheck
		in1state = digitalRead(in1) == LOW;
		in2state = digitalRead(in2) == LOW;
		digitalWrite(curSlot + firstIRLED, LOW);

		//turn next slot-IR-LED on
		digitalWrite(curSlot + firstIRLED + 1, HIGH);

		//led-ansteuerung
		if (in1state || in2state)
		{
			digitalWrite(led1, HIGH);
		}
		else
		{
			digitalWrite(led1, LOW);
		}
		if (in1state != in2state)
		{
			digitalWrite(led2, HIGH);
		}
		else
		{
			digitalWrite(led2, LOW);
		}

		//sensorenlogik + counter
		// Two possible ways things can happen
		// either a bee comes from one direction - or the other
		// first direction we have:
		//			NOTHING -> FRONT -> BOTH -> BACK -> NOTHING
		// other direction:
		//			NOTHING -> BACK -> BOTH -> FRONT -> NOTHING
		// Let in1state correspond to FRONT
		// Let in2state correspond to BACK

		// we should probably also include a default reset in case we
		// dont get all clean results
		// can cause problems when millis overflows after "_long_" runtimes

		/* if (millis() > milliscounter)
		{
			if (start_state > NOTHING)
			{
				start_state = NOTHING;
				laststate = NOTHING;
				error = 1;
			}
			milliscounter = 0;
			}*/

		if (start_state == NOTHING)
		{
			if (in1state && !in2state)
			{
				start_state = FRONT;
				laststate = FRONT;
			}
			else if (!in1state && in2state)
			{
				start_state = BACK;
				laststate = BACK;
			}
			else if (in1state && in2state)
			{
				start_state = BOTH;
				laststate = BOTH;
			}
		}
		else if (start_state > NOTHING)
		{
			if (start_state == FRONT)
			{
				if (in1state && in2state && (laststate == FRONT))
				{
					laststate = BOTH;
				}
				else if (!in1state && in2state && (laststate == BOTH))
				{
					laststate = BACK;
				}
				else if (!in1state && !in2state && (laststate == BACK))
				{
					// we have now passed all possible states -> therefore reset and increment
					// counter
					laststate = NOTHING;
					start_state = NOTHING;
					in1cnt++;
				}
			}
			else if (start_state == BACK)
			{
				if (in1state && in2state && (laststate == BACK))
				{
					laststate = BOTH;
				}
				else if (in1state && !in2state && (laststate == BOTH))
				{
					laststate = FRONT;
				}
				else if (!in1state && !in2state && (laststate == FRONT))
				{
					// we have now passed all possible states -> therefore reset and increment
					// counter
					laststate = NOTHING;
					start_state = NOTHING;
					in2cnt++;
				}
			}
		}
		if (!in1state && !in2state && start_state > NOTHING)
		{
			start_state = NOTHING;
			laststate = NOTHING;
			error = 1;
		}
	}
}
