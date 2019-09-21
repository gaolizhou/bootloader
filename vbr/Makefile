all:
	gcc -c -g -O0 -march=native -ffreestanding -Wall -Werror -I. -o vbr.o vbr.c
	ld -static -Tlinker.ld -nostdlib --nmagic -o vbr.elf vbr.o
	objcopy -O binary vbr.elf vbr.bin
	dd if=/dev/zero of=floppy.img bs=1024 count=1440
	dd if=vbr.bin of=floppy.img bs=1 count=512 conv=notrunc
	qemu -hda floppy.img -boot a

