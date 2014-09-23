//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Serial MSF Test Clock (C) Copyright 2014 Phil Morris					    //
//											    //
// a "Proof of Concept" sketch for the Arduino Uno Rev 3 and an MSF Clock module	    //
// No apologies given for the messy code as this is a sketch simply to show		    //
// some ideas around decoding MSF time signals. The code takes account of Leep		    //
// seconds and performs full parity checking. A simple output is provided to the	    //
// Serial port at 9600 baud. BST/GMT is decoded along with UTC & DUT difference.	    //
// even partial data reception can produce a valid output as long as seconds 17 thru 59	    //
// are received correctly.								    //
//																							//
// For diagnostic purposes, the minimum and maximum pulse lengths are also displayed in ms  //
//											    //
// You may use, modify and/or distribute this sketch as long as you leave this text intact. //
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

const int CARRIER_OFF = HIGH;				// this constant is set for inverted MSF receiver output
											// invert it if your output = LOW for carrier off

const int MSFPIN = 3;						// the Arduino pin for the MSF device (2 or 3)
const int PULSE_IGNORE = 90;				// minimum acceptable pulse length in ms
const int PADDING = 0;						// added to pulse length if necessary if RX gives short pulses

volatile long pulseStart = 0;				// milliseconds when start of pulse occurred
volatile long pulseEnd = 0;					// milliseconds when pulse ended
volatile long lastPulseStart = 0;			// the previous pulse start value
volatile int pulseLength = 0;				// length of pulse/100 as an integer
volatile bool bitBonly = false;				// set if a 'B' only pulse detected
volatile byte secondBits = 0;				// bits decoded from seconds
volatile int bitPointer = 0;				// pointer for bits within buffer bytes
volatile byte bitCounter = 0;				// used to count number of "1"s in chunk routines
volatile bool msfStartOfSecond = 0;			// set at start of second pulse, reset at end of second pulse
volatile bool msfTimeAvailable = false;		// set when time and date data has been decoded successfully

volatile byte msfYear = 0;					// year
volatile byte msfMonth = 0;					// month
volatile byte msfDate = 0;					// date
volatile byte msfDay = 0;					// weekday
volatile byte msfHour = 0;					// hour
volatile byte msfMinute = 0;				// minute
volatile bool msfBst = 0;					// 1 = BST, 0 = GMT
volatile bool msfBstSoon = 0;				// 1 = BST imminent
volatile int LED_PIN = 13;					// pin LED is attached (optional)

byte aBuffer[8];							// buffer for 'A' bits
byte bBuffer[8];							// buffer for 'B' bits

volatile int min100 = 150;					// used for min & max pulse length storage
volatile int min200 = 250;					// -"-
volatile int min300 = 350;					// -"-
volatile int min500 = 550;					// -"-

volatile int max100 = 0;					// -"-
volatile int max200 = 0;					// -"-
volatile int max300 = 0;					// -"-
volatile int max500 = 0;					// -"-
volatile int minMaxPulse = 0;


char* days[7] = {
  "Sun","Mon","Tue","Wed","Thu","Fri","Sat"};	// array for days of week

char* parityError[5] = {						// array for parity error text
"","\"Year\"","\"Month-Date\"","\"Weekday\"","\"Hour-Minute\""};

void setup()
{
  Serial.begin(9600);								// start the Serial port
  attachInterrupt(MSFPIN - 2, MsfPulse, CHANGE);	// set the interrupt
  pinMode(LED_PIN,OUTPUT);							// configue the LED pin
  memset(&aBuffer[0], 0xFF, sizeof(aBuffer));		// clear the "A" buffer to "1"s
  memset(&bBuffer[0], 0, sizeof(bBuffer));			// clear the "B" buffer	to "0"s
}

