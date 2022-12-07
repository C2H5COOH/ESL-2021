#include "systemc.h"
#include "dma.h"
#include <iostream>

#define CYCLE 5
#define DMA_BASE 0x00001000
#define SRC_ADDR 0x00000010
#define TGT_ADDR 0x00000010
#define TEST_SIZE 128

using namespace std;

typedef struct{
    uint32_t src;
    uint32_t tgt;
    uint32_t size;
    uint32_t start;
} ctrl;

typedef struct{
    sc_signal<uint32_t> data;
    sc_signal<uint32_t> addr;
    sc_signal<bool>     rw;
} DMA_S_port;

void ctrl_write(ctrl ctrl_local, DMA_S_port *dma_s){
    dma_s->addr.write(DMA_BASE);
    dma_s->data.write(ctrl_local.src);
    dma_s->rw.write(true);
    sc_start(CYCLE, SC_NS);

    dma_s->addr.write(DMA_BASE+0x4);
    dma_s->data.write(ctrl_local.tgt);
    dma_s->rw.write(true);
    sc_start(CYCLE, SC_NS);

    dma_s->addr.write(DMA_BASE+0x8);
    dma_s->data.write(ctrl_local.size);
    dma_s->rw.write(true);
    sc_start(CYCLE, SC_NS);

    dma_s->addr.write(DMA_BASE+0xc);
    dma_s->data.write(ctrl_local.start);
    dma_s->rw.write(true);
    sc_start(CYCLE, SC_NS);

    dma_s->rw.write(false);
}

int sc_main(int argc, char* argv[]){

    int source[128];
    int target[128];

    sc_clock clk("clk", CYCLE, SC_NS);
    sc_signal<bool> reset;
    sc_signal<bool> interrupt;

    //Master port
    sc_signal<int> DMA_M_data_in;
    sc_signal<int> DMA_M_data_out;
    sc_signal<uint32_t> DMA_M_addr;
    sc_signal<bool>     DMA_M_rw;

    //Slave port
    DMA_S_port *dma_s = new DMA_S_port;

    DMA *dma = new DMA("dma0");
    dma->clk(clk);
    dma->rst(reset);
    dma->intr(interrupt);

    dma->M_data_in(DMA_M_data_in);
    dma->M_data_out(DMA_M_data_out);
    dma->M_addr(DMA_M_addr);
    dma->M_rw(DMA_M_rw);

    // dma->S_data(DMA_S_data);
    // dma->S_addr(DMA_S_addr);
    // dma->S_rw(DMA_S_rw);
    dma->S_data(dma_s->data);
    dma->S_addr(dma_s->addr);
    dma->S_rw(dma_s->rw);


    sc_trace_file *tf = sc_create_vcd_trace_file("RESULT");;
    sc_trace(tf, clk, "clk");
    sc_trace(tf, reset, "reset");
    sc_trace(tf, interrupt, "interrupt");

    sc_trace(tf, DMA_M_data_in, "DMA_M_data_in");
    sc_trace(tf, DMA_M_data_out, "DMA_M_data_out");
    sc_trace(tf, DMA_M_addr, "DMA_M_addr");
    sc_trace(tf, DMA_M_rw, "DMA_M_rw");

    sc_trace(tf, dma_s->addr, "DMA_S_addr");
    sc_trace(tf, dma_s->data, "DMA_S_data");
    sc_trace(tf, dma_s->rw, "DMA_S_rw");
    
    sc_trace(tf, dma->SOURCE, "SOURCE");
    sc_trace(tf, dma->TARGET, "TARGET");
    sc_trace(tf, dma->SIZE, "SIZE");
    sc_trace(tf, dma->START_CLEAR, "START_CLEAR");

    sc_trace(tf, dma->state, "DMA_state");

    for(int i=0; i<128; i++){
        source[i] = i;
        target[i] = i+i;
    }

    //start to test

    //reset
    reset.write(0);
    sc_start(CYCLE, SC_NS);
    reset.write(1);

    //First test

    //write to ctrl register
    ctrl test_ctrl = {
        .src = SRC_ADDR,
        .tgt = TGT_ADDR,
        .size = TEST_SIZE,
        .start = 1
    };
    ctrl_write(test_ctrl, dma_s);

    //receive DMA signal
    int count = 0;
    while(!interrupt.read()){
        if(DMA_M_rw == 0){
            DMA_M_data_in = source[DMA_M_addr >> 2];
            // cout << "DMA read from address: " << DMA_M_addr << ", receives data : " << source[DMA_M_addr >> 2] << endl;
        }
        else if(DMA_M_rw == 1){
            target[DMA_M_addr >> 2] = DMA_M_data_out;
            // cout << "DMA writes to address: " << DMA_M_addr << ", receives data : " << DMA_M_data_out << endl;
        }

        if(count > 10000){
            SC_REPORT_FATAL("Interrupt", "DMA fail to assert interrupt");
            sc_close_vcd_trace_file(tf);
        }
        else count++;
        sc_start(CYCLE, SC_NS);
    }

    //clear start signal
    dma_s->addr.write(DMA_BASE+0xc);
    dma_s->data.write(false);//clear start signal
    dma_s->rw.write(true);
    sc_start(CYCLE*5, SC_NS);

    // check interrupt
    if(interrupt.read()){
        SC_REPORT_FATAL("Interrupt", "DMA fail to pull down interrupt");
        sc_close_vcd_trace_file(tf);
    }

    // check data
    uint32_t check_src_addr = SRC_ADDR >> 2;
    uint32_t check_tgt_addr = TGT_ADDR >> 2;
    for(int i=0; i<(TEST_SIZE >> 2); i++){
        if( target[check_tgt_addr+i] != (source[check_src_addr+i]) ) {
            cout << "Error : Data at " <<  check_tgt_addr+i << ": " << target[check_tgt_addr+i] 
                << ", expects :" << source[check_src_addr+i] << endl;
            SC_REPORT_FATAL("Data", "Target didn't receive correct data");
        }
    }
    cout << "Pass : Data movement is correct." << endl;

    sc_close_vcd_trace_file(tf);
    return 0;
}