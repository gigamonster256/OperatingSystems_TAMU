mkdir ./floppy
hdiutil attach dev_kernel_grub.img -mountpoint ./floppy
cp kernel.elf ./floppy
sleep 1
hdiutil detach ./floppy
rmdir ./floppy
