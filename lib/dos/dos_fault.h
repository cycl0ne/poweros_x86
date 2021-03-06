

INT32 errornums[] = {
	-162,-100,
	103,103,
	105,105,
	114,122,
	202,207,
	209,226,
	232,236,
	240,243,
	303,305,
	0,
};

UINT8 dos_errstrs[] = {
    /* -162 */ "\x11" "Software Failure\0"
    /* -161 */ "\x1a" "%s failed returncode %ld\n\0"
    /* -160 */ "\x10" "%TH USE COUNT\n\n\0"
    /* -159 */ "\x05" "NAME\0"
    /* -158 */ "\x0e" "%TH DISABLED\n\0"
    /* -157 */ "\x0e" "%TH INTERNAL\n\0"
    /* -156 */ "\x0c" "%TH SYSTEM\n\0"
    /* -155 */ "\x0b" "Fault %3ld\0"
    /* -154 */ "\x11" "Fail limit: %ld\n\0"
    /* -153 */ "\x1a" "Bad return code specified\0"
    /* -152 */ "\x12" "Current_directory\0"
    /* -151 */ "\x2b" "The last command did not set a return code\0"
    /* -150 */ "\x1c" "Last command failed because\0"
    /* -149 */ "\x12" "Process %N ending\0"
    /* -148 */ "\x19" "Requested size too small\0"
    /* -147 */ "\x19" "Requested size too large\0"
    /* -146 */ "\x21" "Current stack size is %ld bytes\n\0"
    /* -145 */ "\x10" "NewShell failed\0"
    /* -144 */ "\x16" "Missing ELSE or ENDIF\0"
    /* -143 */ "\x1a" "Must be in a command file\0"
    /* -142 */ "\x20" "More than one directory matches\0"
    /* -141 */ "\x0e" "Can't set %s\n\0"
    /* -140 */ "\x1c" "Block %ld corrupt directory\0"
    /* -139 */ "\x17" "Block %ld corrupt file\0"
    /* -138 */ "\x1a" "Block %ld bad header type\0"
    /* -137 */ "\x17" "Block %ld out of range\0"
    /* -136 */ "\x15" "Block %ld used twice\0"
    /* -135 */ "\x14" "error validating %b\0"
    /* -134 */ "\x12" "on disk block %ld\0"
    /* -133 */ "\x15" "has a checksum error\0"
    /* -132 */ "\x12" "has a write error\0"
    /* -131 */ "\x11" "has a read error\0"
    /* -130 */ "\x1a" "Unable to create process\n\0"
    /* -129 */ "\x17" "New Shell process %ld\n\0"
    /* -128 */ "\x1a" "Cannot open FROM file %s\n\0"
    /* -127 */ "\x0f" "Suspend|Reboot\0"
    /* -126 */ "\x0d" "Retry|Cancel\0"
    /* -125 */ "\x13" "No room for bitmap\0"
    /* -124 */ "\x11" "Command too long\0"
    /* -123 */ "\x0d" "Shell error:\0"
    /* -122 */ "\x16" "Error in command name\0"
    /* -121 */ "\x10" "Unknown command\0"
    /* -120 */ "\x0f" "Unable to load\0"
    /* -119 */ "\x0d" "syntax error\0"
    /* -118 */ "\x20" "unable to open redirection file\0"
    /* -117 */ "\x07" "Error \0"
    /* -116 */ "\x02" "\0\0"
    /* -115 */ "\x1c" "Disk corrupt - task stopped\0"
    /* -114 */ "\x18" "Program failed (error #\0"
    /* -113 */ "\x22" "Wait for disk activity to finish.\0"
    /* -112 */ "\x0f" "in device %s%s\0"
    /* -111 */ "\x0e" "in unit %ld%s\0"
    /* -110 */ "\x18" "You MUST replace volume\0"
    /* -109 */ "\x17" "has a read/write error\0"
    /* -108 */ "\x10" "No disk present\0"
    /* -107 */ "\x0f" "Not a DOS disk\0"
    /* -106 */ "\x0d" "in any drive\0"
    /* -105 */ "\x16" "Please replace volume\0"
    /* -104 */ "\x15" "Please insert volume\0"
    /* -103 */ "\x08" "is full\0"
    /* -102 */ "\x13" "is write protected\0"
    /* -101 */ "\x11" "is not validated\0"
    /* -100 */ "\x07" "Volume\0"

    "\x1C" "not enough memory available\0"
    "\x13" "process table full\0"
    "\x0D" "bad template\0"
    "\x0b" "bad number\0"
    "\x1A" "required argument missing\0"
    "\x1C" "value after keyword missing\0"
    "\x1A" "wrong number of arguments\0"
    "\x11" "unmatched quotes\0"
    "\x22" "argument line invalid or too long\0"
    "\x17" "file is not executable\0"
    "\x19" "invalid resident library\0"
    "\x11" "object is in use\0"
    "\x16" "object already exists\0"
    "\x14" "directory not found\0"
    "\x11" "object not found\0"
    "\x1B" "invalid window description\0"
    "\x11" "object too large\0"
    "\x1C" "packet request type unknown\0"
    "\x14" "object name invalid\0"
    "\x14" "invalid object lock\0"
    "\x1F" "object is not of required type\0"
    "\x13" "disk not validated\0"
    "\x18" "disk is write-protected\0"
    "\x20" "rename across devices attempted\0"
    "\x14" "directory not empty\0"
    "\x10" "too many levels\0"
    "\x22" "device (or volume) is not mounted\0"
    "\x0d" "seek failure\0"
    "\x14" "comment is too long\0"
    "\x0d" "disk is full\0"
    "\x22" "object is protected from deletion\0"
    "\x18" "file is write protected\0"
    "\x17" "file is read protected\0"
    "\x15" "not a valid DOS disk\0"
    "\x11" "no disk in drive\0"
    "\x1D" "no more entries in directory\0"
    "\x14" "object is soft link\0"
    "\x11" "object is linked\0"
    "\x12" "bad loadfile hunk\0"
    "\x19" "function not implemented\0"
    "\x12" "record not locked\0"
    "\x16" "record lock collision\0"
    "\x14" "record lock timeout\0"
    "\x14" "record unlock error\0"
    "\x10" "buffer overflow\0"
    "\x09" "***Break\0"
    "\x14" "file not executable\0"
};

static struct ErrorString dos_errors = {
	errornums,
	dos_errstrs
};
