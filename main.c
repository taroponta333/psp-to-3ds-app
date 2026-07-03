#include <pspkernel.h>
#include <pspctrl.h>

// カーネルモード用のヘッダー（文字列操作や基本的な出力用）
#include <string.h>

PSP_MODULE_INFO("ButtonLogger", 0x1000, 1, 1);

// 【修正！】カーネルモード専用のファイル書き込み関数
void write_log(const char *text) {
    // 追記モードの代わりに、オープン(O_WRONLY)、作成(O_CREAT)、追記(O_APPEND)のフラグを使う
    // パーミッションは 0777 を指定
    int fd = sceIoOpen("ms0:/btn_log.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd >= 0) {
        // 文字列の長さを計算して書き込む
        sceIoWrite(fd, text, strlen(text));
        // 改行コード（\n）を書き込む
        sceIoWrite(fd, "\n", 1);
        // ファイルを閉じる
        sceIoClose(fd);
    }
}

int LoggerThread(SceSize args, void *argp) {
    SceCtrlData pad;
    unsigned int last_buttons = 0;

    write_log("--- Logger Plugin Started ---");

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    while (1) {
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons != last_buttons) {
            if (pad.Buttons & PSP_CTRL_CIRCLE)   write_log("[CIRCLE] Pressed");
            if (pad.Buttons & PSP_CTRL_CROSS)    write_log("[CROSS] Pressed");
            if (pad.Buttons & PSP_CTRL_TRIANGLE) write_log("[TRIANGLE] Pressed");
            if (pad.Buttons & PSP_CTRL_SQUARE)   write_log("[SQUARE] Pressed");
            
            if (pad.Buttons & PSP_CTRL_UP)       write_log("[UP] Pressed");
            if (pad.Buttons & PSP_CTRL_DOWN)     write_log("[DOWN] Pressed");
            if (pad.Buttons & PSP_CTRL_LEFT)     write_log("[LEFT] Pressed");
            if (pad.Buttons & PSP_CTRL_RIGHT)    write_log("[RIGHT] Pressed");
            
            if (pad.Buttons & PSP_CTRL_LTRIGGER) write_log("[L] Pressed");
            if (pad.Buttons & PSP_CTRL_RTRIGGER) write_log("[R] Pressed");
            
            if (pad.Buttons & PSP_CTRL_START)    write_log("[START] Pressed");
            if (pad.Buttons & PSP_CTRL_SELECT)   write_log("[SELECT] Pressed");

            if (pad.Buttons == 0) {
                write_log("[NONE]");
            }

            last_buttons = pad.Buttons;
        }

        sceKernelDelayThread(10000);
    }

    return 0;
}

int module_start(SceSize args, void *argp) {
    int thid = sceKernelCreateThread("logger_thread", LoggerThread, 0x11, 0xFA0, 0, NULL);
    if (thid >= 0) {
        sceKernelStartThread(thid, args, argp);
    }
    return 0;
}

int module_stop(void) {
    write_log("--- Logger Plugin Stopped ---");
    return 0;
}
