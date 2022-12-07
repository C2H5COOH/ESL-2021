#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#ifndef DMA_H
#define DMA_H

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

using namespace sc_core;
using namespace sc_dt;
using namespace std;

#define SOURCE1 ctrl[0] - BUS_BASE
#define TARGET1 ctrl[1] - BUS_BASE
#define SIZE1 ctrl[2]
#define START_CLEAR1 ctrl[3]

#define SOURCE2 ctrl[4] - BUS_BASE
#define TARGET2 ctrl[5] - BUS_BASE
#define SIZE2 ctrl[6]
#define START_CLEAR2 ctrl[7]

#define RISCV1 false
#define RISCV2 true

#define DMA_INIT    1
#define DMA_RDSRC   2
#define DMA_WRTGT   3
#define DMA_DONE    4

#define BUS_BASE    0x100000

SC_MODULE(DMA){
    sc_in<bool> clk, rst;
    tlm::tlm_analysis_port<unsigned int> intr1, intr2;

    tlm_utils::simple_initiator_socket<DMA> M_socket;
    tlm_utils::simple_target_socket<DMA> S_socket;

    void thread_process();
    virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay );

    SC_CTOR(DMA)
    : M_socket("Master")
    , S_socket("Slave")
    {
        S_socket.register_b_transport(this, &DMA::b_transport);

        SC_CTHREAD(thread_process, clk.pos());
        reset_signal_is(rst, false);
    }

    uint32_t base;
    //control register
    uint32_t ctrl[8];

    // internal info
    bool occupied;
    bool serve_id;
    int state;
    unsigned int buffer[256];
    // unsigned char buffer[1024];
};

#endif
