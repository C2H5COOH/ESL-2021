#include "systemc.h"
#ifndef DMA_H
#define DMA_H

#define SOURCE ctrl[0]
#define TARGET ctrl[1]
#define SIZE ctrl[2]
#define START_CLEAR ctrl[3]

SC_MODULE(DMA){
    sc_in<bool> clk, rst;
    sc_out<bool> intr;

    //Master port
    sc_out<int>    M_data_in;
    sc_out<int>    M_data_out;
    sc_out<uint32_t>    M_addr;
    sc_out<bool>        M_rw;

    //Slave port
    sc_in<uint32_t> S_data;
    sc_in<uint32_t> S_addr;
    sc_in<bool>     S_rw;

    void thread_process();

    SC_CTOR(DMA){
        SC_CTHREAD(thread_process, clk.pos());
        reset_signal_is(rst, false);
    }

    //control register
    sc_signal<uint32_t> ctrl[4];
    int state;
    const uint32_t base = 0x00001000;
};

#endif
