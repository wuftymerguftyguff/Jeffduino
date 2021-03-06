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
  //Serial.println("CHANGE");
  lastPulseChange = thisPulseChange;
  thisPulseChange = millis();
  lastPulseWidth = pulseWidth;
  pulseWidth = thisPulseChange - lastPulseChange; 
  //secondBitOffset = secondMillis/100;
  //Serial.println(secondBitOffset);
  
  ledState = !ledState;
    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
}

void risingPulse() {
  pulseChange();
  secondBitOffset = ((currentMillis - previousMillis) )/ 100;
  second[secondBitOffset] = false;
  //print the length of the last low pulse
  //Serial.println("RISING");
  //Serial.print("LOW: ");
  Serial.print(pulseWidth);
  Serial.print(" ");
}

void fallingPulse() {
  if ( lastPulseWidth >= 450 && pulseWidth >= 450 ) {
    TOM = true;
  }
  if ( lastPulseWidth >= 450 ) {
    TOS = true;
  }
  pulseChange(); 
    //print the length of the last high pulse
  //Serial.println(secondBitOffset);
  //Serial.print("HIGH: ");
  
  //Serial.println(pulseWidth);
}



 
  // This interrupt routine continues building a data frame and setting
  // the clock.
  
void setup() {
//attachInterrupt(0, pulseChange, CHANGE) ;
// this looks reversed asthe module reversed the serial output of the carrier state
attachInterrupt(0, risingPulse, FALLING) ;
attachInterrupt(1, fallingPulse, RISING) ;
Serial.begin(57600);           // set up Serial library at 9600 bps
Serial.println("Clock Starting");  // prints hello with ending line break 
pinMode(ledPin, OUTPUT); 
// set all the values of the current second to 1
// as msf signal is on by default and is turned off to "send data"
memset(&second[0], 0x01, sizeof(second));
}

void loop() {
  
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the 
  // difference between the current time and last time you blinked 
  // the LED is bigger than the interval at which you want to 
  // blink the LED.
  
  currentMillis = millis();
 
  if(currentMillis - previousMillis > interval || TOM || TOS) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   
    //Serial.println("Reset second Template ");
    Serial.println(" ");
    for (int i=0;i<10;i++) {
      Serial.print(i);
      Serial.print(":");
      Serial.print(second[i]);
      Serial.print("  ");
    }
 
    memset(&second[0], 0x01, sizeof(second));
    previousMillis = currentMillis; 
    //do this every interval
  }
  
  if ( TOS == true ) {  
    Serial.print("TOS ");  
    TOS = false;
    previousMillis = millis();
  }
  
  if ( TOM == true ) {  
    Serial.print("TOM ");  
    TOM = false;
    previousMillis = millis();
  }
 
}

