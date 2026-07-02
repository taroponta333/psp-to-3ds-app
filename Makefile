TARGET = psp_receiver
OBJS = main.o

# PSPのネットワーク機能（アドホック等）を使うためのライブラリをリンク
LIBS = -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

# EBOOT.PBP（PSPで動く最終形）を自動生成する設定
BUILD_PRX = 1
PSP_EBOOT_TITLE = PSP Wireless Receiver
# PSP_EBOOT_ICON = icon0.png  # もしアイコン画像があればコメント解除

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
