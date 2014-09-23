attachInterrupt(0, pulse, CHANGE) ;

void pulse() {
  int sig = digitalRead(wwvbPin) ;

  // If we've going high after a low, this starts a pulse.  Get the time.

  if(sig == HIGH && pulseStart == 0) {
    pulseStart = millis() ;
    return ;
  }

  // If we see a high but haven't seen a low, just ignore the transition.

  if(sig == HIGH) {
    return ;
  }

  // If we haven't seen a start of pulse and have a low, just ignore.

  if(pulseStart == 0) {
    return ;
  }

  // We got a pulse.  Compute the duration.

  pulseEnd = millis() ;
  pulseWidth = pulseEnd - pulseStart ;
  pulseStart = 0 ;

  // This interrupt routine continues building a data frame and setting
  // the clock.

