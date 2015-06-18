#!/bin/bash

# ---- begin config params ----

harddisk_image="harddisk2.img"
#qemu_cmdline="qemu-system-i386  -serial stdio -hdb harddisk2.img -nographic -monitor telnet:127.0.0.1:1234,server,nowait"   #pty   -monitor stdio "

qemu_cmdline="qemu-system-x86_64 -sdl -serial stdio -vga std -serial stdio"
kernel_args=""
kernel_binary="bin/kernel.bin"

# ----  end config params  ----

# run QEMU
$qemu_cmdline -m 128 -hda "$harddisk_image" -kernel "$kernel_binary" -drive file=harddisk3.img,if=virtio 

echo
