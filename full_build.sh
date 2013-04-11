#!/bin/bash

make clean
make

cp ./bin/kernel.bin ./bin/kernel

losetup /dev/loop10 ./floppy.img
mount /dev/loop10 /mnt/floppy
cp ./bin/kernel /mnt/floppy
umount /dev/loop10
losetup -d /dev/loop10

#../qemu/bin/qemu -L ../qemu/share/ -no-kqemu -m 64 -fda ./floppy.img
qemu -m 64 -vga vmware -serial stdio -fda ./floppy.img

