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

static APTR CreateInitTask(SysBase *SysBase, Task *newTask, char *name, APTR codeStart, APTR data, UINT32 stackSize, INT8 pri)
{
	if (newTask==NULL) return NULL;
	
	newTask->Stack = AllocVec(stackSize, MEMF_FAST|MEMF_CLEAR);
	if (newTask->Stack == NULL)  return NULL;

	newTask->Flags 		= TF_CREATETASKSTACK;
	newTask->Node.ln_Pri= pri;
	newTask->Switch		= NULL;
	newTask->Launch		= NULL;
	newTask->StackSize	= stackSize;
	newTask->tc_SPLower = (UINT32) newTask->Stack;
	newTask->tc_SPUpper = (UINT32) newTask->Stack + stackSize;
	if (name == NULL) 	newTask->Node.ln_Name = "UnknownTask";
	else newTask->Node.ln_Name = name;

	return AddTask(newTask, codeStart, NULL, data);
}

void ExecInit(void)
{
	SysBase *SysBase = INTERN_CreateSysBase(&config);
	if (SysBase == NULL)
	{
		monitor_write("[PANIC] No SysBase created\n");
		for(;;);
	}
	DPrintF("\n%s ______________________________________\n", config.arch_name);

	SysBase->ExecTask.Node.ln_Name = "ExecTask";
	SysBase->thisTask = &SysBase->ExecTask;			// Fake a Task for AllocVec

	// Create two clean Task, one IDLE Task and one Worker Task with Prio 100
	CreateInitTask(SysBase, &SysBase->ExecTask, "ExecTask"	, lib_ExecTask	, SysBase, _EXEC_TASK_STACK_, 100);
	CreateInitTask(SysBase, &SysBase->IdleTask, "idle"		, lib_Idle		, SysBase, _IDLE_TASK_STACK_, -124); 

	arch_irq_init(); 			// Remap IRQ
	arch_irq_create(SysBase); 	// Create IRQ/Exc Handlers/Servers
	arch_clk_init(SysBase);		// create Clock

	if (RomTagScanner(config.base, (UINT32 *)(config.base + config.kernel_size)) == FALSE)
	{
		monitor_write("[PANIC] RomTagScanner FAILED!\n");
		for(;;);
	}
	InitResidentCode(RTF_SINGLETASK);
	DPrintF("[INIT] Activating Multitasking -> Leaving SingleTask\n");
	SysBase->thisTask = NULL;	// Remove Fake Task for proper Schedule
	Permit();					// Sysbase was initialised with a 0 in TDNestCnt, we set it here to -1 to enable Taskswitching
	DPrintF("[INIT] Schedule -> leaving Kernel Init\n");
	Schedule();

	DPrintF("[INIT] PONR (point of no return\n");
	asm volatile("int $0x1");
	for(;;);
}