void loop()
{
// when the time and date data is valid we also check to see if the next minute has started
// by looking for the "startOfSecond" flag. We have up to 500 ms to display the data before
// the next interrupt. The "newTimeAvailable" flag is set up to 700 ms before the start of the
// minute so it's worth waiting, Assuming that writing the time and date data to the RTC or even the
// Time library resets the clock, it is very important to do the writing as soon as possible after
// the start of the first second of the minute. "newTimeAvailable" is set during second 59 of the previous
// minute and "startOfSecond" is set at the start of the first second on the next minute. Put them
// together and we get pretty close to the precise time.

  if(msfTimeAvailable && msfStartOfSecond)
  {
	    msfTimeAvailable = false;	// clear the flag
		msfStartOfSecond = false;		// clear the flag
// display the time
		Serial.println();
		Serial.print(msfHour<10?"0":"");
		Serial.print(msfHour);
		Serial.print(":");
		Serial.print(msfMinute<10?"0":"");
		Serial.print(msfMinute);
		Serial.print(":00");
// get the BST bit status from the "B" buffer
    if(msfBst)					// display BST status
	    {
			Serial.print(" BST");
		}
    else
		{
			Serial.print(" UTC");
		}
// get the status of the BST imminent bit in the "B" buffer
	if(msfBstSoon) Serial.print(" | <BST Imminent>");	// display if BST is imminent
// display the date
	Serial.print(" ");
	Serial.print(days[msfDay]);
	Serial.print(" ");
	Serial.print(msfDate<10?"0":"");
	Serial.print(msfDate);
	Serial.print("/");
	Serial.print(msfMonth<10?"0":"");
	Serial.print(msfMonth);
	Serial.print("/20");
	Serial.print(msfYear);
// set up the UTC output
	Serial.print(" | MSF(UTC) = UT1 ");
// get the first DUT1 byte form the "B" buffer
	GetChunkB(1,8);													// get the DUT1+ "1"s bit count
	if(bitCounter)
	{
	 Serial.print("+ ");
	 Serial.print((bitCounter * 100));
	 Serial.print("ms");	// display count * 100
	 }
// get the second DUT1 byte from the "B" buffer
	GetChunkB(9, 8);												// get the DUT1- "1"s count
	if(bitCounter)
	{
	 Serial.print("- ");
	 Serial.print((bitCounter * 100));
	 Serial.println("ms");	// display count * 100
	 }
// now print the minimum and maximum pulse lengths
	Serial.println();
	Serial.println("Pulse    Min   Max");
	Serial.println("------------------");
	Serial.print("Start:   ");
	Serial.print(min500);
	Serial.print("   ");
	Serial.println(max500);
	Serial.print("100ms:   ");
	Serial.print(min100<100?"0":"");
	Serial.print(min100);
	Serial.print("   ");
	Serial.println(max100);
	Serial.print("200ms:   ");
	Serial.print(min200);
	Serial.print("   ");
	Serial.println(max200);
	Serial.print("300ms:   ");
	Serial.print(min300);
	Serial.print("   ");
	Serial.println(max300);


  }
}

