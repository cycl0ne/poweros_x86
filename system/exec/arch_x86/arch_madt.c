#include "types.h"
#include "arch_acpi.h"
#include "arch_madt.h"
#include "arch_config.h"

#include "exec_interface.h"

extern struct acpi_rsdp *acpi_rsdp;
extern struct acpi_rsdt *acpi_rsdt;
extern struct acpi_xsdt *acpi_xsdt;
extern struct acpi_madt *acpi_madt;

extern arch_config config;

static int isa_irq_map[] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

struct madt_l_apic *madt_l_apic_entries = NULL;
struct madt_io_apic *madt_io_apic_entries = NULL;

static size_t madt_l_apic_entry_index = 0;
static size_t madt_io_apic_entry_index = 0;
static size_t madt_l_apic_entry_cnt = 0;
static size_t madt_io_apic_entry_cnt = 0;

static struct madt_apic_header **madt_entries_index = NULL;

const char *entry[] = {
	"L_APIC",
	"IO_APIC",
	"INTR_SRC_OVRD",
	"NMI_SRC",
	"L_APIC_NMI",
	"L_APIC_ADDR_OVRD",
	"IO_SAPIC",
	"L_SAPIC",
	"PLATFORM_INTR_SRC"
};

extern pSysBase g_SysBase;
#define SysBase g_SysBase

uint32_t apic_id_mask = 0;
volatile uint32_t *l_apic = (uint32_t *) UINT32_C(0xfee00000);
volatile uint32_t *io_apic = (uint32_t *) UINT32_C(0xfec00000);

static void madt_l_apic_entry(struct madt_l_apic *la, size_t i)
{
	if (madt_l_apic_entry_cnt == 0) madt_l_apic_entry_index = i;
	
	madt_l_apic_entry_cnt++;
	
	if (!(la->flags & 0x1)) 
	{
		/* Processor is unusable, skip it. */
		return;
	}
	
	apic_id_mask |= 1 << la->apic_id;
}

static void madt_io_apic_entry(struct madt_io_apic *ioa, size_t i)
{
	if (madt_io_apic_entry_cnt == 0) {
		/* Remember index of the first io apic entry */
		madt_io_apic_entry_index = i;
		io_apic = (uint32_t *) (uint32_t) ioa->io_apic_address;
	} else {
		/* Currently not supported */
	}
	
	madt_io_apic_entry_cnt++;
}

static void madt_intr_src_ovrd_entry(struct madt_intr_src_ovrd *override, size_t i)
{
	//ASSERT(override->source < sizeof(isa_irq_map) / sizeof(int));
	isa_irq_map[override->source] = override->global_int;
}

void acpi_madt_parse(void)
{
	struct madt_apic_header *end = (struct madt_apic_header *) (((uint8_t *) acpi_madt) + acpi_madt->header.length);
	struct madt_apic_header *hdr;
	
	l_apic = (uint32_t *) (uint32_t) acpi_madt->l_apic_address;
	
	/* Count MADT entries */
	uint32_t madt_entries_index_cnt = 0;
	for (hdr = acpi_madt->apic_header; hdr < end; hdr = (struct madt_apic_header *) (((uint8_t *) hdr) + hdr->length)) madt_entries_index_cnt++;
	
	/* Create MADT APIC entries index array */
	KPrintF("%s: Before Alloc\n", __FUNCTION__);
	
	madt_entries_index = (struct madt_apic_header **) AllocVec(madt_entries_index_cnt * sizeof(struct madt_apic_header *), MEMF_FAST);
	if (!madt_entries_index) {KPrintF("Memory allocation error.\n"); for(;;);}
	
	KPrintF("%s: After Alloc\n", __FUNCTION__);
	size_t i = 0;
	
	for (hdr = acpi_madt->apic_header; hdr < end; hdr = (struct madt_apic_header *) (((uint8_t *) hdr) + hdr->length)) 
	{
		madt_entries_index[i] = hdr;
		i++;
	}
	
	/* Sort MADT index structure */
//	if (!gsort(madt_entries_index, madt_entries_index_cnt, sizeof(struct madt_apic_header *), madt_cmp, NULL))
//		panic("Sorting error.");
	
	/* Parse MADT entries */
	for (i = 0; i < madt_entries_index_cnt; i++) 
	{
		hdr = madt_entries_index[i];
		
		switch (hdr->type) 
		{
		case MADT_L_APIC:
			madt_l_apic_entry((struct madt_l_apic *) hdr, i);
			break;
		case MADT_IO_APIC:
			madt_io_apic_entry((struct madt_io_apic *) hdr, i);
		break;
		case MADT_INTR_SRC_OVRD:
			madt_intr_src_ovrd_entry((struct madt_intr_src_ovrd *) hdr, i);
			break;
		case MADT_NMI_SRC:
		case MADT_L_APIC_NMI:
		case MADT_L_APIC_ADDR_OVRD:
		case MADT_IO_SAPIC:
		case MADT_L_SAPIC:
		case MADT_PLATFORM_INTR_SRC:
			KPrintF("MADT: Skipping %s entry (type=%d)\n", entry[hdr->type], hdr->type);
			break;
		default:
			if ((hdr->type >= MADT_RESERVED_SKIP_BEGIN) && (hdr->type <= MADT_RESERVED_SKIP_END))
				KPrintF("MADT: Skipping reserved entry (type=%d)\n", hdr->type);
				
			if (hdr->type >= MADT_RESERVED_OEM_BEGIN)
				KPrintF("MADT: Skipping OEM entry (type=%d)\n", hdr->type);
			
			break;
		}
	}
	
	if (madt_l_apic_entry_cnt > 0) config.cpu_count = madt_l_apic_entry_cnt;
}

