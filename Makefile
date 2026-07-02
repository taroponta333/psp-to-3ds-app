TARGET = psp_receiver
OBJS = main.o

# PSPの実機で動かす「EBOOT.PBP」を作るための魔法の1行を追加！
BUILD_PRX = 1
PSP_EBOOT_TITLE = Turbo Receiver

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

# PSPのネットワーク機能を使うためのライブラリ
LIBS = -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet

include $(PSPSDK)/lib/build.mak
