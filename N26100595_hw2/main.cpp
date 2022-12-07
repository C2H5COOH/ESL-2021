#include "systemc.h"
#include <iostream>

#include "cpu.h"
#include "dma.h"
#include "memory.h"
#include "config.h"

using namespace std;

int sc_main(int argc, char* argv[]){

    sc_clock clk("clk", CYCLE, SC_NS);
    sc_signal<bool> reset;
    sc_signal<bool> interrupt;

    sc_signal<bool> start;
    sc_signal<bool> clear;


    DMA *dma = new DMA("dma0");
    dma->clk(clk);
    dma->rst(reset);
    dma->intr(interrupt);

    Memory *memory = new Memory("mem");
    dma->M_socket.bind(memory->socket);

    CPU *cpu = new CPU("cpu");
    cpu->clk(clk);
    cpu->rst(reset);
    cpu->intr(interrupt);
    cpu->start(start);
    cpu->clear(clear);
    cpu->socket.bind(dma->S_socket);

    sc_trace_file *tf = sc_create_vcd_trace_file("RESULT");;
    sc_trace(tf, clk, "clk");
    sc_trace(tf, reset, "reset");
    sc_trace(tf, interrupt, "interrupt");
    sc_trace(tf, start, "start");
    sc_trace(tf, clear, "clear");
    
    sc_trace(tf, dma->SOURCE, "SOURCE");
    sc_trace(tf, dma->TARGET, "TARGET");
    sc_trace(tf, dma->SIZE, "SIZE");
    sc_trace(tf, dma->START_CLEAR, "START_CLEAR");

    sc_trace(tf, dma->state, "DMA_state");

    //start to test
    //reset
    reset.write(0);
    sc_start(CYCLE, SC_NS);
    reset.write(1);

    //First test
    start.write(1);
    while(!clear.read()){
        sc_start(CYCLE, SC_NS);
    }
    start.write(0);

    // check data
    uint32_t check_src_addr = SRC_ADDR >> 2;
    uint32_t check_tgt_addr = TGT_ADDR >> 2;
    for(int i=0; i<(TEST_SIZE >> 2); i++){
        if( memory->mem[check_src_addr+i] != memory->mem[check_tgt_addr+i] ) {
            cout << "Error : Data at " <<  check_tgt_addr+i << ": " << memory->mem[check_tgt_addr+i]
                << ", expects :" << memory->mem[check_src_addr+i] << endl;
            SC_REPORT_FATAL("Data", "Target didn't receive correct data");
        }
    }

    cout << "        ****************************               \n";
    cout << "        **                        **       ||__||  \n";
    cout << "        **  Congratulations !!    **      / O.O  | \n";
    cout << "        **                        **    /_____   | \n";
    cout << "        **  Simulation PASS!!     **   /^ ^ ^ \\  |\n";
    cout << "        **                        **  |^ ^ ^ ^ |w| \n";
    cout << "        ****************************   \\m___m__|_|\n";
    cout << "\n";

    sc_close_vcd_trace_file(tf);
    return 0;
}