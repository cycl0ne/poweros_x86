#!/bin/bash

# ---- begin config params ----

harddisk_image="harddisk2.img"
qemu_cmdline="qemu-system-x86_64  -serial stdio -nographic -monitor telnet:127.0.0.1:1234,server,nowait"   #pty   -monitor stdio "
#qemu_cmdline="qemu-system-i386 -sdl -serial stdio -vga std -serial stdio"
kernel_args=""
kernel_binary="bin/kernel.bin"

# ----  end config params  ----

# run QEMU
$qemu_cmdline -hda "$harddisk_image" -kernel "$kernel_binary" -drive file=harddisk3.img,if=virtio 
#-drive file=harddisk.img,if=virtio -drive file=harddisk4.img,if=virtio -drive file=harddisk5.img,if=virtio

echo
