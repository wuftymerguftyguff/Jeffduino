#include <MsfTimeLib.h>
#include <Time.h>

#define LED_PIN 13
#define MSF_PADDING 0

bool inSync = false;
byte previousSecond = 0;

char* days[7] = {
"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

MsfTimeLib msf;


void setup()
{
	Serial.begin(9600);

// Arduino Interrupt (0 or 1)
// Output mode false = dec - true = bcd,
// carrierOff = true for HIGH pulse on Carrier Off
// ledPin = pin to flash led on (0 = no led)
// padding in ms if your MSF receiver gives pulse shorter than 100ms

	msf.begin(MSF_INT_1,MSF_PULSE_HIGH,MSF_DEC_OUT,LED_PIN,MSF_PADDING);  // msf.begin(Interrupt Number, Output Mode, Carrier Off, LED Pin, Padding

}

void loop()
{
// when the time and date data is valid we also check to see if the next minute has started
// by looking for the "startOfSecond" flag. We have up to 500 ms to display the data before
// the next interrupt. The "msfTimeAvailable" flag is set up to 700 ms before the start of the
// minute so it's worth waiting, Assuming that writing the time and date data to the RTC or even the
// Time library resets the clock, it is very important to do the writing as soon as possible after
// the start of the first second of the minute. "msfTimeAvailable" is set during second 59 of the previous
// minute and "startOfSecond" is set at the start of the first second on the next minute. Put them
// together and we get pretty close to the precise time. This example synchronises every minute if
// the data is valid.

// set the time in the Time library. Do a double check to make sure that the data is valid
// the Time library performs all the "clock" functions for us
	
  if(msf.TimeAvailable && msf.startOfSecond)
    {
      setTime(msf.Hour,msf.Minute,0,msf.Date,msf.Month,msf.Year);
      msf.TimeAvailable = false;	// clear the flag
      msf.startOfSecond = false;	// clear the flag
      inSync = true;
    }
// display the time every time the seconds change
  if(previousSecond != second())
    {
      DisplayTime();
      previousSecond = second();
      DisplayDate();
// just for comfort show that the sync has happened
      if(inSync) Serial.print(" (Synchronised)");
      inSync = false;
    }
}

void DisplayTime()
{
// Display the Hour, Minute and Seconds for Serial
	Serial.println();
	Serial.print(hour()<10?"0":"");
    Serial.print(hour());
	Serial.print(":");
	Serial.print(minute()<10?"0":"");
	Serial.print(minute());
	Serial.print(":");
	Serial.print(second()<10?"0":"");
	Serial.print(second());
// display UTC or BST
	if(msf.Bst)	Serial.print(" BST");
	else Serial.print(" UTC");
}

void DisplayDate()
{
// display the Day, Date, Month and Year for Serial
	Serial.print(" ");
	Serial.print(days[weekday() - 1]);
	Serial.print(day()<10?"0":"");
	Serial.print(day());
	Serial.print("/");
	Serial.print(month()<10?"0":"");
	Serial.print(month());
	Serial.print("/");
	Serial.print(year());

// display UTC and UT1 data
	Serial.print(" <> UTC = UT1 ");
	if(msf.DutPos)
	{
      Serial.print("- ");
	  Serial.print(msf.DutPos);
	  Serial.print("ms");
	}
	else
	{
	  Serial.print("+ ");
	  Serial.print(msf.DutNeg);
	  Serial.print("ms");
	}
}
