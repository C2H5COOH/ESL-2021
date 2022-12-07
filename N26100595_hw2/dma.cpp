#include "systemc.h"
#include "dma.h"

void DMA::b_transport( tlm::tlm_generic_payload& trans, sc_time& delay ){
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = (trans.get_address() - base) / 4;
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();

    if(adr > 3) 
        SC_REPORT_FATAL("Address", "Invalid address");
    else if(len != 4)
        SC_REPORT_FATAL("Length", "Un-align operation");

    if(cmd == tlm::TLM_WRITE_COMMAND){
        memcpy(&ctrl[adr], ptr, len);
    }
    else{
        //Nothing to be done
    }
    trans.set_response_status( tlm::TLM_OK_RESPONSE );
}
void DMA::thread_process(){
    //reset
    sc_time delay = sc_time(0, SC_NS);
    tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload;

    trans->set_data_ptr(reinterpret_cast<unsigned char*>(&buffer) );
    trans->set_data_length(4);
    trans->set_streaming_width(4);
    trans->set_byte_enable_ptr(0);
    trans->set_dmi_allowed(false);
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    state = 0;
    buffer = 0;
    for(int i=0; i<4; i++)
        ctrl[i] = 0;
    // wait();
    while(1){
        wait();

        switch(state){
            case 0:
                if(START_CLEAR){
                    trans->set_command(tlm::TLM_READ_COMMAND);
                    trans->set_address(SOURCE);
                    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

                    delay = sc_time(10, SC_NS);
                    M_socket->b_transport( *trans, delay );
                    SOURCE = SOURCE + 4;
                    state = 2;
                }
                break;
            
            // read data from source
            case 1:
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(SOURCE);
                trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

                delay = sc_time(10, SC_NS);
                M_socket->b_transport( *trans, delay );
                
                SOURCE = SOURCE + 4;
                state = 2;
                break;

            // write data to target
            case 2:
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                trans->set_address(TARGET);
                trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

                delay = sc_time(10, SC_NS);
                M_socket->b_transport( *trans, delay );
                
                TARGET = TARGET + 4;
                state = (SIZE == 0)? 3 : 1;
                SIZE = SIZE - 4;
                break;

            case 3:
                if(START_CLEAR == 0){
                    state = 0;
                    intr.write(0);
                }
                else{
                    state = 3;
                    intr.write(1);
                }
                break;

            default : 
                break;
        }   
    }
}