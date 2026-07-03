#include <pspkernel.h>
#include <pspctrl.h>
#include <string.h>

PSP_MODULE_INFO("ButtonLogger", 0x1000, 1, 1);

void write_log(const char *text) {
    int fd = sceIoOpen("ms0:/btn_log.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, text, strlen(text));
        sceIoWrite(fd, "\n", 1);
        sceIoClose(fd);
    }
}

int LoggerThread(SceSize args, void *argp) {
    SceCtrlData pad;
    unsigned int last_buttons = 0;

    write_log("--- Logger Plugin Started ---");

    // プラグインでは、他のアプリの設定を壊さないように
    // サイクル設定（sceCtrlSetSamplingCycle）などはあえて行わないのが安全です

    while (1) {
        // 【修正！】Read から Peek に変更し、後ろの引数を「1」から「1」のままPeek仕様にする
        // これで他のアプリの入力をブロック（横取り）しなくなります
        sceCtrlPeekBufferPositive(&pad, 1);

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

        // 【修正！】少しだけ休憩時間を増やして（0.05秒）、他のアプリにCPUを譲る
        sceKernelDelayThread(50000);
    }

    return 0;
}

int module_start(SceSize args, void *argp) {
    // スレッドの優先度（第3引数）を 0x11 から 0x18 くらいに少し下げて、ゲーム側の処理を優先させます
    int thid = sceKernelCreateThread("logger_thread", LoggerThread, 0x18, 0xFA0, 0, NULL);
    if (thid >= 0) {
        sceKernelStartThread(thid, args, argp);
    }
    return 0;
}

int module_stop(void) {
    write_log("--- Logger Plugin Stopped ---");
    return 0;
}
