#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include <iostream>

#ifndef MEMORY_H
#define MEMORY_H

#define SC_INCLUDE_DYNAMIC_PROCESSES
#define MEM_SIZE 4096

using namespace sc_core;
using namespace sc_dt;
using namespace std;

SC_MODULE(Memory){
    int mem[MEM_SIZE/4];
    tlm_utils::simple_target_socket<Memory> socket;
    virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay );

    SC_CTOR(Memory): socket("socket"){
        socket.register_b_transport(this, &Memory::b_transport);
        for(int i=0; i<MEM_SIZE/4; i++){
            mem[i] = i+i;
        }
    }
};
#endif