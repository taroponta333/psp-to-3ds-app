# 出来上がる3DSアプリの名前
TARGET		:= 3ds_to_psp_sender
# 開発者名やアプリの説明
TITLE		:= 3DS to PSP Sender
AUTHOR		:= Homebrew Dev
DESCRIPTION	:= Send PSP homebrew files via 3DS raw Wi-Fi.

# 拡張ツール群の場所（devkitPro環境の標準パス）
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/3ds_rules

# コンパイルオプション
CFLAGS	:= -g -Wall -O2 -mword-relocations \
		   -fomit-frame-pointer -ffunction-sections \
		   $(ARCH)

CFLAGS	+= -DARM11 -I$(3DSEXEC_INC)

LIBS	:= -lctru -lm

# ターゲット指定
.PHONY: all clean

all: $(TARGET).3dsx

$(TARGET).3dsx: $(TARGET).elf
$(TARGET).elf: main.o

# クリーンアップ（生成された一時ファイルを消すコマンド）
clean:
	rm -f *.o *.elf *.3dsx
