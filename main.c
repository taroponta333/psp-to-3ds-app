#include <pspkernel.h>
#include <pspctrl.h>
#include <stdio.h>

// プラグインとして登録するためのモジュール情報（カーネルモード特権を指定）
PSP_MODULE_INFO("ButtonLogger", 0x1000, 1, 1);

// ログファイルにテキストを書き出す関数
void write_log(const char *text) {
    // メモリースティックのルート（一番上の階層）に「btn_log.txt」を追記モードで開く
    FILE *f = fopen("ms0:/btn_log.txt", "a");
    if (f != NULL) {
        fprintf(f, "%s\n", text);
        fclose(f);
    }
}

// バックグラウンドで常にボタンを監視し続ける「裏方スレッド」
int LoggerThread(SceSize args, void *argp) {
    SceCtrlData pad;
    unsigned int last_buttons = 0;

    // プラグインが読み込まれたら、まずは起動ログを記録
    write_log("--- Logger Plugin Started ---");

    // ボタン入力をデジタルモードに設定
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    while (1) {
        // 現在のボタンの状態を読み取る（カーネルモード用の関数）
        sceCtrlReadBufferPositive(&pad, 1);

        // 「前回とボタンの状態が変わった瞬間」だけを検知してログに書く
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

            // 何も押されていない状態に戻ったら記録
            if (pad.Buttons == 0) {
                write_log("[NONE]");
            }

            // 今のボタンの状態を記憶して、次回と比較できるようにする
            last_buttons = pad.Buttons;
        }

        // 10ミリ秒（0.01秒）待つ。これがないとPSPのCPUが100%になってフリーズします
        sceKernelDelayThread(10000);
    }

    return 0;
}

// プラグインが起動した時にPSPシステムから最初に呼ばれる関数（入り口）
int module_start(SceSize args, void *argp) {
    // ログ記録用のスレッド（裏方）をカーネル特権で作成して起動する
    int thid = sceKernelCreateThread("logger_thread", LoggerThread, 0x11, 0xFA0, 0, NULL);
    if (thid >= 0) {
        sceKernelStartThread(thid, args, argp);
    }
    return 0;
}

// プラグインが終了（無効化）された時に呼ばれる関数（出口）
int module_stop(void) {
    write_log("--- Logger Plugin Stopped ---");
    return 0;
}
