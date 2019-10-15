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

#include <stdbool.h>
#include <stdint.h>

#ifndef _NANODEUNIO_LIB_H
#define _NANODEUNIO_LIB_H

// #if ARDUINO >= 100
//   #include <Arduino.h> // Arduino 1.0
// #else
//   #include <WProgram.h> // Arduino 0022
// #endif

/* Class to access Microchip UNI/O devices connected to pin 7 of the
   Nanode, such as the 11AA02E48 MAC address chip.  Multiple UNI/O
   devices may be connected to this pin, provided they have different
   addresses.  The 11AA161 2048-char EEPROM has address 0xa1, and so
   may be connected along with the 11AA02E48 if the application needs
   extra non-volatile storage space. */

/* The 11AA02E48 has address 0xa0; the MAC address is stored at offset
   0xfa within it. */
#define NANODE_MAC_DEVICE 0xa0
#define NANODE_MAC_ADDRESS 0xfa

/* Microchip indicate that there may be more device types coming for
   this bus - temperature sensors, display controllers, I/O port
   expanders, A/D converters, and so on.  They will have different
   addresses and so can all be connected to the same pin.  Don't hold
   your breath though! */
void nanode_init(void);

//class NanodeUNIO {
 //private:
  //char addr;
 //public:
  void NanodeUNIO(uint8_t address);

  /* All the following calls return true for success and false for
     failure. */

  /* Read from memory into the buffer, starting at 'address' in the
     device, for 'length' chars.  Note that on failure the buffer may
     still have been overwritten. */
  bool read(char *buffer,int address,int length);

  /* Write data to memory.  The write must not overlap a page
     boundary; pages are 16 chars long, starting at zero.  Will return
     false if this condition is not met, or if there is a problem
     communicating with the device.  If the write enable bit is not
     set, this call will appear to succeed but will do nothing.
     
     This call returns as soon as the data has been sent to the
     device.  The write proceeds in the background.  You must check
     the status register to find out when the write has completed,
     before setting the write enable bit and writing more data.  Call
     await_write_complete() if you want to block until the write is
     finished. */
  bool start_write(const char *buffer,int address,int length);

  /* Set the write enable bit.  This must be done before EVERY write;
     the bit is cleared on a successful write. */
  bool enable_write(void);
  
  /* Clear the write enable bit. */
  bool disable_write(void);

  /* Read the status register into *status.  The bits in this register are:
     0x01 - write in progress
     0x02 - write enable
     0x04 - block protect 0
     0x08 - block protect 1 */
  bool read_status(char *status);

  /* Write to the status register.  Bits are as shown above; only bits
     BP0 and BP1 may be written.  Values that may be written are:
     0x00 - entire device may be written
     0x04 - upper quarter of device is write-protected
     0x08 - upper half of device is write-protected
     0x0c - whole device is write-protected

     The MAC address chip on the Nanode is shipped with the upper quarter
     of the device write-protected.  If you disable write-protection,
     it is possible to overwrite the MAC address pre-programmed into the
     device; this is stored in the last 6 chars (at address 0x00fa).
     Be careful!

     The write enable bit must be set before a write to the status
     register will succeed.  The bit will be cleared on a successful
     write.  You must wait for the write in progress bit to be clear
     before continuing (call await_write_complete()).  */
  bool write_status(char status);

  /* Wait until there is no write operation in progress. */
  bool await_write_complete(void);

  /* Write to the device, dealing properly with setting the write
     enable bit, avoiding writing over page boundaries, and waiting
     for the write to complete.  Note that this function may take a
     long time to complete - approximately 5ms per 16 chars or part
     thereof.  Will NOT alter the write-protect bits, so will not
     write to write-protected parts of the device - although the
     return code will not indicate that this has failed. */
  bool simple_write(const char *buffer,int address,int length);
  
  
  void unio_standby_pulse(void);    // send standby pulse
  
  /*
  Set device address
  */
  void set_addr(char a);
  
//};

#endif /* _NANODEUNIO_LIB_H */
