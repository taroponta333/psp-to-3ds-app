TARGET = psp_receiver
OBJS = main.o

BUILD_PRX = 1
PSP_EBOOT_TITLE = Turbo Receiver

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet

# 最後の1行を、迷子にならないように絶対パスに書き換える！
include /usr/local/pspdev/psp/sdk/lib/build.mak
