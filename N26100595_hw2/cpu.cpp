#include "systemc.h"
#include "config.h"
#include "cpu.h"
#include <iostream>

using namespace std;

void CPU::thread_process(){
    sc_time delay = sc_time(10, SC_NS);
    tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload;;
    while(1){
        wait();
        if(start.read()){

            data_write[0] = SRC_ADDR;
            data_write[1] = TGT_ADDR;
            data_write[2] = TEST_SIZE;
            data_write[3] = 1;

            cout << "CPU sends data to DMA.\n";

            for(int i=0; i<4; i++){
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                trans->set_address(DMA_BASE + (i*4));
                trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data_write[i]) );
                trans->set_data_length(4);
                trans->set_streaming_width(4);
                trans->set_byte_enable_ptr(0);
                trans->set_dmi_allowed(false);
                trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

                socket->b_transport(*trans, delay);
                // wait(delay);
            }

            cout << "CPU waits for interrupt.\n";
            SC_WAIT_UNTIL(intr.read());
            cout << "CPU receives interrupt.\n";

            data_write[3] = 0;
            trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data_write[3]) );
            trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

            socket->b_transport(*trans, delay);
            // wait(delay);
            
            cout << "CPU clears interrupt.\n";
            SC_WAIT_UNTIL(!intr.read());
            cout << "CPU clears interrupt done.\n";
            clear.write(1);

            SC_WAIT_UNTIL(!start.read());
            cout << "CPU done.\n";
        }
    }
}