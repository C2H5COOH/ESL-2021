#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#ifndef DMA_H
#define DMA_H

#define SC_INCLUDE_DYNAMIC_PROCESSES

using namespace sc_core;
using namespace sc_dt;
using namespace std;

#define SOURCE ctrl[0]
#define TARGET ctrl[1]
#define SIZE ctrl[2]
#define START_CLEAR ctrl[3]

SC_MODULE(DMA){
    sc_in<bool> clk, rst;
    sc_out<bool> intr;

    tlm_utils::simple_initiator_socket<DMA> M_socket;
    tlm_utils::simple_target_socket<DMA> S_socket;

    void thread_process();
    virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay );

    SC_CTOR(DMA)
    : M_socket("Master")
    , S_socket("Slave")
    {
        base = 0x00001000;
        S_socket.register_b_transport(this, &DMA::b_transport);

        SC_CTHREAD(thread_process, clk.pos());
        reset_signal_is(rst, false);
    }

    // const uint32_t base = 0x00001000;
    uint32_t base;
    //control register
    uint32_t ctrl[4];
    int state;
    int buffer;
};

#endif
