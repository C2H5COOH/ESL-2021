/*
 *
 * Copyright (c) 2005-2021 Imperas Software Ltd., www.imperas.com
 *
 * The contents of this file are provided under the Software License
 * Agreement that you accepted before downloading this file.
 *
 * This source forms part of the Software and can be used for educational,
 * training, and demonstration purposes but cannot be used for derivative
 * works except in cases where the derivative works require OVP technology
 * to run.
 *
 * For open source models released under licenses that you can use for
 * derivative works, please visit www.OVPworld.org or www.imperas.com
 * for the location of the open source models.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dmacRegisters.h"

typedef unsigned int  Uns32;
typedef unsigned char Uns8;

#include "riscvInterrupts.h"

#define LOG(_FMT, ...)  printf( "CPU1: " _FMT,  ## __VA_ARGS__)

#define INIT_ADDR   BUS_BASE + 0x200400
#define TARG_ADDR   BUS_BASE + 0x300000
#define END_ADDR    BUS_BASE + 0x2007FF
#define MOT_ADDR    BUS_BASE + 0x200800

#define ENABLE      0x00000001
#define INTEN       0x00008000
// burst size is 1<<BURST_SIZE
#define BURST_SIZE       2

void int_init(void (*handler)()) {

	// Set MTVEC register to point to handler function in direct mode
	int handler_int = (int) handler & ~0x1;
	write_csr(mtvec, handler_int);

	// Enable Machine mode external interrupts
    set_csr(mie, MIE_MEIE);
}

void int_enable() {
    set_csr(mstatus, MSTATUS_MIE);
}

static inline void writeReg32(Uns32 address, Uns32 offset, Uns32 value)
{
    *(volatile Uns32*) (address + offset) = value;
}

static inline Uns32 readReg32(Uns32 address, Uns32 offset)
{
    return *(volatile Uns32*) (address + offset);
}

static inline void writeReg8(Uns32 address, Uns32 offset, Uns8 value)
{
    *(volatile Uns8*) (address + offset) = value;
}

static inline Uns8 readReg8(Uns32 address, Uns32 offset)
{
    return *(volatile Uns8*) (address + offset);
}

volatile static Uns32 interruptCount = 0;

void interruptHandler(void)
{
    LOG("Interrupt received\n");
    //Uns32 intStatus = readReg8(DMA_BASE, DMA_INT_TC_STATUS);    // read interrupt status

    // check channel 0 interrupts enabled and status indicates interrupt set
    /*if ( (readReg32(DMA_BASE, DMA_C0_CONFIGURATION) & 0x8000 ) &&
         (intStatus & 1<<0)){
        LOG("Interrupt ch0 0x%x (0x%02x)\n", readReg32(DMA_BASE, DMA_C0_CONFIGURATION),
                                             intStatus);
        writeReg32(DMA_BASE, DMA_C0_CONFIGURATION, 0);      // disable ch0 interrupt
        writeReg8(DMA_BASE, DMA_INT_TC_STATUS,     1<<0);   // clear ch0 interrupt
        interruptCount++;
    }

    // check channel 1 interrupts enabled and status indicates interrupt set
    else if ( (readReg32(DMA_BASE, DMA_C1_CONFIGURATION) & 0x8000 ) &&
         (intStatus & 1<<1)){
        LOG("Interrupt ch1 0x%x (0x%02x)\n", readReg32(DMA_BASE, DMA_C1_CONFIGURATION),
                                             intStatus);
        writeReg32(DMA_BASE, DMA_C1_CONFIGURATION, 0);      // disable ch1 interrupt
        writeReg8(DMA_BASE, DMA_INT_TC_STATUS,     1<<1);   // clear ch1 interrupt
        interruptCount++;
    }*/
    writeReg32(DMA_CTRL1, DMA_STARTCLR, 0);
}

/*static void dmaBurst(Uns32 ch, void *from, void *to, Uns32 bytes)
{
    Uns32 offset = ch * DMA_CHANNEL_STRIDE;
    LOG("dmaBurst ch:%d  bytes:%d\n", ch, bytes);
    writeReg32(DMA_BASE, DMA_C0_SRC_ADDR + offset, (Uns32)from);
    writeReg32(DMA_BASE, DMA_C0_DST_ADDR + offset, (Uns32)to);
    writeReg32(DMA_BASE, DMA_C0_CONTROL  + offset, bytes);

    writeReg32(DMA_BASE, DMA_C0_CONFIGURATION + offset, ENABLE|INTEN);
}*/

static void dmaCmd(Uns32 src, Uns32 tgt, Uns32 len){
    writeReg32(DMA_CTRL1, DMA_SRC, src);
    writeReg32(DMA_CTRL1, DMA_TGT, tgt);
    writeReg32(DMA_CTRL1, DMA_SIZE, len);
    writeReg32(DMA_CTRL1, DMA_STARTCLR, 1);
    wfi();
}

static void dumpMemory(int ram){
    int start = BUS_BASE;
    start += (ram == 3)? 0x200000 : 0x300000;
    int end = start + 0x000400;

    printf("%x, %x\n", start, end);

    printf("==============================================\n");
    if(ram == 3)    printf("RAM3 data content\n");
    else            printf("RAM4 data content\n");
    printf("==============================================\n");
    Uns8 data;
    int i = 0;
    while(i<64){
        printf("| addr = 0x%x | data = ", start - BUS_BASE);
        for(int j=0; j<16; j++){
            printf("%c", readReg8(start+j, 0));
        }
        printf(" |\n");
        start += 16;
        i++;
    }
    printf("==============================================\n");
}


int main(int argc, char **argv)
{
    const Uns8 init_data[16] = "0123456789ABCDEF";
    int_init(trap_entry);
    int_enable();

    LOG("Start to write initial data\n");

    // write initial data
    int i, j;
    for(i=0; i<64; i++){
        for(j=0; j<16; j++){
            writeReg8(INIT_ADDR, i*16+j, init_data[j]);
            // printf("%d, %d\n", i, j);
        }
    }

    Uns8 data;
    // for(i=0; i<64; i++){
    //     for(j=0; j<16; j++){
    //         data = readReg8(INIT_ADDR, i*16+j);
    //         printf("%c", data);
    //     }
    //     printf("\n");
    // }

    // Uns8 data;
    // for(i=0; i<64; i++){
    //     for(j=0; j<16; j++){
    //         data = readReg8(INIT_ADDR, i*64+j);
    //         printf("%c", data);
    //     }
    //     printf("\n");
    // }
    
    for(i=0; i<3; i++){
        while(readReg8(MOT_ADDR, 0) != 0x0){
            // printf("%d\n", readReg8(MOT_ADDR, 0));
        }
        LOG("Write data for %d time.\n", i+1);
        dmaCmd(INIT_ADDR, TARG_ADDR, 1024);
        dumpMemory(4);
        writeReg8(MOT_ADDR, 0, 0x1);

    }
    
    // for(i=0; i<64; i++){
    //     for(j=0; j<16; j++){
    //         data = readReg8(TARG_ADDR, i*64+j);
    //         printf("%c", data);
    //     }
    //     printf("\n");
    // }
    LOG("Simulation complete.\n");

    return 1;
}

