#ifndef asm_h
#define asm_h

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

__attribute__((no_instrument_function)) static inline void pio_write_8(volatile UINT16 *port, UINT8 val)
{
	asm volatile (
		"outb %b[val], %w[port]\n"
		:: [val] "a" (val),
		   [port] "d" (port)
	);
}

__attribute__((no_instrument_function)) static inline void pio_write_16(volatile UINT16 *port, UINT16 val)
{
	asm volatile (
		"outw %w[val], %w[port]\n"
		:: [val] "a" (val),
		   [port] "d" (port)
	);
}

__attribute__((no_instrument_function)) static inline void pio_write_32(volatile UINT16 *port, UINT32 val)
{
	asm volatile (
		"outl %[val], %w[port]\n"
		:: [val] "a" (val),
		   [port] "d" (port)
	);
}

__attribute__((no_instrument_function)) static inline UINT8 pio_read_8(volatile UINT16 *port)
{
	UINT8 val;
	
	asm volatile (
		"inb %w[port], %b[val]\n"
		: [val] "=a" (val)
		: [port] "d" (port)
	);
	
	return val;
}

__attribute__((no_instrument_function)) static inline UINT16 pio_read_16(volatile UINT16 *port)
{
	UINT16 val;
	
	asm volatile (
		"inw %w[port], %w[val]\n"
		: [val] "=a" (val)
		: [port] "d" (port)
	);
	
	return val;
}

__attribute__((no_instrument_function)) static inline UINT32 pio_read_32(volatile UINT16 *port)
{
	UINT32 val;
	
	asm volatile (
		"inl %w[port], %[val]\n"
		: [val] "=a" (val)
		: [port] "d" (port)
	);
	
	return val;
}

#endif
