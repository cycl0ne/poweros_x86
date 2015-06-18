#include "types.h"
#include "arch_acpi.h"
#include "arch_madt.h"
#include "arch_config.h"

#include "exec_interface.h"

extern struct acpi_rsdp *acpi_rsdp;
extern struct acpi_rsdt *acpi_rsdt;
extern struct acpi_xsdt *acpi_xsdt;
extern struct acpi_madt *acpi_madt;


extern pSysBase g_SysBase;
#define SysBase g_SysBase

extern arch_config config;

void acpi_madt_parse(void);

void smp_init(void)
{
	if (acpi_madt) {
		KPrintF("%s\n", __FUNCTION__);
		acpi_madt_parse();
		//ops = &madt_config_operations;
	}

	KPrintF("CPU Count = %d\n", config.cpu_count);
}

