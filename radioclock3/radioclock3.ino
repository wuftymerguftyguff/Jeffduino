#define wwvbPin 2
#define ledPin 13
#define LOW 0
#define HIGH 1

int pulseStart,pulseEnd,pulseWidth,seconds = 0;
boolean pulseStatus = false;
int sigWas = LOW;

void pulse() {
  int sig = digitalRead(wwvbPin) ;
    
  if(sig == HIGH && pulseStart == 0) {
    pulseStart = millis() ;
    return ;
  }

  // If we've going high after a low, this starts a pulse.  Get the time.
/*
  if(sig == HIGH && pulseStart == 0) {
    pulseStart = millis() ;
    return ;
  }
  
*/
/*
  // If we see a high but haven't seen a low, just ignore the transition.

  if(sig == HIGH) {
    return ;
  }
*/
  // If we haven't seen a start of pulse and have a low, just ignore.
/*
  if(pulseStart == 0) {
    return ;
  }
  */

  // We got a pulse.  Compute the duration.

  pulseEnd = millis() ;
  pulseWidth = pulseEnd - pulseStart ;
  pulseStart = 0 ;
  pulseStatus = !pulseStatus;
  if ( !pulseStatus ) {
    digitalWrite(ledPin,LOW);
  } else {
    digitalWrite(ledPin,HIGH);
  }

  Serial.print(millis()/1000);
  Serial.print("\t");  

  if (pulseWidth >= 500 ) {
  seconds = 0;
  Serial.print("top of minute");
  Serial.print("\t");  
  } else {
    seconds++;
  }
  Serial.print("Seconds: ");
  Serial.print(seconds);   
  Serial.print(" Pulse Width: ");
  Serial.println(pulseWidth);  // prints hello with ending line break 
 
}
  // This interrupt routine continues building a data frame and setting
  // the clock.
  
void setup() {
attachInterrupt(0, pulse, CHANGE) ;
Serial.begin(9600);           // set up Serial library at 9600 bps
Serial.println("Clock Starting");  // prints hello with ending line break 
pinMode(ledPin, OUTPUT); 
//digitalWrite(ledPin,HIGH);
}

void loop() {
}

