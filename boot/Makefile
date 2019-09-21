all:
	as -o boot.o boot.s
	ld -o boot.bin --oformat binary -e init -Ttext 0x7c00 boot.o
	qemu-system-x86_64 boot.bin -nographic
