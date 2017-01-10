#include <Arduino.h>

// roessner.pascal@gmail.com

#define NOTHING 0
#define FRONT 1
#define BOTH 2
#define BACK 3

const int in1 = 10;
const int in2 = 11;
const int led1 = 13;
const int led2 = 12;

long int in1cnt = 0;
long int in2cnt = 0;

int start_state = NOTHING;
int laststate = NOTHING;

bool in1state = false;
bool in2state = false;

unsigned long milliscounter = 0;
int timeout = 5000; // how many millis until we time out

unsigned long pctimer = 0;
int pctimout = 1000; // how many millis until we print to screen


void setup() {
	pinMode(in1, INPUT);
	pinMode(in2, INPUT);
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	Serial.begin(9600);
  pctimer = millis() + pctimout;
}

void loop() {
	 //stati an computer
   // can cause problems when millis overflows after "long" runtimes
   if (millis() > pctimer) {
    pctimer = millis() + pctimout;
		Serial.print("Time: ");
		Serial.print(millis()/1000);
		Serial.print(", Amount: in1: ");
		Serial.print(in1cnt);
		Serial.print("; in2: ");
		Serial.print(in2cnt);
		Serial.print("; laststate: ");
    Serial.println(laststate);
	}

	//sensorencheck
  in1state = digitalRead(in1) == LOW;
	in2state = digitalRead(in2) == LOW;

	//led-ansteuerung
	if (in1state) {
		digitalWrite(led1, HIGH);
	}else{
		digitalWrite(led1, LOW);
	}
	if (in2state) {
		digitalWrite(led2, HIGH);
	}else{
		digitalWrite(led2, LOW);
	}

	//sensorenlogik + counter
  // Two possible ways things can happen
  // either a bee comes from one direction - or the other
  // first direction we have:
  //      NOTHING -> FRONT -> BOTH -> BACK -> NOTHING
  // other direction:
  //      NOTHING -> BACK -> BOTH -> FRONT -> NOTHING
  // Let in1state correspond to FRONT
  // Let in2state correspond to BACK

  // we should probably also include a default reset in case we
  // dont get all clean results
  // can cause problems when millis overflows after "_long_" runtimes
  if (millis() > milliscounter) {
    if (start_state > NOTHING) {
      start_state = NOTHING;
      laststate = NOTHING;
      Serial.print("Reset based on timeout\n");
    }
    milliscounter = 0;
  }


  if (in1state && !in2state && start_state == NOTHING) {
  	start_state = FRONT;
    laststate = FRONT;
    milliscounter = millis() + timeout;
  }
  else if (!in1state && in2state && start_state == NOTHING) {
    start_state = BACK;
    laststate = BACK;
    milliscounter = millis() + timeout;
  }

  if (start_state > NOTHING) {
    if (start_state == FRONT) {
      if (in1state && in2state && laststate == FRONT) laststate = BOTH;
      else if (!in1state && in2state && laststate == BOTH) laststate = BACK;
      else if (!in1state && !in2state && laststate == BACK) {
        // we have now passed all possible states -> therefore reset and increment
        // counter
        laststate = NOTHING;
        start_state = NOTHING;
        in1cnt ++;
      }
    }
    else if (start_state == BACK) {
      if (in1state && in2state && laststate == BACK) laststate = BOTH;
      else if (!in1state && in2state && laststate == BOTH) laststate = FRONT;
      else if (!in1state && !in2state && laststate == FRONT) {
        // we have now passed all possible states -> therefore reset and increment
        // counter
        laststate = NOTHING;
        start_state = NOTHING;
        in2cnt ++;
      }
    }
  }

  // not sure we need this? should have this?
  // since we don't have analog pins -- dont think we really
  // need to wait but we will just keep this short wait
  delayMicroseconds(10);
	// delay(1);
}
