# コンパイラに言われた通り、自動でパスを見つける呪文を一番上に追加！
PSPSDK := $(shell psp-config --pspsdk-path)

TARGET = psp_receiver
OBJS = main.o

BUILD_PRX = 1
PSP_EBOOT_TITLE = Turbo Receiver

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet

# 元の環境変数を使った書き方に戻す（これでbuild.makの内部も完璧に連動します）
include $(PSPSDK)/lib/build.mak
