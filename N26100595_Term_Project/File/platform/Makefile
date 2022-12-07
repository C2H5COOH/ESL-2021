IMPERAS_ARCH = Linux32
ifndef IMPERAS_HOME
  IMPERAS_ERROR := $(error "IMPERAS_HOME not defined, please setup Imperas/OVP environment")
endif
IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")

# Build locally without using a VLNV library
NOVLNV=1

# define the processor based upon the CrossCompiler used (default RISC-V 32-bit)
CROSS?=RISCV32
CFLAGS+=-D$(CROSS)
export IMPERAS_ARCH=Linux64

OPTLM=1
SRC=platform.cpp dma.cpp

include ${IMPERAS_HOME}/ImperasLib/buildutils/Makefile.TLM.platform
