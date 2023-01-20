#include "CH453_driver.h"
#include "Wire.h"

#define CH423_CMD_SET_SYSTEM_ARGS   (0x48 >> 1) ///< Set system parameter command
#define CH423_CMD_SET_GPO_L         (0x44 >> 1) ///< Set low 8-bit GPO command
#define CH423_CMD_SET_GPO_H         (0x46 >> 1) ///< Set high 8-bit GPO command
#define CH423_CMD_SET_GPIO           0x30       ///< Set bi-directional input/output pin command
#define CH423_CMD_READ_GPIO         (0x4D >> 1) ///< Set bi-directional input/output pin command

#define CH453_CMD_SET_DIGIT         (0x60 >> 1)

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
    uint8_t KEYB:1;  /**< Control dynamic scanning disable/enable of output pin GPO7～GPO0*/
    uint8_t unused1:1; 
    uint8_t intEn:1;  /**< Enable/disable input level change interrupt, disable when it's 0, allow outputting level change interrupt from GPO15 pin when it's 1 and decH is 0 */
    uint8_t unused2:1;  
    uint8_t intens:2;  /**< Control the brightness dynamic display driver: 00=high (internal limiter), 01=low, 10=medium, 11=high (no limiter)*/
    uint8_t sleep:1;  /**< Let CH453 enter low-power sleep state*/    
  };
  uint8_t args;
}uSystemCmdArgsCH453;


bool CH453_driver::begin() {
  device_config = 0;
  return register_write(CH423_CMD_SET_SYSTEM_ARGS, device_config);
}
bool CH453_driver::sleep() 
{
  uSystemCmdArgsCH453 _args;
  _args.sleep = 1;// fugitive high level to allow any next command to wake up the device, do not change the device_config 
  register_write(CH423_CMD_SET_SYSTEM_ARGS, _args.args);  
}
bool CH453_driver::display(bool enable, uint8_t intensity ) 
{
  uSystemCmdArgsCH453 &_args = (uSystemCmdArgsCH453 &)device_config;  
  _args.DISP = (enable)?1:0;
  switch (intensity) {
    case 0: _args.intens = 1; break; // low level
    case 1: _args.intens = 2; break; // medium
    default: _args.intens = 0; break; // high    
  }
  register_write(CH423_CMD_SET_SYSTEM_ARGS, _args.args);  
}
bool CH453_driver::keyboard(bool enable, bool irq_on_dig15) 
{
  uSystemCmdArgsCH453 &_args = (uSystemCmdArgsCH453 &)device_config;  
  _args.KEYB = (enable)?1:0;
  _args.intEn = (irq_on_dig15)?1:0;
  register_write(CH423_CMD_SET_SYSTEM_ARGS, _args.args);  
}
bool CH453_driver::register_write(uint8_t offset, uint8_t value) 
{
  if (selectBus()) {
    Wire.beginTransmission(offset);
    Wire.write(value);
    return Wire.endTransmission();
  }
  return false;
}
bool CH453_driver::setDigitRaw(uint8_t index, uint8_t value)
{
  uSystemCmdArgsCH453 &_args = (uSystemCmdArgsCH453 &)device_config;
  uint8_t nb_digits=16;
  if (_args.intEn) nb_digits--;
  if (index<nb_digits) {
    return register_write(CH453_CMD_SET_DIGIT+index, value);
  }
  return false;  
}
bool CH453_driver::setDigit(uint8_t index, uint8_t value, bool dp)
{
  uint8_t val=0;
  switch (value) {
    case 0: val=0b00111111;break;
    case 1: val=0b00000110;break;
    case 2: val=0b01011011;break;
    case 3: val=0b01001111;break;
    case 4: val=0b01100100;break;
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
  return setDigitRaw(index, val);
}

bool CH453_driver::setDigitEmpty(uint8_t index, bool dp)
{
  uint8_t val=0;
  if(dp)
    val|=0x80;
  return setDigitRaw(index, val);
}
bool CH453_driver::setDigitMinus(uint8_t index)
{
  return setDigitRaw(index, 0b01000000);
}