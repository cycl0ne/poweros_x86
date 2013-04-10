#include "types.h"

static inline INT16 inw(INT16 _port)
{
	INT16 result;
	__asm__ volatile ("inw %1, %0" : "=a" (result) : "Nd" (_port));
	return result;
}

/// in in INT8 ///
static inline INT8 inb(INT16 _port)
{
	INT8 result;
	__asm__ volatile ("inb %1, %0" : "=a" (result) : "Nd" (_port));
	return result;
}

/// in in long(32 bit) ///
static inline INT32 inl(INT16 _port)
{
	INT32 result;
	__asm__ volatile ("inl %1, %0" : "=a" (result) : "Nd" (_port));
	return result;
}



/// out in INT32 ///
static inline void outw(INT16 _port, INT16 _data)
{
	__asm__ volatile ("outw %0, %1" : : "a" (_data), "Nd" (_port));
}

/// out in INT8 ///
static inline void outb(INT16 _port, INT8 _data)
{
	__asm__ volatile ("outb %0, %1" : : "a" (_data), "Nd" (_port));
}

/// out in long(32 bit) ///
static inline void outl(INT16 _port, INT32 _data)
{
	__asm__ volatile ("outl %0, %1" : : "a"(_data), "Nd" (_port));
}

/* Ein INT8 an einen IO Port senden und für langsame Ports kurz verzögern */
static inline void outb_wait(INT16 _port, INT8 _data)
{
	__asm__ volatile ("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (_data), "Nd" (_port));
}

static inline void mouse_wait(INT8 a_type) //unsigned char
{
  UINT32 _time_out=100000; //unsigned int
  if(a_type==0)
  {
    while(_time_out--) //Data
    {
      if((inb(0x64) & 1)==1)
      {
        return;
      }
    }
    return;
  }
  else
  {
    while(_time_out--) //Signal
    {
      if((inb(0x64) & 2)==0)
      {
        return;
      }
    }
    return;
  }
}

static inline void mouse_write(UINT8 a_write) //unsigned char
{
  //Wait to be able to send a command
  mouse_wait(1);
  //Tell the mouse we are sending a command
  outb(0x64, 0xD4);
  //Wait for the final part
  mouse_wait(1);
  //Finally write
  outb(0x60, a_write);
}

static inline UINT8 mouse_read()
{
  //Get's response from mouse
  mouse_wait(0);
  return inb(0x60);
}


void arch_ps2m_init()
{
  UINT8 _status;  //unsigned char

  //Enable the auxiliary mouse device
  mouse_wait(1);
  outb(0x64, 0xA8);
 
  //Enable the interrupts
  mouse_wait(1);
  outb(0x64, 0x20);
  mouse_wait(0);
  _status=(inb(0x60) | 2);
  mouse_wait(1);
  outb(0x64, 0x60);
  mouse_wait(1);
  outb(0x60, _status);
 
  //Tell the mouse to use default settings
  mouse_write(0xF6);
  mouse_read();  //Acknowledge
 
  //Enable the mouse
  mouse_write(0xF4);
  mouse_read();  //Acknowledge
}

