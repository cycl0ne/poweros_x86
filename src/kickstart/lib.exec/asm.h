#ifndef asm_h
#define asm_h

__attribute__((no_instrument_function)) static inline void pio_write_8(volatile UINT8 *port, UINT8 val)
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

__attribute__((no_instrument_function)) static inline UINT8 pio_read_8(volatile UINT8 *port)
{
	UINT8 val;
	
	asm volatile (
		"inb %w[port], %b[val]\n"
		: [val] "=a" (val)
		: [port] "d" (port)
	);
	
	return val;
}

__attribute__((no_instrument_function)) static inline UINT16 pio_read_16(volatile UINT16 port)
{
	UINT16 val;
	
	asm volatile (
		"inw %w[port], %w[val]\n"
		: [val] "=a" (val)
		: [port] "d" (port)
	);
	
	return val;
}

__attribute__((no_instrument_function)) static inline UINT32 pio_read_32(volatile UINT16 port)
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