

static inline unsigned short inports(unsigned short _port) {
	unsigned short rv;
	asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
	return rv;
}

/*
 * Output multiple sets of shorts
 */
static inline void outportsm(unsigned short port, unsigned char * data, unsigned long size) {
	asm volatile ("rep outsw" : "+S" (data), "+c" (size) : "d" (port));
}

/*
 * Input multiple sets of shorts
 */
static inline void inportsm(unsigned short port, unsigned char * data, unsigned long size) {
	asm volatile ("rep insw" : "+D" (data), "+c" (size) : "d" (port) : "memory");
}

/*
 * inportb
 * Read from an I/O port.
 */
static inline unsigned char inportb(unsigned short _port) {
	unsigned char rv;
	asm volatile ("inb %1, %0" : "=a" (rv) : "dN" (_port));
	return rv;
}

/*
 * outportb
 * Write to an I/O port.
 */
static inline void outportb(unsigned short _port, unsigned char _data) {
	asm volatile ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

static void ata_io_wait(pPataUnit unit) 
{
	inportb(unit->pu_ioBase + ATA_REG_ALTSTATUS);
	inportb(unit->pu_ioBase + ATA_REG_ALTSTATUS);
	inportb(unit->pu_ioBase + ATA_REG_ALTSTATUS);
	inportb(unit->pu_ioBase + ATA_REG_ALTSTATUS);
}

static void ata_soft_reset(pPataUnit unit) 
{
	outportb(unit->pu_Control, 0x04);
	outportb(unit->pu_Control, 0x00);
}

static int ata_wait(pPataUnit dev, int advanced) {
	UINT8 status = 0;

	ata_io_wait(dev);

	while ((status = inportb(dev->pu_ioBase + ATA_REG_STATUS)) & ATA_SR_BSY);

	if (advanced) {
		status = inportb(dev->pu_ioBase + ATA_REG_STATUS);
		if (status   & ATA_SR_ERR)  return 1;
		if (status   & ATA_SR_DF)   return 1;
		if (!(status & ATA_SR_DRQ)) return 1;
	}

	return 0;
}
