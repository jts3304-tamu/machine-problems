# Replace "/mnt/floppy" with the whatever directory is appropriate.
# sudo mount -o loop dev_kernel_grub.img /mnt/floppy
# sudo cp kernel.bin /mnt/floppy
# sleep 1s
# sudo umount /mnt/floppy

mkdir -p ../mnt/floppy
hdiutil attach -mountpoint ../mnt/floppy dev_kernel_grub.img
cp kernel.bin ../mnt/floppy/ # do i need this?
sleep 1
hdiutil detach ../mnt/floppy
rm -rf ../mnt/floppy