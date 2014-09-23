//////////////////////////////////////////////////////////////////////////////////////////////		
// A class to decode the MSF Time Signal from Anthorn, Cumbria, UK							//
// Inspired by Richard Jarkman's original MSFTime library but with a different				//
// approach. I have taken the idea of using a Singleton Pointer to allow the				//
// Arduino "attachInterrupt" function to work within a class. other than that				//
// this library is my own work.																//
//																							//
// You are free to use this library as you see fit as long as this text remains with it!	//
// Copyright 2014 Phil Morris http://www.lydiard.plus.com									//
//////////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <MsfTimeLib.h>

MsfTimeLib *MSFs = NULL; // pointer to singleton

MsfTimeLib::MsfTimeLib() {}

void msfIntChange() // static function for the interrupt handler
{
  if (MSFs) MSFs->msfPulse();
}

// Arduino Interrupt (0 or 1), Output mode false = dec - true = bcd,
// carrierOff = true for HIGH pulse on Carrier Off
// ledPin = pin to flash led on (0 = no led)
// padding in ms if your MSF receiver gives pulse shorter than 100ms
void MsfTimeLib::begin(uint8_t intNum, bool oMode, bool carrierOff, uint8_t ledPin, uint8_t padding)
{
	_outputMode = oMode;
	_carrierOff = LOW;
	if(carrierOff) _carrierOff++;
	_ledPin = ledPin;
	_padding = padding;
	if(_ledPin)	pinMode(_ledPin, OUTPUT);			// set LED pin to OUTPUT if specified
	memset(&_aBuffer[0], 0xFF, sizeof(_aBuffer));	// clear the "A" buffer to "1"s
	memset(&_bBuffer[0], 0, sizeof(_bBuffer));		// clear the "B" buffer	to "0"s
	_intNum = intNum;
	_msfPin = _intNum + 2;
	
	MSFs = this; // singleton pointer
	
	attachInterrupt(_intNum, msfIntChange, CHANGE);
}

