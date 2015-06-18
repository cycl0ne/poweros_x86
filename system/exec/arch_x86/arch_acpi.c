/*
*
*
*/

#include "types.h"
#include "arch_acpi.h"
#include "arch_madt.h"

#include "exec_interface.h"

#define BIOS_EBDA_PTR  0x40eU

uint32_t ebda = 0;

#define RSDP_SIGNATURE      "RSD PTR "
#define RSDP_REVISION_OFFS  15

#define CMP_SIGNATURE(left, right) \
	(((left)[0] == (right)[0]) && \
	((left)[1] == (right)[1]) && \
	((left)[2] == (right)[2]) && \
	((left)[3] == (right)[3]))

struct acpi_rsdp *acpi_rsdp = NULL;
struct acpi_rsdt *acpi_rsdt = NULL;
struct acpi_xsdt *acpi_xsdt = NULL;
struct acpi_madt *acpi_madt = NULL;

typedef unsigned long size_t;

extern pSysBase g_SysBase;
#define SysBase g_SysBase

struct acpi_signature_map signature_map[] = {
	{
		(uint8_t *) "APIC",
		(void *) &acpi_madt,
		"Multiple APIC Description Table"
	}
};

static void bios_init(void)
{
	ebda = *((uint16_t *) BIOS_EBDA_PTR) * 0x10U;
}

static int acpi_sdt_check(uint8_t *sdt)
{
	struct acpi_sdt_header *hdr = (struct acpi_sdt_header *) sdt;
	uint8_t sum = 0;
	unsigned int i;
	for (i = 0; i < hdr->length; i++) sum = (uint8_t) (sum + sdt[i]);
	return !sum;
}

static int rsdp_check(uint8_t *_rsdp) 
{
	struct acpi_rsdp *rsdp = (struct acpi_rsdp *) _rsdp;
	uint8_t sum = 0;
	uint32_t i;
	
	for (i = 0; i < 20; i++) sum = (uint8_t) (sum + _rsdp[i]);
	if (sum) return 0; /* bad checksum */
	if (rsdp->revision == 0) return 1; /* ACPI 1.0 */
	for (; i < rsdp->length; i++) sum = (uint8_t) (sum + _rsdp[i]);
	return !sum;
}

static void configure_via_rsdt(void)
{
	size_t i;
	size_t j;
	size_t cnt = (acpi_rsdt->header.length - sizeof(struct acpi_sdt_header)) / sizeof(uint32_t);
	
	for (i = 0; i < cnt; i++) 
	{
		for (j = 0; j < sizeof(signature_map) / sizeof(struct acpi_signature_map); j++) 
		{
			struct acpi_sdt_header *hdr = (struct acpi_sdt_header *) acpi_rsdt->entry[i];
			struct acpi_sdt_header *vhdr = hdr;
			if (CMP_SIGNATURE(vhdr->signature, signature_map[j].signature)) 
			{
				if (!acpi_sdt_check((uint8_t *) vhdr)) break;
				*signature_map[j].sdt_ptr = vhdr;
				KPrintF("%p: ACPI (rsdt) %s\n", *signature_map[j].sdt_ptr, signature_map[j].description);
			}
		}
	}
}

static void configure_via_xsdt(void)
{
	size_t i;
	size_t j;
	size_t cnt = (acpi_xsdt->header.length - sizeof(struct acpi_sdt_header)) / sizeof(uint64_t);
	
	for (i = 0; i < cnt; i++)
	{
		for (j = 0; j < sizeof(signature_map) / sizeof(struct acpi_signature_map); j++) 
		{
			struct acpi_sdt_header *hdr = (struct acpi_sdt_header *) ((uintptr_t) acpi_xsdt->entry[i]);
			struct acpi_sdt_header *vhdr = hdr;
			if (CMP_SIGNATURE(vhdr->signature, signature_map[j].signature)) 
			{
				if (!acpi_sdt_check((uint8_t *) vhdr)) break;
				*signature_map[j].sdt_ptr = vhdr;
				KPrintF("%p: ACPI (xsdt) %s\n", *signature_map[j].sdt_ptr, signature_map[j].description);
			}
		}
	}
}

void acpi_init(void)
{
	uint8_t *addr[2] = { NULL, (uint8_t *) 0xe0000 };
	unsigned int i;
	unsigned int j;
	unsigned int length[2] = { 1024, 128 * 1024 };
	uint64_t *sig = (uint64_t *) RSDP_SIGNATURE;
	
	bios_init();
	/*
	 * Find Root System Description Pointer
	 * 1. search first 1K of EBDA
	 * 2. search 128K starting at 0xe0000
	 */
	
	addr[0] = (uint8_t *) ebda;
	for (i = (ebda ? 0 : 1); i < 2; i++) 
	{
		for (j = 0; j < length[i]; j += 16) 
		{
			if ((*((uint64_t *) &addr[i][j]) == *sig)
			    && (rsdp_check(&addr[i][j]))) 
			{
				acpi_rsdp = (struct acpi_rsdp *) &addr[i][j];
				goto rsdp_found;
			}
		}
	}
	
	return;
	
rsdp_found:
	KPrintF("%p: ACPI Root System Description Pointer\n", acpi_rsdp);
	
	uintptr_t acpi_rsdt_p = (uintptr_t) acpi_rsdp->rsdt_address;
	uintptr_t acpi_xsdt_p = 0;

	if (acpi_rsdp->revision) acpi_xsdt_p = (uintptr_t) acpi_rsdp->xsdt_address;
	if (acpi_rsdt_p) acpi_rsdt = (struct acpi_rsdt *) acpi_rsdt_p;
	if (acpi_xsdt_p) acpi_xsdt = (struct acpi_xsdt *) acpi_xsdt_p;
	if ((acpi_rsdt) && (!acpi_sdt_check((uint8_t *) acpi_rsdt))) 
	{
		KPrintF("RSDT: bad checksum\n");
		return;
	}
	
	if ((acpi_xsdt) && (!acpi_sdt_check((uint8_t *) acpi_xsdt))) {
		KPrintF("XSDT: bad checksum\n");
		return;
	}
	
	if 		(acpi_xsdt)	configure_via_xsdt();
	else if (acpi_rsdt)	configure_via_rsdt();
}


