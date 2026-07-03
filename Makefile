PSPSDK := $(shell psp-config --pspsdk-path)

# 出力されるプラグインの名前（ボタンロガー.prx）
TARGET = btn_logger
OBJS = main.o

# プラグイン（PRX）としてビルドし、最終ターゲットに指定する
BUILD_PRX = 1
EXTRA_TARGETS = $(TARGET).prx

# カーネルモード（FWレベル）で動作させるための特権フラグ
USE_KERNEL_LIBS = 1

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = 

include $(PSPSDK)/lib/build.mak
