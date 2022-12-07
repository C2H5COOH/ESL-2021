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

//////////////////////// Registers for DMA Controller based on ARM PL081 ////////////////////////////

#ifndef DMAC_REGISTERS_H
#define DMAC_REGISTERS_H

#define BUS_BASE            0x100000
#define DMA_BASE            BUS_BASE + 0x100000

#define DMA_CTRL1           DMA_BASE + 0x0
#define DMA_CTRL2           DMA_BASE + 0x10
#define DMA_SRC             0x0 
#define DMA_TGT             0x4
#define DMA_SIZE            0x8
#define DMA_STARTCLR        0xc            

#endif
