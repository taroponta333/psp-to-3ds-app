#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h> // ボタン入力を扱うためのライブラリ

// PSPの基本設定（おまじない）
PSP_MODULE_INFO("HelloPSP", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int main(int argc, char *argv[]) {
    // 1. 画面デバッグ表示の初期化
    pspDebugScreenInit();
    
    // 2. ボタン入力の設定（標準的な読み取りモードに設定）
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    // 画面の文字色を緑（ハッカーっぽく）にして、タイトルを表示
    pspDebugScreenSetTextColor(0xFF00FF00); 
    pspDebugScreenPrintf("===================================\n");
    pspDebugScreenPrintf("      Hello PSP by minax333       \n");
    pspDebugScreenPrintf("===================================\n\n");

    // 文字色を白に戻す
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenPrintf("Press any button to test entry...\n");
    pspDebugScreenPrintf("Press HOME button to exit app.\n\n");

    // 入力データを格納する構造体
    SceCtrlData pad;

    // メインループ（ボタンの状態を監視し続ける）
    while (1) {
        // 現在のボタンの状態を読み取る
        sceCtrlReadBufferPositive(&pad, 1);

        // 画面の4行目、0列目にカーソルを固定して上書き描画（画面のチラつき防止）
        pspDebugScreenSetXY(0, 6);
        pspDebugScreenPrintf("Current Button State: ");

        // 押されているボタンを判定して表示
        if (pad.Buttons & PSP_CTRL_CIRCLE)   pspDebugScreenPrintf("[ CIRCLE ] ");
        if (pad.Buttons & PSP_CTRL_CROSS)    pspDebugScreenPrintf("[ CROSS ] ");
        if (pad.Buttons & PSP_CTRL_TRIANGLE) pspDebugScreenPrintf("[ TRIANGLE ] ");
        if (pad.Buttons & PSP_CTRL_SQUARE)   pspDebugScreenPrintf("[ SQUARE ] ");
        
        if (pad.Buttons & PSP_CTRL_UP)       pspDebugScreenPrintf("[ UP ] ");
        if (pad.Buttons & PSP_CTRL_DOWN)     pspDebugScreenPrintf("[ DOWN ] ");
        if (pad.Buttons & PSP_CTRL_LEFT)     pspDebugScreenPrintf("[ LEFT ] ");
        if (pad.Buttons & PSP_CTRL_RIGHT)    pspDebugScreenPrintf("[ RIGHT ] ");
        
        if (pad.Buttons & PSP_CTRL_LTRIGGER) pspDebugScreenPrintf("[ L ] ");
        if (pad.Buttons & PSP_CTRL_RTRIGGER) pspDebugScreenPrintf("[ R ] ");
        
        if (pad.Buttons & PSP_CTRL_START)    pspDebugScreenPrintf("[ START ] ");
        if (pad.Buttons & PSP_CTRL_SELECT)   pspDebugScreenPrintf("[ SELECT ] ");

        // 何も押されていない時は空白で埋める
        if (pad.Buttons == 0) {
            pspDebugScreenPrintf("[ NONE ]                                ");
        } else {
            // 前の文字が残らないようにスペースでクリア
            pspDebugScreenPrintf("        ");
        }

        // CPUを休ませるためのほんの少しのウェイト（フリーズ防止）
        sceKernelDelayThread(10000); 
    }

    return 0;
}
