/* Copyright (C) 2011 by Stephen Early <steve@greenend.org.uk>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#include "config.h"
#include "NanodeUNIO.h"
#include <stdint.h>
#include <Arduino.h>


#define UNIO_STARTHEADER 0x55
#define UNIO_READ        0x03
#define UNIO_CRRD        0x06
#define UNIO_WRITE       0x6c
#define UNIO_WREN        0x96
#define UNIO_WRDI        0x91
#define UNIO_RDSR        0x05
#define UNIO_WRSR        0x6e
#define UNIO_ERAL        0x6d
#define UNIO_SETAL       0x67

// The following are defined in the datasheet as _minimum_ times, in
// microseconds.  There is no maximum.
#define UNIO_TSTBY 600  // 600
#define UNIO_TSS   5  // 10
#define UNIO_THDR  5  // 5

#define UNIO_QUARTER_BIT 20

// Add this to all the times defined above, to be on the safe side!
#define UNIO_FUDGE_FACTOR 0

#define UNIO_OUTPUT() pinMode(ID_PIN,  OUTPUT)
#define UNIO_INPUT()  pinMode(ID_PIN,  INPUT)

char addr = 0;

void set_addr(char a) {
  addr = a;
}

void set_bus(bool state) {
  digitalWrite(ID_PIN, state);
}

bool read_bus(void) {
  return digitalRead(ID_PIN);
}

/* If multiple commands are to be issued to a device without a standby
   pulse in between, the bus must be held high for at least UNIO_TSS
   between the end of one command and the start of the next. */
void unio_inter_command_gap(void) {
  set_bus(1);
  delayMicroseconds(UNIO_TSS+UNIO_FUDGE_FACTOR);
}

/* Send a standby pulse on the bus.  After power-on or brown-out
   reset, the device requires a low-to-high transition on the bus at
   the start of the standby pulse.  To be conservative, we take the
   bus low for UNIO_TSS, then high for UNIO_TSTBY. */
void unio_standby_pulse(void) {
  set_bus(0);
  UNIO_OUTPUT();
  delayMicroseconds(UNIO_TSS+UNIO_FUDGE_FACTOR);
  set_bus(1);
  //delayMicroseconds(UNIO_TSTBY+UNIO_FUDGE_FACTOR);     // replaced by tmr0 overflow

}

void wait_int(void){
  delayMicroseconds(UNIO_QUARTER_BIT);
}

/* While bit-banging, all delays are expressed in terms of quarter
   bits.  We use the same code path for reading and writing.  During a
   write, we perform dummy reads at 1/4 and 3/4 of the way through
   each bit time.  During a read we perform a dummy write at the start
   and 1/2 way through each bit time. */

static volatile bool rwbit(bool w) {
  bool a,b;
  set_bus(!w);
  wait_int();
  a=read_bus();
  wait_int();
  set_bus(w);
  wait_int();
  b=read_bus();
  wait_int();
  return b&&!a;
}

static bool read_bit(void) {
  bool b;
  UNIO_INPUT();
  b=rwbit(1);
  UNIO_OUTPUT();
  return b;
}

static bool send_char(char b, bool mak) {
  for (int i=0; i<8; i++) {
    rwbit(b&0x80);
    b<<=1;
  }
  rwbit(mak);
  return read_bit();
}

static bool read_char(char *b, bool mak) {
  char data=0;
  UNIO_INPUT();
  for (int i=0; i<8; i++) {
    data = (data << 1) | rwbit(1);
  }
  UNIO_OUTPUT();
  *b=data;
  rwbit(mak);
  return read_bit();
}

/* Send data on the bus. If end is true, send NoMAK after the last
   char; otherwise send MAK. */
static bool unio_send(const char *data,int length,bool end) {
  for (int i=0; i<length; i++) {
    /* Rules for sending MAK: if it's the last char and end is true,
       send NoMAK.  Otherwise send MAK. */
    if (!send_char(data[i],!(((i+1)==length) && end))) return false;
  }
  return true;
}

/* Read data from the bus.  After reading 'length' chars, send NoMAK to
   terminate the command. */
static bool unio_read(char *data,int length)  {
  for (int i=0; i<length; i++) {
    if (!read_char(data+i,!((i+1)==length))) return false;
  }
  return true;
}

