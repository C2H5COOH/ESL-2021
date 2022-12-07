#include "dma.h"
#include "systemc.h"
#include <stdio.h>

void DMA::b_transport( tlm::tlm_generic_payload& trans, sc_time& delay ){
    tlm::tlm_command cmd = trans.get_command();
    unsigned int     adr = (trans.get_address() - base) / 4;
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();

    bool request_id = (adr > 3)? RISCV2 : RISCV1;
    while(occupied && request_id != serve_id){
        std::cout << "request_id=" << request_id << endl;
        std::cout << "serve_id=" << serve_id << endl;
        wait();// wait for DMA release
    }
    occupied = true;
    serve_id = request_id;

    if(adr > 7) 
        SC_REPORT_FATAL("Address", "Invalid address");
    else if(len != 4)
        SC_REPORT_FATAL("Length", "Un-align operation");

    if(cmd == tlm::TLM_WRITE_COMMAND){
        memcpy(&ctrl[adr], ptr, len);
        std::cout << "adr=" << adr << ", ";
        std::cout << hex << ctrl[adr] << endl;
    }
    else{
        //Nothing to be done
    }
    trans.set_response_status( tlm::TLM_OK_RESPONSE );
    
    delay = sc_time(10, SC_NS);
    wait(delay);
}
void DMA::thread_process(){
    //reset
    sc_time delay = sc_time(0, SC_NS);
    tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload;

    trans->set_data_ptr(reinterpret_cast<unsigned char*>(&buffer) );
    // trans->set_data_ptr(buffer);
    trans->set_data_length(4);
    trans->set_streaming_width(4);
    trans->set_byte_enable_ptr(0);
    trans->set_dmi_allowed(false);
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    base = 0;
    occupied = false;
    serve_id = RISCV1;
    state = DMA_INIT;

    for(int i=0; i<256; i++){
        buffer[i] = 0;
    }

    for(int i=0; i<8; i++){
        ctrl[i] = 0;
    }

    while(1){
        wait();
        // std::cout << "state = " << state << endl;
        switch(state){
            case DMA_INIT:
                if(START_CLEAR1 || START_CLEAR2){
                    trans->set_command(tlm::TLM_READ_COMMAND);
                    if(serve_id == RISCV1){  
                        trans->set_address(SOURCE1);
                        trans->set_data_length(SIZE1);
                        trans->set_streaming_width(SIZE1);
                    }
                    else{
                        trans->set_address(SOURCE2);
                        trans->set_data_length(SIZE2);
                        trans->set_streaming_width(SIZE2);
                    }
                    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
                    delay = sc_time(0, SC_NS);
                    M_socket->b_transport( *trans, delay );

                    // std::cout << "ctrl[0]=" << hex<< SOURCE2 << endl;
                    // std::cout << "ctrl[1]=" << hex<< TARGET2 << endl;
                    // std::cout << "ctrl[2]=" << SIZE2 << endl;
                    // std::cout << "ctrl[3]=" << START_CLEAR2 << endl;
                    if(trans->get_response_status() != tlm::TLM_OK_RESPONSE){
                        SC_REPORT_FATAL("DMA_READ", "Unknown transaction fail.");
                    }

                    std::cout << "serve_id=" << serve_id << endl;

                    state = DMA_WRTGT;                 
                }

                break;
            
            // read data from source
            /*case DMA_RDSRC:
                trans->set_command(tlm::TLM_READ_COMMAND);
                if(serve_id == RISCV1)  
                    trans->set_address(SOURCE1);
                else
                    trans->set_address(SOURCE2);

                trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
                delay = sc_time(10, SC_NS);
                M_socket->b_transport( *trans, delay );

                if(serve_id == RISCV1)  
                    SOURCE1 = SOURCE1 + 4;
                else
                    SOURCE2 = SOURCE2 + 4;

                state = DMA_WRTGT;

                break;*/

            // write data to target
            case DMA_WRTGT:
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                    if(serve_id == RISCV1){  
                        trans->set_address(TARGET1);
                        trans->set_data_length(SIZE1);
                        trans->set_streaming_width(SIZE1);
                    }
                    else{
                        trans->set_address(TARGET2);
                        trans->set_data_length(SIZE2);
                        trans->set_streaming_width(SIZE2);
                    }
                    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
                    delay = sc_time(0, SC_NS);
                    M_socket->b_transport( *trans, delay );

                    if(trans->get_response_status() != tlm::TLM_OK_RESPONSE){
                        std::cout << "ctrl[0]=" << hex << SOURCE1 << endl;
                        std::cout << "ctrl[1]=" << hex<< TARGET1 << endl;
                        std::cout << "ctrl[2]=" << SIZE1 << endl;
                        std::cout << "ctrl[3]=" << START_CLEAR1 << endl;
                        SC_REPORT_FATAL("DMA_WRITE", "Unknown transaction fail.");
                    }

                    state = DMA_DONE;
                break;

            case DMA_DONE:
                if(serve_id == RISCV1){
                    if(START_CLEAR1 == 0){
                        state = DMA_INIT;
                        intr1.write(0);
                        occupied = false;
                    }
                    else{
                        state = DMA_DONE;
                        intr1.write(1);
                    }
                }
                else{
                    if(START_CLEAR2 == 0){
                        state = DMA_INIT;
                        intr2.write(0);
                        occupied = false;
                    }
                    else{
                        state = DMA_DONE;
                        intr2.write(1);
                    }
                }
                
                break;

            default : 
                break;
        }   
    }
}