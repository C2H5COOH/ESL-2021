#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"

#ifndef CPU_H
#define CPU_H

#define SC_INCLUDE_DYNAMIC_PROCESSES

using namespace sc_core;
using namespace sc_dt;
using namespace std;

SC_MODULE(CPU){
    sc_in<bool> clk, rst;
    sc_in<bool> intr;

    sc_in<bool> start;
    sc_out<bool> clear;

    tlm_utils::simple_initiator_socket<CPU> socket;

    void thread_process();

    SC_CTOR(CPU)
    : socket("Socket_CPU")
    {
        SC_CTHREAD(thread_process, clk.pos());
        reset_signal_is(rst, false);
    }

    uint32_t data_write[4];
};

#endif