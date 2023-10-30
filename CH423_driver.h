#ifndef __CH423_driver__
#define __CH423_driver__

#include "Arduino.h"

#define CH423_LOW_LEVEL_DEBUG 0

/*
 * I2C Address of CH423/453
 * 20h : reserved
 * 21h : reserved
 * 22h : Set low 8-bit general output command [OC_L_DAT:8]
 * 23h : Set high 8-bit general output command [OC_H_DAT:8]
 * 24h : System parameter Settings [SLEEP:1][INTENS:2][OD_EN:1][X_INT:1][DEC_H:1][DEC_L:1][IO_OE:1]
 * 25h : reserved
 * 26h : Read bidirectional I/O command
 * 27h : reserved
 * 30h to 3Fh : Set bidirectional I/O command
*/
class CH423_driver {

public:
  CH423_driver(bool _openDrain=false)
    : device_config(0), openDrain(_openDrain)
    {}

  // Activate the display mode (scan enabled)  
  bool display(bool enable, uint8_t intensity=2 /*0=low, 2=max*/ ) ;
  
  // change the display value of digit "index". Bits of value are defined by
  //   b7  b6  b5  b4  b3  b2  b1  b0
  //   DP  G   F   E   D   C   B   A
  bool setDigitRaw(uint8_t index, uint8_t value); 

  bool setDigit(uint8_t index, uint8_t value, bool dp=false); 
  bool setDigitMinus(uint8_t index); 
  bool setDigitEmpty(uint8_t index, bool dp=false);
  
  // 
  bool beginGPIO(bool outputOnly=false, bool IRQenabled=false);
  // return state of GPIO (8 bits)
  uint8_t getKeyCode(); 

  bool setOutputLow(uint8_t value) ;
  bool setOutputHigh(uint8_t value);
  bool setOutputBidir(uint8_t value);
  
  bool sleep() ;
  bool wakeup() ;
  
protected:  
  bool register_write(uint8_t offset, uint8_t value); 
  bool register_read(uint8_t offset, uint8_t &value); 
  virtual bool selectBus() {return true;}

private:

  uint8_t device_config;  
  bool openDrain;
};

#endif
