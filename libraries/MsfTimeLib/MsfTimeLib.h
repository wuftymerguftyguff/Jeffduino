///////////////////////////////////////////////////////////////////////////////////////////////		
// A class to decode the MSF Time Signal from Anthorn, Cumbria, UK							 //
// Inspired by Richard Jarkman's original MSFTime library but with a different				 //
// approach. I have taken the idea of using a Singleton Pointer to allow the				 //
// Arduino "attachInterrupt" function to work within a class. other than that				 //
// this is all my own work.																     //
//																							 //
// You are free to use this library as you see fit as long as this text remains with it!	 //
//                                                                                           //
// The variables available to the user are:                                                  //
//                                                                                           //
// bool startOfSecond;		// set at start of second pulse,                                 //
//                          // reset at end of the second pulse                              //
// bool TimeAvailable;		// set when time and date data has been decoded successfully     //
//                          // used with "startOfsecond" to identify the exact start of the  //
//                          // 500 mS START pulse so, if both "startOfSecond" and "          //
//                          // "TimeAvailable" are "true", set the RTC or Time library "NOW" //
// uint8_t Year;			// year (BCD or DECIMAL" format)                                 //
// uint8_t Month;			// month (BCD or DECIMAL" format)                                //
// uint8_t Date;			// date (BCD or DECIMAL" format)                                 //
// uint8_t Day;			    // weekday (1 - 7)                                               //
// uint8_t Hour;			// hour (BCD or DECIMAL" format)                                 //
// uint8_t Minute;			// minute (BCD or DECIMAL" format)                               //
// uint8_t RxSecs;			// number of seconds received for decoding                       //
// bool Bst;				// 1 = BST, 0 = GMT                                              //
// bool BstSoon;			// 1 = BST imminent                                              //
// uint16_t DutPos;		    // DUT1 Positive value in ms                                     //
// uint16_t DutNeg;		    // DUT1 Negative value in ms                                     //
//                                                                                           //
// Constants available for configuration are:                                                //
//                                                                                           //
// MSF_DEC_OUT   			// forces DECIMAL output                                         //
// MSF_BCD_OUT				// forces BCD output                                             //
// MSF_PULSE_LOW			// MSF "off" pulse is LOW                                        //
// MSF_PULSE_HIGH			// MSF "off" pulse is HIGH                                       //
// MSF_INT_0 0				// Arduino UNO Interrupt 0 on Pin 2                              //
// MSF_INT_1 1				// Arduino UNO Interrupt on Pin3                                 //
//                                                                                           //
// To start an instance of MsfTimeLib:                                                       //
//                                                                                           //
// <instance>.begin(Interrupt, Output Mode, MSF Pulse, LED Pin, Padding);                    //
//                                                                                           //
// LED Pin is optional but, if set to a Pin number, and LED on that Pin will flash in sync   //
// with the received MSF pulses                                                              //
//                                                                                           //
// Padding: In the unlikely event that your MSF receiver outputs pulses that are SHORTER     //
// than the specified pulses, you can add padding in mS to make up the shortfall             //
//                                                                                           //
// example: msf.begin(MSF_INT_1, MSF_PULSE_HIGH, MSF_DEC_OUT, 13, 0);                        //
//                                                                                           //
// ////////////////////////////////////////////////////////////////////////////////////////////


#ifndef MsfTimeLib_h
#define MsfTimeLib_h

#include <Arduino.h>

// configuration constants:
#define MSF_DEC_OUT 1						// forces DECIMAL output
#define MSF_BCD_OUT 0						// forces BCD output
#define MSF_PULSE_LOW 0						// MSF "off" pulse is LOW
#define MSF_PULSE_HIGH 1					// MSF "off" pulse is HIGH
#define MSF_INT_0 0							// Arduino UNO Interrupt 0 on Pin 2 
#define MSF_INT_1 1							// Arduino UNO Interrupt on Pin3

#define MIN_STREAM_LEN 58					// minimum number of seconds to receive for
											// a valid decode

class MsfTimeLib
{
	private:
		uint8_t _aBuffer[8];				// buffer for 'A' bits
		uint8_t _bBuffer[8];				// buffer for 'B' bits
		uint8_t _arduinoInt;				// Arduino interrupt #, 0 or 1 on Pin 3 or 4
		volatile uint32_t _pulseStart;		// milliseconds when start of pulse occurred
		volatile uint32_t _pulseEnd;		// milliseconds when pulse ended
		volatile uint32_t _lastPulseStart;	// the previous pulse start value
		volatile uint8_t _pulseLength;		// length of pulse/100 as an integer
		volatile bool _bitBonly;			// set if a 'B' only pulse detected
		volatile uint8_t _secondBits;		// bits decoded from seconds
		volatile uint8_t _bitPointer;		// pointer for bits within buffer bytes
		volatile uint8_t _bitCounter;		// used to count number of "1"s in chunk routines
		volatile bool _outputMode;			// True = BCD, False = Dec
		volatile uint8_t _ledPin;			// pin to flash on pulses, 0 = off
		volatile uint8_t _msfPin;			// pin for MSF Rx signal
		volatile bool _carrierOff;			// True = Rx output is HIGH when carrier is off
		volatile bool _pinState;			// used for interrupt pin sensing
		volatile uint8_t _intNum;			// Arduino interrupt number 0 or 1 on Pin 2 or 3
		volatile uint8_t _padding;			// time to add to pulse length measurement in ms
		uint8_t getChunk(uint8_t * _buffer, uint8_t _start, uint8_t _numBits);
		uint8_t getParity();				// returns the parity status
		bool checkParity(uint8_t start, uint8_t numBits, uint8_t parityBitNum);
		bool getBit(uint8_t * _buffer, uint8_t _bitPos); // returns a single bit from a buffer

	
	public:
		MsfTimeLib();
		void begin(uint8_t intNum, bool oMode, bool carrierOff, uint8_t ledPin, uint8_t padding);
		uint8_t bcdToDec(uint8_t val);
		void msfPulse();
		volatile bool startOfSecond;		// set at start of second pulse, reset at end of second pulse
		volatile bool TimeAvailable;		// set when time and date data has been decoded successfully
		volatile uint8_t Year;			// year
		volatile uint8_t Month;			// month
		volatile uint8_t Date;			// date
		volatile uint8_t Day;			// weekday
		volatile uint8_t Hour;			// hour
		volatile uint8_t Minute;			// minute
		volatile uint8_t RxSecs;			// number of seconds received for decoding
		volatile bool Bst;				// 1 = BST, 0 = GMT
		volatile bool BstSoon;			// 1 = BST imminent
		volatile uint16_t DutPos;		// DUT1 Positive value in ms
		volatile uint16_t DutNeg;		// DUT1 Negative value in ms
		volatile uint8_t LastParityResult;	// the last parity result
};

#endif