/* Send a start header on the bus.  Hold the bus low for UNIO_THDR,
   then transmit UNIO_STARTHEADER.  There is a time slot for SAK after
   this transmission, but we must ignore the value because no slave
   devices will drive the bus; if there is more than one device
   connected then that could cause bus contention. */
void unio_start_header(void) {
  UNIO_OUTPUT();
  set_bus(0);
  delayMicroseconds(UNIO_THDR+UNIO_FUDGE_FACTOR);
  send_char(UNIO_STARTHEADER,true);
}

void NanodeUNIO(uint8_t address) {
  addr=address;
}

#define fail() return false; 

bool read(char *buffer,int address,int length) {
  char cmd[4];
  cmd[0]=addr;
  cmd[1]=UNIO_READ;
  cmd[2]=(char)(address>>8);
  cmd[3]=(char)(address&0xff);
  //unio_standby_pulse();
  //cli();
  unio_start_header();
  if (!unio_send(cmd,4,false)) fail();
  if (!unio_read(buffer,length)) fail();
  //sei();
  return true;
}

bool start_write(const char *buffer,int address,int length) {
  char cmd[4];
  if (((address&0x0f)+length)>16) return false; // would cross page boundary
  cmd[0]=addr;
  cmd[1]=UNIO_WRITE;
  cmd[2]=(char)(address>>8);
  cmd[3]=(char)(address&0xff);
  //unio_standby_pulse();
  //cli();
  unio_start_header();
  if (!unio_send(cmd,4,false)) fail();
  if (!unio_send(buffer,length,true)) fail();
  //sei();
  return true;
}

bool enable_write(void) {
  char cmd[2];
  cmd[0]=addr;
  cmd[1]=UNIO_WREN;
  //unio_standby_pulse();
  //cli();
  unio_start_header();
  if (!unio_send(cmd,2,true)) fail();
  //sei();
  return true;
}

bool disable_write(void) {
  char cmd[2];
  cmd[0]=addr;
  cmd[1]=UNIO_WRDI;
  //unio_standby_pulse();
  //cli();
  unio_start_header();
  if (!unio_send(cmd,2,true)) fail();
  //sei();
  return true;
}

bool read_status(char *status) {
  char cmd[2];
  cmd[0]=addr;
  cmd[1]=UNIO_RDSR;
  //unio_standby_pulse();
  //cli();
  unio_start_header();
  if (!unio_send(cmd,2,false)) fail();
  if (!unio_read(status,1)) fail();
  //sei();
  return true;
}

bool write_status(char status) {
  char cmd[3];
  cmd[0]=addr;
  cmd[1]=UNIO_WRSR;
  cmd[2]=status;
  //unio_standby_pulse();
  //cli();
  unio_start_header();
  if (!unio_send(cmd,3,true)) fail();
  //sei();
  return true;
}

bool await_write_complete(void) {
  char cmd[2];
  char status;
  cmd[0]=addr;
  cmd[1]=UNIO_RDSR;
  //unio_standby_pulse();
  /* Here we issue RDSR commands back-to-back until the WIP bit in the
     status register is cleared.  Note that this isn't absolutely the
     most efficient way to monitor this bit; after sending the command
     we could read as many chars as we like as long as we send MAK
     after each char.  The unio_read() function isn't set up to do
     this, though, and it's not really performance-critical compared
     to the eeprom write time! We re-enable interrupts briefly between
     each command so that any background tasks like updating millis()
     continue to happen.*/
  do {
    unio_inter_command_gap();
    //cli();
    unio_start_header();
    if (!unio_send(cmd,2,false)) fail();
    if (!unio_read(&status,1)) fail();
    //sei();
  } while (status&0x01);
  return true;
}

bool simple_write(const char *buffer,int address,int length) {
  int wlen;
  while (length>0) {
    wlen=length;
    if (((address&0x0f)+wlen)>16) {
      /* Write would cross a page boundary.  Truncate the write to the
   page boundary. */
      wlen=16-(address&0x0f);
    }
    if (!enable_write()) return false;
    if (!start_write(buffer,address,wlen)) return false;
    if (!await_write_complete()) return false;
    buffer+=wlen;
    address+=wlen;
    length-=wlen;
  }
  return true;
}
