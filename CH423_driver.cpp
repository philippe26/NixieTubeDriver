#include "CH423_driver.h"
#include "Wire.h"

#define CH423_CMD_SET_SYSTEM_ARGS   (0x48 >> 1) ///< Set system parameter command
#define CH423_CMD_SET_GPO_L         (0x44 >> 1) ///< Set low 8-bit GPO command
#define CH423_CMD_SET_GPO_H         (0x46 >> 1) ///< Set high 8-bit GPO command
#define CH423_CMD_READ_GPIO         (0x4D >> 1) ///< read segment SEG0-SEG7
#define CH423_CMD_SET_GPO_BIDIR     (0x60 >> 1) 

#if 0
typedef union{
  struct{
    uint8_t ioEn:1;  /**< Control three-state output of bi-directional input/output pin GPIO7～IO0, 0 for output disabled, 1 for output enabled*/
    uint8_t decL:1;  /**< Control dynamic scanning disable/enable of output pin GPO7～GPO0*/
    uint8_t decH:1;  /**< Control dynamic scanning disable/enable of output pin GPO15～GPO8*/
    uint8_t intEn:1;  /**< Enable/disable input level change interrupt, disable when it's 0, allow outputting level change interrupt from GPO15 pin when it's 1 and decH is 0 */
    uint8_t odEn:1;  /**< Enable open-drain output of output pin GPO15～GPO0, 0: GPO15～GPO0 is set to push-pull output(can output high/low level), 1: GPO15～GPO0 is set to open-drain output(only output low level or do not output)*/
    uint8_t intens:1;  /**< Control the brightness dynamic display driver*/
    uint8_t sleep:1;  /**< Let CH423 enter low-power sleep state*/
    uint8_t rsv:1;
  };
  uint8_t args;
}uSystemCmdArgsCH423;

typedef union{
  struct{
    uint8_t DISP:1;  /**< Control three-state output of bi-directional input/output pin GPIO7～IO0, 0 for output disabled, 1 for output enabled*/
    uint8_t SCAN:1;  /**< Control dynamic scanning disable/enable of output pin GPO7～GPO0*/
    uint8_t unused1:1; 
    uint8_t intEn:1;  /**< Enable/disable input level change interrupt, disable when it's 0, allow outputting level change interrupt from GPO15 pin when it's 1 and decH is 0 */
    uint8_t unused2:1;  
    uint8_t intens:2;  /**< Control the brightness dynamic display driver: 00=high (internal limiter), 01=low, 10=medium, 11=high (no limiter)*/
    uint8_t sleep:1;  /**< Let CH453 enter low-power sleep state*/    
  };
  uint8_t args;
}uSystemCmdArgsCH453;
#define CH453_CMD_READ_KEY          (0x4F >> 1) ///< read keycode from keyboard scanning process
#define CH453_CMD_SET_DIGIT         (0x60 >> 1)
#endif

bool CH423_driver::beginGPIO(bool outputOnly, bool IRQenabled) 
{  
  device_config=0;
  if(outputOnly) device_config |= 0x01;
  if(IRQenabled) device_config |= 0x08;  
  if (openDrain) device_config |= 0x10;
  return register_write(CH423_CMD_SET_SYSTEM_ARGS, device_config);
}
bool CH423_driver::sleep() 
{  
  return register_write(CH423_CMD_SET_SYSTEM_ARGS, 0x80);  
}
bool CH423_driver::wakeup() 
{  
  //Serial.print("CH423 wakes up with config=0x"); Serial.println(device_config, HEX);
  return register_write(CH423_CMD_SET_SYSTEM_ARGS, device_config);  
}
bool CH423_driver::display(bool enable, uint8_t intensity ) 
{
  //Serial.print("Set Display ");Serial.println((enable)?"ON":"OFF");

  if (enable)  
    device_config |= 0x7; // set Bit DISP & SCAN (DEC_L, DEC_H)
  else
    device_config &= ~7; // clear bit DISP & SCAN

  if (openDrain) 
    device_config |= 0x10;  

  device_config &= ~0x60; // clear bit INTENS
  switch (intensity) {
      case 0: device_config |= 0x20; break; // low level
      case 1: device_config |= 0x40; break; // medium
      default: break; // high    
  }
  
  return register_write(CH423_CMD_SET_SYSTEM_ARGS, device_config);  
}

