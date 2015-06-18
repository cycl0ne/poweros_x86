#!/bin/bash

# ---- begin config params ----
harddisk_image="harddisk2.img"

qemu_cmdline="qemu-system-x86_64 -sdl -serial stdio -vga std"
kernel_args=""
kernel_binary="bin/kernel.bin"

# ----  end config params  ----

# run QEMU
$qemu_cmdline -m 128 -hda "$harddisk_image" -kernel "$kernel_binary" -drive file=harddisk3.img,if=virtio 

echo
