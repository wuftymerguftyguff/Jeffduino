#define wwvbPin 2
#define ledPin 13
#define LOW 0
#define HIGH 1

// an array to store the bits in the current second
// in msf each second is cplit into 10 100ms "bits"

bool second[10];

long pulseStart,pulseEnd,pulseTimeLow,pulseTimeHigh,seconds = 0;
boolean pulseStatus = false;
boolean childPulse = false;
long lastPulse = 0;
int sigWas = LOW;
int carrierState;

int secondMillis;

int pulseTime = 0;

long startTimeHigh = 0;
long startTimeLow = 0;


void pulse() {
  //get the state of the carrier pin
  carrierState =  digitalRead(wwvbPin);
  
  //the real carrier state is the inverse
  carrierState = !carrierState;
  
  pulseStart = millis();
  
  int secondBitOffset = secondMillis/100;
  
  if ( pulseTime >= 1000 ) {
    pulseTime = 0;
    pulseTimeHigh = 0;
    pulseTimeLow = 0;
  }
  
  if ( carrierState == HIGH ) {
    int pulseWidthLow;
    startTimeHigh = pulseStart;
    pulseWidthLow = millis() - startTimeLow;
    pulseTimeLow += pulseWidthLow;
    if ( pulseWidthLow >= 450) {
      Serial.println("TOM");  
    }
    //Serial.print("Carrier Low for: ");
    //Serial.println(pulseTimeLow);
    second[secondBitOffset] = true;

  } else if ( carrierState == LOW )  {
    int pulseWidthHigh;
    startTimeLow = pulseStart;
    pulseWidthHigh = millis() - startTimeHigh;
    pulseTimeHigh += pulseWidthHigh;
    second[secondBitOffset] = false;

  }

  pulseTime += pulseTimeLow;
  pulseTime += pulseTimeHigh;
  
  //Serial.println(secondMillis/100);
  
}


 
  // This interrupt routine continues building a data frame and setting
  // the clock.
  
void setup() {
attachInterrupt(0, pulse, CHANGE) ;
Serial.begin(57600);           // set up Serial library at 9600 bps
Serial.println("Clock Starting");  // prints hello with ending line break 
pinMode(ledPin, OUTPUT); 
// set all the values of the current second to 1
// as msf signal is on by default and is turned off to "send data"
memset(&second[0], 0x01, sizeof(second));
}

void loop() {
  delay(10);
  secondMillis += 10 ;
  
  if (secondMillis >= 1000) {
  //Serial.print("Carrier Low for: ");
  //Serial.println(pulseTimeLow);
  //Serial.print("Carrier High for: ");
  //Serial.println(pulseTimeHigh);
  //Serial.println(pulseTime);
  //Serial.print(carrierState);
  //Serial.print("\t");
  for (int i=0;i<10;i++) {
    Serial.print(second[i]);
  }
  Serial.println("Again..");
  secondMillis = 0;
  memset(&second[0], 0x01, sizeof(second));
  }
}

