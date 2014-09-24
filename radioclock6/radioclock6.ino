#define wwvbPin 2
#define ledPin 13
#define LOW 0
#define HIGH 1

unsigned long currentMillis;
unsigned long previousMillis = 0; 
unsigned long interval = 1000;
int ledState = LOW;

// the bit number of the current data
volatile unsigned long secondBitOffset;

// time of this pulse state change
long thisPulseChange = 0;
long lastPulseChange = 0;

// an array to store the bits in the current second
// in msf each second is cplit into 10 100ms "bits"

bool second[10];

// a boolean to show if we are at the top of the minute
bool TOM = false;
bool TOS = false;

// how long we aim to pause the main loop (ms)
long plannedDelay = 25;

// hw long it takes to go around the main loop (we assume perfect timing as a starting point
long mainElapsed = 0;

// how long we need to delay to get the planned delay in reality
long actualDelay = plannedDelay;

long pulseStart,pulseEnd,pulseTimeLow,pulseTimeHigh,seconds = 0;

long pulseWidth;
long lastPulseWidth;

boolean pulseStatus = false;
boolean childPulse = false;
long lastPulse = 0;
int sigWas = LOW;
int carrierState;

int secondMillis;

int pulseTime = 0;

long startTimeHigh = 0;
long startTimeLow = 0;

void pulseChange() {
  // save the timestamp of the last pulse interrupt
  lastPulseChange = thisPulseChange;
  
  // grab the start time of this pulse interrupt
  thisPulseChange = millis();
  
  // save the duration of the last pulse
  lastPulseWidth = pulseWidth;
  
  //calculate the duration of this pulse
  pulseWidth = thisPulseChange - lastPulseChange; 
  
  //toggle the state of the led
  ledState = !ledState;
  
  // set the LED with the ledState of the variable:
  digitalWrite(ledPin, ledState);
}

void risingPulse() {
  pulseChange();
  secondBitOffset = ((currentMillis - previousMillis) )/ 100;
  second[secondBitOffset] = false;
  //print the length of the last low pulse
  //Serial.print("RISING");
  Serial.print("LOW: ");
  Serial.print(pulseWidth);
  Serial.print(" ");
}

void fallingPulse() {
  pulseChange(); 
  Serial.print("HIGH: ");
  Serial.print(pulseWidth);
  if ( lastPulseWidth >= 450 && pulseWidth >= 450 ) {
    TOM = true;
  }
  if ( pulseWidth >= 450 ) {
    TOS = true;
  }

    //print the length of the last high pulse
  //Serial.println(secondBitOffset);
  //Serial.print("HIGH: ");
  
  //Serial.println(pulseWidth);
}

  
void setup() {
//attachInterrupt(0, pulseChange, CHANGE) ;
// this looks reversed as the module reversed the serial output of the carrier state
attachInterrupt(0, risingPulse, FALLING) ;
attachInterrupt(1, fallingPulse, RISING) ;


Serial.begin(19200);           // set up Serial library at 19200 bps
Serial.println("Clock Starting");  // Say something to show we have restarted.
pinMode(ledPin, OUTPUT); // set up the ledpin
// set all the values of the current second to 1

// as msf signal is on by default and is turned off to "send data"
memset(&second[0], 0x01, sizeof(second));
}

void loop() {
  
   if ( TOM == true ) {  
    Serial.print("\nTOM *************\n");  
    TOM = false;
    previousMillis = millis();
  }
  
  if ( TOS == true ) {  
    Serial.print("\nTOS ");  
    TOS = false;
    memset(&second[0], 0x01, sizeof(second));
  }
  
 
 
}

