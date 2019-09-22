all:
	gcc -c -g -O0 -march=x86-64 -m32 -ffreestanding -Wall -Werror -I. -o vbr.o vbr.c
	ld  -m elf_i386 -static -Tlinker.ld -nostdlib --nmagic -o vbr.elf vbr.o
	objcopy -O binary vbr.elf vbr.bin
	dd if=/dev/zero of=floppy.img bs=1024 count=1440
	dd if=vbr.bin of=floppy.img bs=1 count=512 conv=notrunc
	qemu-system-x86_64  -nographic -hda floppy.img -boot a -S -s & \
	gdb -ix ../gdbinit_real_mode.txt vbr.elf \
        -ex 'target remote localhost:1234' \
        -ex 'break main' \
        -ex 'continue'


