#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"

#include "memory.h"

void Memory::b_transport( tlm::tlm_generic_payload& trans, sc_time& delay ){
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address() / 4;
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();

    if (adr >= sc_dt::uint64(MEM_SIZE) || len != 4 )
      SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction");

    if ( cmd == tlm::TLM_READ_COMMAND )
      memcpy(ptr, &mem[adr], len);
    else if ( cmd == tlm::TLM_WRITE_COMMAND )
      memcpy(&mem[adr], ptr, len);

    trans.set_response_status( tlm::TLM_OK_RESPONSE );
}