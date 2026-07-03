#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

PSP_MODULE_INFO("HelloPSP", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

/* ====================================================================
   ★ここがポイント：HOMEボタン用の「割り込み処理（コールバック）」の設定
   ==================================================================== */

// HOMEボタンが押されたときに、PSPシステムから自動で呼び出される関数
int exit_callback(int arg1, int arg2, void *common) {
    // ゲームを安全に終了させる指令
    sceKernelExitGame();
    return 0;
}

// コールバックを監視するためのスレッド（裏方で動く仕組み）
int CallbackThread(SceSize args, void *argp) {
    // 終了用のコールバックを登録
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);

    // アプリが動いている間、ずっと裏でHOMEボタンを見張り続ける
    sceKernelSleepThreadCB();
    return 0;
}

// このコールバック機能を起動する関数
int SetupCallbacks(void) {
    int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) {
        sceKernelStartThread(thid, 0, 0);
    }
    return thid;
}

/* ==================================================================== */

int main(int argc, char *argv[]) {
    // 画面の初期化
    pspDebugScreenInit();
    
    // ★ここでHOMEボタンの割り込み機能をセットオン！
    SetupCallbacks();
    
    // ボタン入力の設定
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    // タイトル表示（ハッカーグリーン）
    pspDebugScreenSetTextColor(0xFF00FF00); 
    pspDebugScreenPrintf("===================================\n");
    pspDebugScreenPrintf("      Hello PSP by minax333       \n");
    pspDebugScreenPrintf("===================================\n\n");

    // 説明文（白）
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenPrintf("Press any button to test entry...\n");
    pspDebugScreenPrintf("Press HOME button to open Exit Menu.\n\n");

    SceCtrlData pad;

    // メインループ
    while (1) {
        sceCtrlReadBufferPositive(&pad, 1);

        pspDebugScreenSetXY(0, 6);
        pspDebugScreenPrintf("Current Button State: ");

        // 押されているボタンを判定して表示
        if (pad.Buttons & PSP_CTRL_CIRCLE)   pspDebugScreenPrintf("[ O ] ");
        if (pad.Buttons & PSP_CTRL_CROSS)    pspDebugScreenPrintf("[ X ] ");
        if (pad.Buttons & PSP_CTRL_TRIANGLE) pspDebugScreenPrintf("[ TRIANGLE ] ");
        if (pad.Buttons & PSP_CTRL_SQUARE)   pspDebugScreenPrintf("[ SQUARE ] ");
        
        if (pad.Buttons & PSP_CTRL_UP)       pspDebugScreenPrintf("[ UP ] ");
        if (pad.Buttons & PSP_CTRL_DOWN)     pspDebugScreenPrintf("[ DOWN ] ");
        if (pad.Buttons & PSP_CTRL)     pspDebugScreenPrintf("[ LEFT ] ");
        if (pad.Buttons & PSP_CTRL_RIGHT)    pspDebugScreenPrintf("[ RIGHT ] ");
        
        if (pad.Buttons & PSP_CTRL_LTRIGGER) pspDebugScreenPrintf("[ L ] ");
        if (pad.Buttons & PSP_CTRL_RTRIGGER) pspDebugScreenPrintf("[ R ] ");
        
        if (pad.Buttons & PSP_CTRL_START)    pspDebugScreenPrintf("[ START ] ");
        if (pad.Buttons & PSP_CTRL_SELECT)   pspDebugScreenPrintf("[ SELECT ] ");

        if (pad.Buttons == 0) {
            pspDebugScreenPrintf("[ NONE ]                                ");
        } else {
            pspDebugScreenPrintf("        ");
        }

        sceKernelDelayThread(10000); 
    }

    return 0;
}
