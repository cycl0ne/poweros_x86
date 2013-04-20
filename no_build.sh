#!/bin/bash

#make clean
#make

cp ./bin/kernel.bin ./bin/kernel

losetup /dev/loop10 ./harddisk.img
mount /dev/loop10 /mnt/floppy
cp ./bin/kernel /mnt/floppy
cp ./bin/kernel.bin /mnt/floppy
umount /dev/loop10
losetup -d /dev/loop10

#../qemu/bin/qemu -L ../qemu/share/ -no-kqemu -m 64 -hda ./harddisk.img
qemu-system-i386 -m 64 -vga vmware -serial stdio -hda ./harddisk.img
