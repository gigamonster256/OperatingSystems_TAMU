attach=$(hdiutil attach dev_kernel_grub.img)
mountpoint=${attach#* }
# trim leading and trailing whitespace
mountpoint=$(echo $mountpoint | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
device=${attach%% *}
cp kernel.bin "$mountpoint"
sleep 1
hdiutil detach "$device" > /dev/null