void MsfPulse()	// interrupt routine which is called every time the MSF receiver output changes state
{
// This routine is called every time the selected Interrupt pin changes state. If it is the start of
// a pulse, the millis count is stored in "pulseStart". If it is the end of a pulse the millis count
// is stored in "pulseEnd". "pulseLength" is the result in millis/100. The data is processed to produce an
// integer 1 - 5 representing 100 - 500 ms pulses (no "4" is decoded). "secondBits" contains the binary data
// for the "A" and "B" buffer contents. "secondBits" data is written as it is detected so, even a double "B"
// pulse is written as an "A" bit first. When the second "B" bit is detected, the "bitBonly" flag is set
// during the current second, and the pointer to the buffers is decremented one position which overwrites
// the previous data.

  bitBonly = false;						// clear the bitOnly flag
  secondBits = 0;							// clear the secondBits variable
  bool pinState = digitalRead(MSFPIN);	// get the state of the interrupt pin

// is this a pulse start?

  if (pinState == CARRIER_OFF)	// pulse or sub-pulse of has started, carrier going off
	{
		pulseStart = millis();		// pulseStart = current millis everytime the MSFPIN goes low
		msfStartOfSecond = true;		// set this flag for later use
		digitalWrite(LED_PIN,HIGH);	// turn on LED
		return;						// until there's a another interrupt change
	}

// is it a pulse end?

  if(pinState != CARRIER_OFF)								// pulse end, carrier going on
	{
		pulseEnd = (millis() + PADDING);								// set the pulse end ms
		msfStartOfSecond = false;								// clear the start of second flag
		minMaxPulse = pulseEnd - pulseStart;				// actual length in millis
		pulseLength = abs((minMaxPulse) / 100);				// get the pulse length in ms/100
		pulseStart = millis();								// set the pulseStart to current millis
		if (!pulseLength) return;							// if the pulse is too short ("0"), return

// if the sequence was 100ms off + 100ms on + 100ms off, this is a 'B' only bit
// so, if this start pulse is less than 300ms after the last start pulse it must be
// a double 100ms pulse second

		if(pulseStart - lastPulseStart < 300) bitBonly = true;	// this is a 'B' bit
	    lastPulseStart = pulseStart;				// keep the last pulse start ms count
		digitalWrite(LED_PIN,LOW);					// turn off the LED
	}

 switch(pulseLength)	// start processing the valid pulse
	{
		case 5:	// check for start pulse i.e. 500ms
			{
				if(minMaxPulse > 490 && minMaxPulse < 550)
				{
					if(minMaxPulse < min500) min500 = minMaxPulse;
					if(minMaxPulse > max500) max500 = minMaxPulse;
				}
				Serial.println();
				Serial.print("Start: ");					// output start of line
				bitPointer = 0;								// clear the buffer bit pointer
				memset(&aBuffer[0], 0xFF, sizeof(aBuffer));	// clear the "A" buffer to all "1"s
				memset(&bBuffer[0], 0, sizeof(bBuffer));	// clear the "B" buffer
				break;
			}
		case 4:	// in the unlikely event we get a "4" quit
			{
				return;
			}
		case 3:	// check for 300ms pulse, this is an 'A' + 'B' bit case
			{
				if(minMaxPulse > 290 && minMaxPulse < 350)
				{
					if(minMaxPulse < min300) min300 = minMaxPulse;
					if(minMaxPulse > max300) max300 = minMaxPulse;
				}
				secondBits = 0b11;	// both "A" and "B" bits are set
				break;
			}
		case 2:	// check for 200ms pulse, this is an 'A' bit case
			{
				if(minMaxPulse > 190 && minMaxPulse < 250)
				{
					if(minMaxPulse < min200) min200 = minMaxPulse;
					if(minMaxPulse > max200) max200 = minMaxPulse;
				}
				secondBits = 0b01;	// only the "A" bit is set
				break;
			}
		case 1:	// check for 100ms pulse
			{
				if(minMaxPulse > 90 && minMaxPulse < 150)
				{
					if(minMaxPulse < min100) min100 = minMaxPulse;
					if(minMaxPulse > max100) max100 = minMaxPulse;
				}
				secondBits = 0b00;
				if(bitBonly) secondBits = 0b10;		// only the "B" bit is set
				break;
			}
	}


// store the data in the arrays
  if(pulseLength < 5)  // only pass this point if it's not a "Start" pulse e.g. < 500ms
	{
		bitPointer++;						// increment the bit pointer which allways starts at 1
		if(bitBonly) bitPointer--;			// if this is a "B" bit, decrement the bit pointer as
											// we have already written 0 bits to both the "A" and "B"
											// buffers
		// store the bit in the "A" buffer
			bitWrite(aBuffer[abs(bitPointer/8)], (bitPointer % 8) ^ 0x07, bitRead(secondBits,0));
		// store the bit in the "B" buffer
			bitWrite(bBuffer[abs(bitPointer/8)], (bitPointer % 8) ^ 0x07, bitRead(secondBits,1));

		if(bitBonly) Serial.print("\b");		// backspace the serial monitor (not on Arduino IDE)

		Serial.print(secondBits);				// output the second data to the serial port

// we detect the last second of the minute by looking for the binary sequence "01111110" in the "A" buffer
// bits 52 thru 59. If we see this sequence it's time to stop decoding and start working on the data received

    if(GetChunkA(bitPointer - 7, 8) == 0x7E)	// look for "01111110" sequence in "A" buffer
		{										// which indicates that the last second data has been received
												// and round it up for comfort as we've been using it -1
			Serial.print(" ");
			Serial.print((bitPointer + 1));
			Serial.print(" Seconds");		// display the "bitPointer" at end of data stream
			msfTimeAvailable = false;								// clear the flag
			byte parityResult = GetParity();						// check the parity of the data, Good = 0

// if the parity is OK, get the data from the "A" buffer into the variables. The MSF data is in BCD so we convert
// it to decimal here but you don't have to conver it. You would leave it as BCD for a RTC such as the DS1307

			if(!parityResult)
				{
					msfYear = bcdToDec(GetChunkA(bitPointer - 42,8));		// year	(offset, number of bits to read)
					msfMonth = bcdToDec(GetChunkA(bitPointer - 34,5));		// month
					msfDate = bcdToDec(GetChunkA(bitPointer - 29,6));		// date
					msfDay = bcdToDec(GetChunkA(bitPointer - 23,3));		// weekday
					msfHour = bcdToDec(GetChunkA(bitPointer - 20,6));		// hour
					msfMinute = bcdToDec(GetChunkA(bitPointer - 14,7));		// minute
					msfBst = GetBbit(bitPointer - 1);						//BST = 1, GMT = 0
					msfBstSoon = GetBbit(bitPointer - 6);					// BST imminent = 1
					msfTimeAvailable = true;
				}
			else
				{
				// if the parity check failed, output the point where the parity check failed
					Serial.println();
					Serial.print("Parity bad on ");
					Serial.print(parityError[parityResult]);
					Serial.print(" data!");
				}
		}
  }
}

