poweros_x86
===========

An experimental OS called PowerOS for the x86 platform

Screenshots:
AUX Handler - Shell
![Screenshot of AUX device/handler](http://i.imgur.com/vZJoz7A.jpg)
New console.device in action with nyancat animation to check ANSI Code Handling.
![Screenshot of new vga mode console driver+test](http://i.imgur.com/9cJsqa3.jpg)


Summary:
========

This directory contains the source code for a experimental OS called PowerOS,
which I ported to the Raspberry PI as an educational OS. I now ported it back to x86,
because on my long business trips im in lack of a rasperry pi development stack.
It is partly inspired by the good old AmigaOS, but it is not compatible. Some
Functions are the same others dont. So dont rely on it.

Playing around:
===============
I know that there is only textoutput at the moment, but pe patient :) if you want to test things for yourselve, please go into boot.test. Since there is no disk operations at the moment you need to get booted from the kernel file.
In boot.test you will find a .c file, inside it is a test_main or something similar. This is your starting point to test the API of the OS. I implemented a small load routine for the ps2mouse device driver. This is why the screens fills when you move your mouse.
Following output is generated on the STDIO of Qemu:

PowerOS x86 ______________________________________  
[INIT] RTF_SINGLETASK  
InitResident expansion.library (0x140240)  
InitResident vgagfx.library (0x13fec0)  
[INIT] Activating SysBase Permit/Enable -> Leaving SingleTask  
[INIT] Schedule -> leaving Kernel Init  
[EXECTASK] RTF_COLDSTART  
InitResident utility.library (0x13c0c0)  
InitResident region.library (0x13ff60)  
InitResident coregfx.library (0x13f540)  
InitResident mouseport.device (0x140320)  
InitResident timer.device (0x140020)  
InitResident keyboard.device (0x1408e0)  
InitResident input.device (0x1428c0)  
InitResident test (0x142980)  


Status:
=======
At the moment im reporting the Raspberry Pi Source back to x86. The system boots up at the moment,
shows debug output, scans the PCI bus and initializes timer.device and a mouseport.device. So nothing
special at the moment.

Directory Hierarchy:
====================

* root/src/kickstart -> Here you find all files that are needed to "kickstart" the system. 
* root/makefile -> makefile for the project
* root/run.sh -> Run script for QEMU to boot up the kernel

License:
========
While chekcing which license fits best, take this as a note:
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 - The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
