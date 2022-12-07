ifndef IMPERAS_HOME
  IMPERAS_ERROR := $(error "IMPERAS_HOME not defined, please setup Imperas/OVP environment")
endif
IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")
include $(IMPERAS_HOME)/bin/Makefile.include

# APP
CROSS?=RISCV32
export IMPERAS_ARCH=Linux32
-include $(IMPERAS_HOME)/lib/$(IMPERAS_ARCH)/CrossCompiler/$(CROSS).makefile.include
ifeq ($($(CROSS)_CC),)
    IMPERAS_ERROR := $(error "Please install the $(CROSS) toolchain")
endif

OPTIMIZATION?=-g -O0

all: cpuTest1.$(CROSS).elf cpuTest2.$(CROSS).elf


%.$(CROSS).elf: %.$(CROSS).o trap.$(CROSS).o
	$(V) echo "Linking Application $@"
	$(V) $($(CROSS)_LINK) -o $@  $^ $(IMPERAS_LDFLAGS)

%.$(CROSS).o: %.c
	$(V) echo "Compiling Application $@"
	$(V) $($(CROSS)_CC) -c $(OPTIMIZATION) -o $@  $<
	
%.$(CROSS).o: %.S
	$(V) echo "Assembling Application $@"
	$(V) $($(CROSS)_AS) -o $@  $<

clean:
	$(V) - rm -f cpuTest1.$(CROSS).elf cpuTest2.$(CROSS).elf *.o 