void MsfTimeLib::msfPulse()	// interrupt routine which is called every time the MSF receiver output changes state
{
// This routine is called every time the selected Interrupt pin changes state. If it is the start of
// a pulse, the millis count is stored in "pulseStart". If it is the end of a pulse the millis count
// is stored in "pulseEnd". "pulseLength" is the result in millis/100. The data is processed to produce an
// integer 1 - 5 representing 100 - 500 ms pulses (no "4" is decoded). "secondBits" contains the binary data
// for the "A" and "B" buffer contents. "secondBits" data is written as it is detected so, even a double "B"
// pulse is written as an "A" bit first. When the second "B" bit is detected, the "bitBonly" flag is set
// during the current second, and the pointer to the buffers is decremented one position which overwrites
// the previous data.

  _bitBonly = false;				// clear the bitOnly flag
  _secondBits = 0;					// clear the secondBits variable
  _pinState = digitalRead(_msfPin);	// get the state of the interrupt pin

// is this a pulse start?

  if (_pinState == _carrierOff)		// pulse or sub-pulse of has started, carrier going off
	{
		_pulseStart = millis();		// pulseStart = current millis everytime the MSFPIN goes low
		startOfSecond = true;		// set this flag for later use
		if(_ledPin)	digitalWrite(_ledPin,HIGH);	// turn on LED if ledPin > 0
		return;						// until there's a another interrupt change
	}

// is this a pulse end?

  if(_pinState != _carrierOff)								// pulse end, carrier going on
	{
		_pulseEnd = millis();								// set the pulse end ms
		startOfSecond = false;								// clear the start of second flag
		_pulseLength = abs(((_pulseEnd - _pulseStart)
		+ _padding) / 100);									// get the pulse length in ms/100
		_pulseStart = millis();								// set the pulseStart to current millis
		if (!_pulseLength) return;							// if the pulse is too short ("0"), return

// if the sequence was 100ms off + 100ms on + 100ms off, this is a 'B' only bit
// so, if this start pulse is less than 300ms after the last start pulse it must be
// a double 100ms pulse second

		if(_pulseStart - _lastPulseStart < 300) _bitBonly = true;	// this is a 'B' bit
	    _lastPulseStart = _pulseStart;								// keep the last pulse start ms count
		if(_ledPin) digitalWrite(_ledPin,LOW);						// turn off the LED if ledPin > 0
	}

 switch(_pulseLength)	// start processing the valid pulse
	{
		case 5:	// check for start pulse i.e. 500ms
				
			_bitPointer = 0;								// clear the buffer bit pointer
			memset(&_aBuffer[0], 0xFF, sizeof(_aBuffer));	// clear the "A" buffer to all "1"s
			memset(&_bBuffer[0], 0, sizeof(_bBuffer));		// clear the "B" buffer
			break;
			
		case 4:	// in the unlikely event we get a "4" quit
			return;
			
		case 3:	// check for 300ms pulse, this is an 'A' + 'B' bit case
	
			_secondBits = 0b11;	// both "A" and "B" bits are set
			break;
			
		case 2:	// check for 200ms pulse, this is an 'A' bit case
			
			_secondBits = 0b01;	// only the "A" bit is set
			break;
			
		case 1:	// check for 100ms pulse
			
			_secondBits = 0b00;
			if(_bitBonly) _secondBits = 0b10;		// only the "B" bit is set
			break;			
	}

// store the data in the arrays
  if(_pulseLength < 5)  // only pass this point if it's not a "Start" pulse e.g. < 500ms
	{
		_bitPointer++;							// increment the bit pointer which allways starts at 1
		if(_bitBonly) _bitPointer--;			// if this is a "B" bit, decrement the bit pointer as
												// we have already written 0 bits to both the "A" and "B"
												// buffers
	// store the bit in the "A" buffer
		bitWrite(_aBuffer[abs(_bitPointer/8)], (_bitPointer % 8) ^ 0x07, bitRead(_secondBits,0));
	// store the bit in the "B" buffer
		bitWrite(_bBuffer[abs(_bitPointer/8)], (_bitPointer % 8) ^ 0x07, bitRead(_secondBits,1));

// we detect the last second of the minute by looking for the binary sequence "01111110" in the "A" buffer
// bits 52 thru 59. If we see this sequence it's time to stop decoding and start working on the data received

    if(getChunk(_aBuffer, _bitPointer - 7, 8) == 0b01111110)		// look for "01111110" sequence in "A" buffer
		{												// which indicates that the last second data has been received
														// and round it up for comfort as we've been using it -1
			TimeAvailable = false;						// clear the flag
			uint8_t _parityResult = getParity();		// check the parity of the data, Good = 0

// if the parity is OK, get the data from the "A" buffer into the variables. The MSF data is in BCD so we convert
// it to decimal here for the Time library. You would leave it as BCD for a RTC such as the DS1307

			if(!_parityResult && _bitPointer >= MIN_STREAM_LEN)	// make sure there are enough bits to work on e.g. 58 or more seconds worth
				{
					if(_outputMode)	// outputs DECIMAL if _outputMode = true
					{
						Year = bcdToDec(getChunk(_aBuffer, _bitPointer - 42,8));		// year	(offset, number of bits to read)
						Month = bcdToDec(getChunk(_aBuffer, _bitPointer - 34,5));		// month
						Date = bcdToDec(getChunk(_aBuffer, _bitPointer - 29,6));		// date
						Day = bcdToDec(getChunk(_aBuffer, _bitPointer - 23,3));		// weekday
						Hour = bcdToDec(getChunk(_aBuffer, _bitPointer - 20,6));		// hour
						Minute = bcdToDec(getChunk(_aBuffer, _bitPointer - 14,7));	// minute
										}
					else	// outputs BCD if _outputMode = false
					{
						Year = getChunk(_aBuffer, _bitPointer - 42,8);		// year	(offset, number of bits to read)
						Month = getChunk(_aBuffer, _bitPointer - 34,5);		// month
						Date = getChunk(_aBuffer, _bitPointer - 29,6);		// date
						Day = getChunk(_aBuffer, _bitPointer - 23,3);			// weekday
						Hour = getChunk(_aBuffer, _bitPointer - 20,6);		// hour
						Minute = getChunk(_aBuffer, _bitPointer - 14,7);		// minute
					}
						RxSecs = _bitPointer + 1;					// number of seconds received
						Bst = getBit(_bBuffer, _bitPointer - 1);				//BST = 1, GMT = 0
						BstSoon = getBit(_bBuffer, _bitPointer - 6);			// BST imminent = 1
						getChunk(_bBuffer, 1,8);									// get DUT1 Positive bit count
						DutPos = _bitCounter * 100;
						getChunk(_bBuffer, 9,8);									// get DUT1 Negative bit count
						DutNeg = _bitCounter * 100;
						LastParityResult = _parityResult;			// store the last parity result
						TimeAvailable = true;						// set the flag to say time received OK
				}
		}
  }
}// End of "msfPulse" Interrupt Routine

