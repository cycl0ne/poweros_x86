#include "types.h"
#include "sysbase.h"
#include "_debug.h"
#include "context.h"
#include "arch_config.h"
#include "exec_funcs.h"

void gdt_init();
void arch_exc_init();
void arch_irq_init();
void arch_irq_create(SysBase *SysBase);
void arch_clk_init(SysBase *SysBase);

SysBase *INTERN_CreateSysBase(arch_config *config);
void lib_Idle(SysBase *SysBase);
void lib_ExecTask(SysBase *SysBase);
extern arch_config config;

#define _IDLE_TASK_STACK_ 4096
#define _EXEC_TASK_STACK_ 4096

void __Debugger()
{
	monitor_write("[PANIC] DEBUGGER\n");
}

//static inline void memset32(void *dest, UINT32 value, UINT32 size) { asm volatile ("cld; rep stosl" : "+c" (size), "+D" (dest) : "a" (value) : "memory"); }
void d_showtask(struct SysBase *SysBase);

void ExecInit(void)
{
	SysBase *SysBase = INTERN_CreateSysBase(&config);
	if (SysBase == NULL)
	{
		monitor_write("[PANIC] No SysBase created\n");
		for(;;);
	}
	DPrintF("\n%s ______________________________________\n", config.arch_name);

//DPrintF("SysBase: %x\n", SysBase);
//	memset32((APTR)0x230000, 0x00, 0x200000);
//	monitor_write("[PANIC] Memset\n");
//	DPrintF("Memset\n");	

	
//	monitor_write("[SysBase]\n");
//	monitor_write_hex((UINT32)SysBase);
//	monitor_put('\n');

	// Remap IRQ
	arch_irq_init();
	// Create IRQ/Exc Handlers/Servers
	arch_irq_create(SysBase);
	// create Clock
	arch_clk_init(SysBase);

//	while(1);

	
	// Create two clean Task, one IDLE Task and one Worker Task with Prio 100
	Task *task1 = TaskCreate("idle", lib_Idle, SysBase, _IDLE_TASK_STACK_, -124); 
	Task *task2 = TaskCreate("ExecTask", lib_ExecTask, SysBase, _EXEC_TASK_STACK_, 100);
	if (task1 == task2) DPrintF("Couldnt allocate Task\n");
	if (RomTagScanner(config.base, (UINT32 *)(config.base + config.kernel_size)) == FALSE)
	{
		monitor_write("[PANIC] RomTagScanner FAILED!\n");
		for(;;);
	}
	InitResidentCode(RTF_SINGLETASK);
	DPrintF("[INIT] Activating SysBase Permit/Enable -> Leaving SingleTask\n");
	Permit();
//	asm volatile("sti");
	//UINT32 ipl = Disable();
	//Enable(ipl);

	DPrintF("[INIT] Schedule -> leaving Kernel Init\n");
//	for(;;);
//d_showtask(SysBase);

	Schedule();
	DPrintF("[INIT] PONR (point of no return\n");
	asm volatile("int $0x1");
	for(;;);
}

/*
RETIRED
// DBUG
monitor_write("[PANIC] AFTEr EXCINT\n");	
    asm volatile("int $0x3");
monitor_write("[PANIC] AFTER INT0x3\n");	
for(;;);
//DEBUG
//
	monitor_clear();
*/
