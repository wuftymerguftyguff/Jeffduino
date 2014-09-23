#define wwvbPin 2
#define ledPin 13
#define LOW 0
#define HIGH 1

long pulseStart,pulseEnd,pulseTimeLow,pulseTimeHigh,seconds = 0;
boolean pulseStatus = false;
boolean childPulse = false;
long lastPulse = 0;
int sigWas = LOW;
int carrierState;

int pulseTime = 0;

long startTimeHigh = 0;
long startTimeLow = 0;


void pulse() {
  //get the state of the carrier pin
  carrierState =  digitalRead(wwvbPin);
  
  //the real carrier state is the inverse
  carrierState = !carrierState;
  
  pulseStart = millis();
  
  if ( carrierState == HIGH ) {
    int pulseWidthLow;
    startTimeHigh = pulseStart;
    pulseWidthLow = millis() - startTimeLow;
    pulseTimeLow += pulseWidthLow;
    Serial.print("Carrier Low for: ");
    Serial.println(pulseTimeLow);

  } else if ( carrierState == LOW )  {
    int pulseWidthHigh;
    startTimeLow = pulseStart;
    pulseWidthHigh = millis() - startTimeHigh;
    pulseTimeHigh += pulseWidthHigh;

  }


}


 
  // This interrupt routine continues building a data frame and setting
  // the clock.
  
void setup() {
attachInterrupt(0, pulse, CHANGE) ;
Serial.begin(57600);           // set up Serial library at 9600 bps
Serial.println("Clock Starting");  // prints hello with ending line break 
pinMode(ledPin, OUTPUT); 
//digitalWrite(ledPin,HIGH);
}

void loop() {
  //delay(100);
  //Serial.print(carrierState);
  //Serial.print("\t");
  //Serial.println(millis()-pulseStart);
}

