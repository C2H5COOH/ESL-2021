#include "systemc.h"
#include "dma.h"
void DMA::thread_process(){
    //reset
    state = 0;
    for(int i=0; i<4; i++)
        ctrl[i] = 0;

    while(1){
        wait();
        // std::cout << "current state=" << state << endl;
        //write to control register
        //Host writes to DMA
        if(S_rw){
            switch(S_addr-base){
                case 0x0: 
                    SOURCE = S_data;
                    break;

                case 0x4: 
                    TARGET = S_data;
                    break;

                case 0x8: 
                    SIZE = S_data;
                    break;

                case 0xc: 
                    START_CLEAR = S_data;
                    break;

                default:
                    SC_REPORT_FATAL("Master", "Invalid address"); 
                    break;
            }
        }
        else{
            //Nothing to be done
        }

        switch(state){
            case 0:
                M_addr.write(SOURCE);
                M_rw.write(0);

                if(START_CLEAR){
                    SOURCE = SOURCE + 4;
                    state = 2;
                }
                break;

            case 1:
                M_addr.write(SOURCE);
                M_rw.write(0);

                SOURCE = SOURCE + 4;
                state = 2;
                break;

            case 2:
                M_addr.write(TARGET);
                M_rw.write(1);
                M_data_out.write(M_data_in);
                
                TARGET = TARGET +  4;
                state = (SIZE == 0)? 3 : 1;
                SIZE = SIZE - 4;
                break;

            case 3:
                M_rw.write(0);
                if(START_CLEAR == 0){
                    state = 0;
                    intr.write(0);
                }
                else{
                    state = 3;
                    intr.write(1);
                }
                break;

            default:
                break;
        }
            
    }
}