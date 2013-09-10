#include "virtiodev.h"
#include "sysbase.h"
#include "resident.h"
#include "exec_funcs.h"
#include "expansion_funcs.h"
#include "lib_virtio.h"
#include "arch_config.h"

#define DEVICE_VERSION_STRING "\0$VER: virtioblk.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

static const char name[] = "virtioblk.device";
static const char version[] = DEVICE_VERSION_STRING
static const char EndResident;

APTR vdev_OpenDev(struct VDBase *VDBase, struct IORequest *ioreq, UINT32 unitnum, UINT32 flags);
APTR vdev_CloseDev(struct VDBase *VDBase, struct IORequest *ioreq);
APTR vdev_ExpungeDev(struct VDBase *VDBase);
APTR vdev_ExtFuncDev(struct VDBase *VDBase);
void vdev_BeginIO(VDBase *VDBase, struct IORequest *ioreq);
void vdev_AbortIO(VDBase *VDBase, struct IORequest *ioreq);
static struct VDBase *vdev_Init(struct VDBase *VDBase, UINT32 *segList, struct SysBase *SysBase);


static APTR FuncTab[] = 
{
	(void(*)) vdev_OpenDev,
	(void(*)) vdev_CloseDev,
	(void(*)) vdev_ExpungeDev,
	(void(*)) vdev_ExtFuncDev,

	(void(*)) vdev_BeginIO,
	(void(*)) vdev_AbortIO,
	(APTR) ((UINT32)-1)
};

static const struct VDBase VDevData =
{
	.vd_Device.dd_Library.lib_Node.ln_Name = (APTR)&name[0],
	.vd_Device.dd_Library.lib_Node.ln_Type = NT_DEVICE,
	.vd_Device.dd_Library.lib_Node.ln_Pri = 50,
	.vd_Device.dd_Library.lib_OpenCnt = 0,
	.vd_Device.dd_Library.lib_Flags = LIBF_SUMUSED|LIBF_CHANGED,
	.vd_Device.dd_Library.lib_NegSize = 0,
	.vd_Device.dd_Library.lib_PosSize = 0,
	.vd_Device.dd_Library.lib_Version = DEVICE_VERSION,
	.vd_Device.dd_Library.lib_Revision = DEVICE_REVISION,
	.vd_Device.dd_Library.lib_Sum = 0,
	.vd_Device.dd_Library.lib_IDString = (APTR)&version[7],
};

static volatile const APTR InitTab[4]=
{
	(APTR)sizeof(struct VDBase),
	(APTR)FuncTab,
	(APTR)&VDevData,
	(APTR)vdev_Init
};

static volatile const struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT|RTF_COLDSTART,
	DEVICE_VERSION,
	NT_DEVICE,
	50,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

UINT32 vdev_InputTask(APTR data, struct SysBase *SysBase);

static struct VDBase *vdev_Init(struct VDBase *VDBase, UINT32 *segList, struct SysBase *SysBase)
{
	VDBase->vd_SysBase	= (APTR)SysBase;
	VDBase->vd_BootTask = FindTask(NULL);

	VDBase->vd_ExpansionBase = OpenLibrary("expansion.library", 0);
	if (VDBase->vd_ExpansionBase == NULL) {
		DPrintF("virtio_blk_InitDev: Cant open expansion.library\n");
		return NULL;
	}

//	VDBase->vd_VirtIOBase = (struct LibVirtioBase *)OpenLibrary("virtio.library", 0);
//	if (VDBase->vd_VirtIOBase == NULL) {
//		DPrintF("virtio_blk_InitDev: Cant open virtio.library\n");
//		return NULL;
//	}

	return VDBase;
}

static const char EndResident = 0;