bool CH423_driver::register_write(uint8_t offset, uint8_t value) 
{
  if (selectBus()) {
    Wire.beginTransmission(offset);
    Wire.write(value);    
    return (Wire.endTransmission()==0)?true:false;
  }
  return false;
}

bool CH423_driver::register_read(uint8_t offset, uint8_t &value) 
{
  if (selectBus()) {

    // ask for 1 bytes to be returned
		if (Wire.requestFrom(offset, 1u) != 1)
		{
				// we are not receiving the number of bytes we need
				return false;  
		};
		// read byte
		value = Wire.read();
    return true;
  }
  return false;
}

uint8_t CH423_driver::getKeyCode()
{
  uint8_t rawcode=0xff;  
  if ((device_config & 0x7)==0) // keyboard enable when all LSB are cleared
    register_read(CH423_CMD_READ_GPIO, rawcode);
  return rawcode;  
}

bool CH423_driver::setOutputLow(uint8_t value) 
{
  if ((device_config & 0x2)) return false; // scan enabled for LOW
  return register_write(CH423_CMD_SET_GPO_L, value);
}
bool CH423_driver::setOutputHigh(uint8_t value)
{
  if ((device_config & 0x4)) return false; // scan enabled for HIGH
  return register_write(CH423_CMD_SET_GPO_H, value);
}
bool CH423_driver::setOutputBidir(uint8_t value)
{
  if ((device_config & 0x6)) return false; // scan enabled for LOW/HIGH
  if ((device_config & 0x1)==0) return false; // output not enabled for bidir port
  return register_write(CH423_CMD_SET_GPO_BIDIR, value);
}

bool CH423_driver::setDigitRaw(uint8_t index, uint8_t value)
{  
  uint8_t nb_digits=16;
  if (device_config&0x8) nb_digits--; // when interrupt is enabled, only 15 output over 16 are allowed
  if (index<nb_digits) {
    return register_write(device_config+index, value);
  }
  return false;  
}

/*        
 *     ___a___
 *    |       |
 *   f|       |b
 *    |___g___|
 *    |       | 
 *   e|       |c 
 *    |_______|
 *        d
 *    _______________________________________
 *   | S7 | S6 | S5 | S4 | S3 | S2 | S1 | S0 |
 *   | dp |  G |  F |  E |  D |  C |  B |  A |
 */

bool CH423_driver::setDigit(uint8_t index, uint8_t value, bool dp)
{
  uint8_t val=0;
  switch (value) {
    case 0: val=0b00111111;break;
    case 1: val=0b00000110;break;
    case 2: val=0b01011011;break;
    case 3: val=0b01001111;break;
    case 4: val=0b01100110;break;
    case 5: val=0b01101101;break;
    case 6: val=0b01111101;break;
    case 7: val=0b00000111;break;
    case 8: val=0b01111111;break;
    case 9: val=0b01101111;break;
    case 10: val=0b01110111;break; //a
    case 11: val=0b01111100;break; //b
    case 12: val=0b00111001;break; //c
    case 13: val=0b01011110;break; //d
    case 14: val=0b01111001;break; //e
    case 15: val=0b01110001;break; //f
  }
  if(dp)
    val|=0x80;
   
   //Serial.print("Dig("); Serial.print(index); Serial.print(")=");Serial.print(value); Serial.print("=> 0x");Serial.println(val, HEX);

  return setDigitRaw(index, val);
}

bool CH423_driver::setDigitEmpty(uint8_t index, bool dp)
{
  uint8_t val=0;
  if(dp)
    val|=0x80;
  return setDigitRaw(index, val);
}
bool CH423_driver::setDigitMinus(uint8_t index)
{
  return setDigitRaw(index, 0b01000000);
}