/* Everything beyond this point is for decoding and parity checking */

uint8_t MsfTimeLib::getChunk(uint8_t * _buffer, uint8_t _start, uint8_t _numBits)
{
// return the byte/time 'chunk' containing the number of bits read at the starting
// bit position in the Raw Buffer up to 8 bits. returns a byte

  uint8_t _chunk = 0;
  uint8_t _bitVal = 0;
  uint8_t _counter = _numBits - 1;
  _bitCounter = 0;				// a count of the number of "1" bits
  
  for(uint16_t i = _start;i < _start + _numBits;i++)		// loop for numBits
	{
		// return the value of the single bit (bitPos) from the "A" buffer
		_bitVal = bitRead(_buffer[abs(i/8)], (i % 8) ^ 0x07);	// get the bit from the buffer
		bitWrite(_chunk,_counter,_bitVal);					// write the bit to "chunk"
		if(_bitVal) _bitCounter++;							// if it's a "1" increment the bitCounter
		_counter--;											// decrement the counter
	}
  return _chunk;
}

uint8_t MsfTimeLib::getParity()
{
// calculate the parity bits and return true if all's well
// all data and parity bits are relative to the last second count copntained in "bitPointer"
// this allows for leep seconds which are added or removed at second 16 i.e. before
// the actual date & time data. There can be 59, 60 or 61 seconds in a leep minute

  if(!checkParity(_bitPointer - 42, 8, _bitPointer - 5)) return 1;	// Year data parity check
  if(!checkParity(_bitPointer - 34, 11, _bitPointer - 4)) return 2;	// Month data parity check
  if(!checkParity(_bitPointer - 23, 3, _bitPointer - 3)) return 3;	// Day of week data parity check
  if(!checkParity(_bitPointer - 20, 13, _bitPointer - 2)) return 4;	// Time data parity check
  return 0;
}

bool MsfTimeLib::checkParity(uint8_t _start, uint8_t _numBits, uint8_t _parityBitNum)
{
// count the number of "1" bits in the specified chunk of the "A" buffer
// then add the parity bit to the result

  uint8_t _parity = 0;

  for(uint16_t i = _start;i < _start + _numBits;i++)	// loop for numBits
	{
		// add the bits to "parity"
		_parity += getBit(_aBuffer, i);
		//_parity += bitRead(_aBuffer[abs(i/8)], (i % 8) ^ 0x07);
	}

// now add the actual parity bit to parity
  _parity += getBit(_bBuffer, _parityBitNum);			// add the parity bit
  if(_parity & 0x01) return true;	// return true if parity is negative e.g. Good
  return false;
}

bool MsfTimeLib::getBit(uint8_t * _buffer, uint8_t _bitPos)
{
// return the value of the single bit (bitPos) from the buffer
	return bitRead(_buffer[abs(_bitPos/8)], (_bitPos % 8) ^ 0x07);
}

uint8_t MsfTimeLib::bcdToDec(uint8_t _val)			// Convert binary coded decimal to normal decimal numbers
{
  return ( (_val/16*10) + (_val%16) );
}
