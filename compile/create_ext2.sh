#!/bin/bash

#genext2fs -d ../hdd -B 4096 -U -N 4096 -b 25600 harddisk2.img

# prepare a disk to store data
if [ ! -f "harddisk2.img" ]
then
dd if=/dev/zero of=harddisk2.img bs=4096 count=2560
mkfs.ext2 -F -N 4096 -b 4096 harddisk2.img 2560
fi
# copy data to it
mkdir -p /tmp/harddisk2
sudo mount -o loop harddisk2.img /tmp/harddisk2
sudo cp -r ../hdd/* /tmp/harddisk2
sudo umount /tmp/harddisk2