byte GetChunkA(int start, int numBits)
{
// "A" buffer: return the byte 'chunk' containing the number of bits read at the starting
// bit position in the aRawBuffer up to 8 bits. returns a byte

  byte chunk = 0;
  byte bitVal = 0;
  int counter = numBits - 1;
  bitCounter = 0;				// a count of the number of "1" bits

  for(int i = start;i < start + numBits;i++)					// loop for numBits
	{
		bitVal = bitRead(aBuffer[abs(i/8)], (i % 8) ^ 0x07);	// get the bit from the buffer
		bitWrite(chunk,counter,bitVal);							// write the bit to "chunk"
		if(bitVal) bitCounter++;								// if it's a "1" increment the bitCounter
		counter--;												// decrement the counter
	}
  return chunk;
}

byte GetChunkB(int start, int numBits)
{
// "B" buffer: return the byte 'chunk' containing the number of bits read at the starting
// bit position in the bRawBuffer upto 8 bits. returns a byte

  byte chunk = 0;
  byte bitVal = 0;
  int counter = numBits - 1;
  bitCounter = 0;				// a count of the number of "1" bits

  for(int i = start;i < start + numBits;i++)					// loop for numBits
	{
		bitVal = bitRead(bBuffer[abs(i/8)], (i % 8) ^ 0x07);	// get the bit from the buffer
		bitWrite(chunk,counter,bitVal);							// write the bit to "chunk"
		if(bitVal) bitCounter++;								// if it's a "1" increment the bitCounter
		counter--;												// decrement the counter
	}

  return chunk;
}

byte GetParity()
{
// calculate the parity bits and return true if all's well
// all data and parity bits are relative to the last second count copntained in "bitPointer"
// this allows for leep seconds which are added or removed at second 16 i.e. before
// the actual date & time data. There can be 59, 60 or 61 seconds in a leep minute

  if(!CheckParity(bitPointer - 42, 8, bitPointer - 5)) return 1;	// Year data parity check
  if(!CheckParity(bitPointer - 34, 11, bitPointer - 4)) return 2;	// Month data parity check
  if(!CheckParity(bitPointer - 23, 3, bitPointer - 3)) return 3;	// Day of week data parity check
  if(!CheckParity(bitPointer - 20, 13, bitPointer - 2)) return 4;	// Time data parity check
  return 0;
}

bool CheckParity(int start, int numBits, int parityBitNum)
{
// count the number of "1" bits in the specified chunk of the "A" buffer
// then add the parity bit to the result

  byte parity = 0;

  for(int i = start;i < start + numBits;i++)	// loop for numBits
	{
		// add the bits to "parity"
		parity += bitRead(aBuffer[abs(i/8)], (i % 8) ^ 0x07);
	}

// now add the actual parity bit to parity
  parity += GetBbit(parityBitNum);	//parityBit;	// add the parity bit

  if(parity & 0x01) return true;	// return true if parity is negative e.g. Good
  return false;
}

bool GetBbit(int bitPos)
{
// return the value of the single bit (bitPos) from the "B" buffer
	return bitRead(bBuffer[abs(bitPos/8)], (bitPos % 8) ^ 0x07);
}

byte decToBcd(byte val)			// Convert normal decimal numbers to binary coded decimal
{
  return ( (val/10*16) + (val%10) );
}

byte bcdToDec(byte val)			// Convert binary coded decimal to normal decimal numbers
{
  return ( (val/16*10) + (val%16) );
}
