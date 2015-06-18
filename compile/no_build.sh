#!/bin/bash

#make clean
#make
#make apps
#make install


kernel_binary="bin/kernel.bin"


# now run quemu
qemu-system-x86_64 -m 64 -smp sockets=1,cores=2,threads=2 -nographic -monitor telnet:127.0.0.1:1234,server,nowait -serial stdio -hda ./harddisk2.img -kernel "$kernel_binary" -drive file=harddisk3.img,if=virtio






