#ifndef __CH453_driver__
#define __CH453_driver__

#include "Arduino.h"
#include "TwoWireMultiplex.h"


class CH453_driver {

public:
  CH453_driver() {mux_drv=NULL; mux_channel=0; device_config= 0;}

  void attachTwoWireMultiplex(TwoWireMultiplex *mux, uint8_t channel) {mux_drv=mux; mux_channel=channel;}
  inline bool selectBus() {if (mux_drv) return mux_drv->selectChannel(mux_channel); }
  
  bool begin();
  bool display(bool enable, uint8_t intensity=2 /*0=low, 2=max*/ ) ;
  bool keyboard(bool enable, bool irq_on_dig15); // care that irq reduce the capacity of display from 16 to 15 digits

  // change the display value of digit "index". Bits of value are defined by
  //   b7  b6  b5  b4  b3  b2  b1  b0
  //   DP  G   F   E   D   C   B   A
  bool setDigitRaw(uint8_t index, uint8_t value); 

  bool setDigit(uint8_t index, uint8_t value, bool dp=false); 
  bool setDigitMinus(uint8_t index); 
  bool setDigitEmpty(uint8_t index, bool dp=false);
 
  bool sleep() ;
protected:  
  bool register_write(uint8_t offset, uint8_t value); 

private:
	TwoWireMultiplex *mux_drv;
  uint8_t mux_channel;
  uint8_t device_config;  
};

#endif
