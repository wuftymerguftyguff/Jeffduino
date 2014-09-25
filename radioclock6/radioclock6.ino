#define wwvbPin 2
#define ledPin 13
#define LOW 0
#define HIGH 1

//# define if we are DEBUGGING
//#define DEBUG 

// the time of this pulse
unsigned long currentMillis;
unsigned long previousMillis = 0; 
unsigned long interval = 1000;

//starting offset inside per second "byte"
int startingOffset = 0;
// starting state of the LED
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

void setBits(int width,int level) {
  
  int numBitsToSet = max(width/100,1);  // Each "bit" is 100ms, but we have to set at least one (one bit pulses are oten reported as < 100 ms !!
  int bitOffset = startingOffset;       // Which bit should we set next?
  startingOffset += numBitsToSet;       // what bit should we start with the next time we are called?
  
  if ( level == LOW ) return;  //return immediately for low bits as these are defaulted to 1 anyway
  
  
  for (int i=0;i < numBitsToSet;i++) {
    //Serial.print(" setting bit ");
    //Serial.print(bitOffset);
    //Serial.print(" of ");
    //Serial.print(numBitsToSet);
    //Serial.print(" bits "); 
    second[bitOffset] = true;
    bitOffset++; 
  } 
  
 
  
}

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
  
  // do the generic stuff that applies for any pulse
  pulseChange();
  
#ifdef DEBUG  
  Serial.print("RISING ");
  Serial.print("HIGH: ");
  Serial.print(pulseWidth);
  Serial.print(" ");
#endif  

  setBits(pulseWidth,HIGH);
}

void fallingPulse() {
  
  // do the generic stuff that applies for any pulse
  pulseChange(); 
  
#ifdef DEBUG  
  Serial.print("FALLING ");
  Serial.print("LOW: ");
  Serial.print(pulseWidth);
  Serial.print(" ");
#endif  
  
  setBits(pulseWidth,LOW);
  
  if ( lastPulseWidth >= 450 && pulseWidth >= 450 ) {
    TOM = true;
  }
  if ( pulseWidth >= 450 ) {
    TOS = true;
  }
}

  
void setup() {
// this looks reversed as the module reversed the serial output of the carrier state
attachInterrupt(0, risingPulse, FALLING) ;
attachInterrupt(1, fallingPulse, RISING) ;


Serial.begin(115200);           // set up Serial library at 19200 bps
Serial.println("Clock Starting");  // Say something to show we have restarted.
pinMode(ledPin, OUTPUT); // set up the ledpin
// set all the values of the current second to 1

// as msf signal is on by default and is turned off to "send data"
memset(&second[0], 0x00, sizeof(second));
}

void loop() {
  
   if ( TOM == true ) { 
    Serial.print("\nTOM *************\n"); 
    TOM = false;
    //previousMillis = millis();
  }
  
  if ( TOS == true ) {
    
    
#ifdef DEBUG
    // loop thru the 10 bits in the prior second and print them ou
    Serial.print(" ");
    for (int i=0;i<=9;i++) { 
      Serial.print(second[i]); 
    }
    
#endif
    Serial.print(" ");
    //Display the A and B Bits
    for (int i=1;i<=2;i++) { 
      Serial.print(second[i]); 
    }
    
#ifdef DEBUG
    Serial.print("\nTOS "); 
#endif 

    TOS = false;
    memset(&second[0], 0x00, sizeof(second));
    startingOffset = 0;
  }
  
 
 
}

