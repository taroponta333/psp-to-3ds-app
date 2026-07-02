PSPSDK := $(shell psp-config --pspsdk-path)

TARGET = psp_receiver
OBJS = main.o

# PRXビルドの時は、これを書かないとEBOOT.PBPの自動生成までたどり着きません！
BUILD_PRX = 1
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Turbo Receiver

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet

include $(PSPSDK)/lib/build.mak
