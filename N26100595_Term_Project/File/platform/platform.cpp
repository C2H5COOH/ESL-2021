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

#include "tlm/tlmModule.hpp"
#include "tlm/tlmDecoder.hpp"
#include "tlm/tlmMemory.hpp"

// Processor configuration
#include "riscv.ovpworld.org/processor/riscv/1.0/tlm/riscv_RV32I.igen.hpp"
#include "dma.h"

using namespace sc_core;

////////////////////////////////////////////////////////////////////////////////
//                      BareMetal Class                                       //
////////////////////////////////////////////////////////////////////////////////

class BareMetal : public sc_module {

  public:
    BareMetal (sc_module_name name);

    sc_in<bool>     clk;
    sc_in<bool>     reset;

    tlmModule       Platform;
    tlmDecoder      bus1;
    tlmDecoder      bus2;
    tlmDecoder      bus_sh;
    tlmRam          ram10, ram11; 
    tlmRam          ram20, ram21;
    tlmRam          ram3, ram4;
    riscv_RV32I           cpu1;
    riscv_RV32I           cpu2;
    DMA*            dma1; // note it's a pointer

  private:
    params paramsForcpu1() {
        params p;
        p.set("defaultsemihost", true);
        return p;
    }

    params paramsForcpu2() {
        params p;
        p.set("defaultsemihost", true);
        return p;
    }

    params paramsForPlatform() {
        params p;
        p.set("verbose", true);
        return p;
    }

}; /* BareMetal */

BareMetal:: BareMetal (sc_module_name name)
    : sc_module (name)
    , Platform ("", paramsForPlatform())
    , bus1 ( Platform, "bus1", 2, 3)
    , bus2 ( Platform, "bus2", 2, 3)
    , bus_sh( Platform, "bus_sh", 3, 3)
    , ram10 ( Platform, "ram10", 0xfffff)
    , ram11 ( Platform, "ram11", 0xfffff)
    , ram20 ( Platform, "ram20", 0xfffff)
    , ram21 ( Platform, "ram21", 0xfffff)
    , ram3 ( Platform, "ram3", 0xfffff)
    , ram4 ( Platform, "ram4", 0xfffff)
    , cpu1 ( Platform, "cpu1",  paramsForcpu1())
    , cpu2 ( Platform, "cpu2",  paramsForcpu2())
{

    // bus1 connection
    bus1.connect(cpu1.INSTRUCTION);
    bus1.connect(cpu1.DATA);
    bus1.connect(ram10.sp1, 0x0, 0xfffff);
    bus1.connect(ram11.sp1, 0xfff00000, 0xffffffff);
    bus1.connect(bus_sh, 0x100000, 0x4fffff);

    // bus2 connection
    bus2.connect(cpu2.INSTRUCTION);
    bus2.connect(cpu2.DATA);
    bus2.connect(ram20.sp1, 0x0, 0xfffff);
    bus2.connect(ram21.sp1, 0xfff00000, 0xffffffff);
    bus2.connect(bus_sh, 0x100000, 0x4fffff);

    // global bus connection
    bus_sh.connect(ram3.sp1,0x200000, 0x2fffff);
    bus_sh.connect(ram4.sp1,0x300000, 0x3fffff);
    
    // dma connection
    dma1 = new DMA("dma1");
    dma1->clk(clk);
    dma1->rst(reset);

    dma1->intr1.bind(cpu1.MExternalInterrupt); // for interrupt handler
    dma1->intr2.bind(cpu2.MExternalInterrupt);

    dma1->M_socket(*bus_sh.nextTargetSocket());
    bus_sh.nextInitiatorSocket(0x100000, 0x10001F)->bind(dma1->S_socket);
}

int sc_main (int argc, char *argv[]) {

    sc_clock clk("clk", 10);
    sc_signal<bool> reset;

    // start the CpuManager session
    session s;

    // create a standard command parser and parse the command line
    parser  p(argc, (const char**) argv);

    // create an instance of the platform
    BareMetal top ("top");
    top.clk(clk);
    top.reset(reset);
    reset.write(1);

    // start SystemC
    sc_start();
    return 0;
}

