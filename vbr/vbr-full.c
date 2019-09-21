/*
 * A simple bootloader skeleton for x86, using gcc.
 *
 * Prashant Borole (boroleprashant at Google mail)
 * */

/* XXX these must be at top */
#include "code16gcc.h"
__asm__ ("jmpl  $0, $main\n");


#define __NOINLINE  __attribute__((noinline))
#define __REGPARM   __attribute__ ((regparm(3)))
#define __PACKED    __attribute__((packed))
#define __NORETURN  __attribute__((noreturn))

#define IMAGE_SIZE  8192
#define BLOCK_SIZE  512
#define IMAGE_LMA   0x8000
#define IMAGE_ENTRY 0x800c

/* BIOS interrupts must be done with inline assembly */
void    __NOINLINE __REGPARM print(const char   *s){
        while(*s){
                __asm__ __volatile__ ("int  $0x10" : : "a"(0x0E00 | *s), "b"(7));
                s++;
        }
}

#if 0
/* use this for the HD/USB/Optical boot sector */
typedef struct __PACKED TAGaddress_packet_t{
    char                size;
    char                :8;
    unsigned short      blocks;
    unsigned short      buffer_offset;
    unsigned short      buffer_segment;
    unsigned long long  lba;
    unsigned long long  flat_buffer;
}address_packet_t ;

int __REGPARM lba_read(const void   *buffer, unsigned int   lba, unsigned short blocks, unsigned char   bios_drive){
        int i;
        unsigned short  failed = 0;
        address_packet_t    packet = {.size = sizeof(address_packet_t), .blocks = blocks, .buffer_offset = 0xFFFF, .buffer_segment = 0xFFFF, .lba = lba, .flat_buffer = (unsigned long)buffer};
        for(i = 0; i < 3; i++){
                packet.blocks = blocks;
                __asm__ __volatile__ (
                                "movw   $0, %0\n"
                                "int    $0x13\n"
                                "setcb  %0\n"
                                :"=m"(failed) : "a"(0x4200), "d"(bios_drive), "S"(&packet) : "cc" );
                /* do something with the error_code */
                if(!failed)
                        break;
        }
        return failed;
}
#else
/* use for floppy, or as a fallback */
typedef struct {
        unsigned char   spt;
        unsigned char   numh;
}drive_params_t;

int __REGPARM __NOINLINE get_drive_params(drive_params_t    *p, unsigned char   bios_drive){
        unsigned short  failed = 0;
        unsigned short  tmp1, tmp2;
        __asm__ __volatile__
            (
             "movw  $0, %0\n"
             "int   $0x13\n"
             "setcb %0\n"
             : "=m"(failed), "=c"(tmp1), "=d"(tmp2)
             : "a"(0x0800), "d"(bios_drive), "D"(0)
             : "cc", "bx"
            );
        if(failed)
                return failed;
        p->spt = tmp1 & 0x3F;
        p->numh = tmp2 >> 8;
        return failed;
}

int __REGPARM __NOINLINE lba_read(const void    *buffer, unsigned int   lba, unsigned char  blocks, unsigned char   bios_drive, drive_params_t  *p){
        unsigned char   c, h, s;
        c = lba / (p->numh * p->spt);
        unsigned short  t = lba % (p->numh * p->spt);
        h = t / p->spt;
        s = (t % p->spt) + 1;
        unsigned char   failed = 0;
        unsigned char   num_blocks_transferred = 0;
        __asm__ __volatile__
            (
             "movw  $0, %0\n"
             "int   $0x13\n"
             "setcb %0"
             : "=m"(failed), "=a"(num_blocks_transferred)
             : "a"(0x0200 | blocks), "c"((s << 8) | s), "d"((h << 8) | bios_drive), "b"(buffer)
            );
        return failed || (num_blocks_transferred != blocks);
}
#endif

/* and for everything else you can use C! Be it traversing the filesystem, or verifying the kernel image etc.*/

void __NORETURN main(){
        unsigned char   bios_drive = 0;
        __asm__ __volatile__("movb  %%dl, %0" : "=r"(bios_drive));      /* the BIOS drive number of the device we booted from is passed in dl register */

        drive_params_t  p = {};
        get_drive_params(&p, bios_drive);

        void    *buff = (void*)IMAGE_LMA;
        unsigned short  num_blocks = ((IMAGE_SIZE / BLOCK_SIZE) + (IMAGE_SIZE % BLOCK_SIZE == 0 ? 0 : 1));
        if(lba_read(buff, 1, num_blocks, bios_drive, &p) != 0){
            print("read error :(\r\n");
            while(1);
        }
        print("Running next image...\r\n");
        void*   e = (void*)IMAGE_ENTRY;
        __asm__ __volatile__("" : : "d"(bios_drive));
        goto    *e;
}
