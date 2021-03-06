#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
//Board = Arduino Mega 2560 or Mega ADK
#define __AVR_ATmega2560__
#define 
#define _VMDEBUG 1
#define ARDUINO 105
#define ARDUINO_MAIN
#define __AVR__
#define F_CPU 16000000L
#define __cplusplus
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__

#define __builtin_va_list
#define __builtin_va_start
#define __builtin_va_end
#define __DOXYGEN__
#define __attribute__(x)
#define NOINLINE __attribute__((noinline))
#define prog_void
#define PGM_VOID_P int
            
typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

//
void wakeTheDead();
void ledState(byte state);
//
void CFG (byte * cmd,int size);
void ShieldInit();
int  MessageReceived( );
long join_4_bytes(unsigned char Buffer[]);
long join_2_bytes(unsigned char Buffer[]);
long done_byte(unsigned char Buffer[]);
void Shieldubx_checksum(byte ubx_data);

#include "C:\Program Files\arduino\hardware\arduino\variants\mega\pins_arduino.h" 
#include "C:\Program Files\arduino\hardware\arduino\cores\arduino\arduino.h"
#include "C:\Users\James Brown\Documents\Arduino\PixelEtherMega\PixelEtherMega.ino"
#include "C:\Users\James Brown\Documents\Arduino\PixelEtherMega\Shield.ino"
#endif
