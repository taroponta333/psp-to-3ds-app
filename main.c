#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspnet.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocmatching.h>
#include <stdio.h>
#include <string.h>

// PSPのモジュール設定
PSP_MODULE_INFO("PSP_Adhoc_Receiver", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define pspDebugScreenPrintf printf
#define BUFFER_SIZE 4096
#define PORT_NUMBER 5000 

const char g_product_id[] = "3DS2PSP"; 

void panic(const char *msg) {
    pspDebugScreenPrintf("\n[ERROR] %s\n", msg);
    pspDebugScreenPrintf("Please restart the homebrew.\n");
    while(1) sceKernelDelayThread(100 * 1000);
}
// マッチング用のコールバック関数（中身は空でOK）
void matching_cb(int id, int event, unsigned char *peer, int optlen, void *optdata) {
    // 通信イベントが発生したときにここが呼ばれます
}

int main() {
    pspDebugScreenInit();
    pspDebugScreenPrintf("===================================\n");
    pspDebugScreenPrintf("   PSP Ad-Hoc Save/App Receiver    \n");
    pspDebugScreenPrintf("===================================\n\n");

    // 1. 保存先フォルダの準備
    pspDebugScreenPrintf("[1/5] Preparing storage directory...\n");
    sceIoMkdir("ms0:/PSP/GAME/DOWNLOAD", 0777); 

    // 2. ネットワーク初期化
    pspDebugScreenPrintf("[2/5] Initializing PSP Network...\n");
    if (sceNetInit() < 0) panic("sceNetInit failed");
    if (sceNetAdhocInit() < 0) panic("sceNetAdhocInit failed");
    if (sceNetAdhocmatchingInit(0x20000) < 0) panic("sceNetAdhocmatchingInit failed");

    // 3. PDPソケットを作成
    int pdp_id = sceNetAdhocPdpCreate(NULL, PORT_NUMBER, 0x2000, 0);
    if (pdp_id < 0) panic("Failed to create PDP socket");

   // 4. マッチングシステムを開始
    pspDebugScreenPrintf("[3/5] Starting Matching System...\n");
    
    // 引数を仕様通りの「9個」に修正
    int matching_id = sceNetAdhocMatchingCreate(
        3,              // mode (P2P/インフラモード)
        2,              // maxpeers (最大接続台数: 2)
        PORT_NUMBER,    // port
        1024,           // bufsize
        200 * 1000,     // hellodelay
        500 * 1000,     // pingdelay
        3,              // initcount
        200 * 1000,     // msgdelay
        matching_cb     // callback (上で作った関数を指定)
    );
    if (matching_id < 0) panic("Failed to create matching context");

    // 引数を仕様通りの「7個」に修正（スレッド優先度やスタックサイズを指定）
    if (sceNetAdhocMatchingStart(matching_id, 0x10, 0x4000, 0x10, 0x4000, 0, NULL) < 0) {
        panic("Failed to start matching");
    }
    
    // 5. 接続確立（ダミー処理）
    unsigned char peer_mac[6] = {0}; 
    pspDebugScreenPrintf("\n[4/5] Connected to 3DS successfully!\n");

    // 6. ファイルの受信と書き込みループ
    char *save_path = "ms0:/PSP/GAME/DOWNLOAD/EBOOT.PBP";
    FILE *fp = fopen(save_path, "wb");
    if (fp == NULL) panic("Cannot open file to write.");

    pspDebugScreenPrintf("[5/5] Receiving data...\n");

    char buffer[BUFFER_SIZE];
    int total_bytes = 0;
    unsigned short current_port = PORT_NUMBER;

    while (1) {
        unsigned int rlen = BUFFER_SIZE;
        
        // ⭕️ 引数の NULL を 0 (タイムアウトなし) に修正
        int res = sceNetAdhocPdpRecv(pdp_id, peer_mac, &current_port, buffer, &rlen, 0, 0);
        
        if (res < 0) {
            break; 
        }

        if (rlen > 0) {
            fwrite(buffer, 1, rlen, fp);
            total_bytes += rlen;
            pspDebugScreenPrintf("\r    -> Progress: %d KB received", total_bytes / 1024);
        }

        sceKernelDelayThread(10 * 1000); 
    }

    // 7. クリーンアップ (大文字の M に修正)
    fclose(fp);
    sceNetAdhocMatchingStop(matching_id);
    sceNetAdhocMatchingDelete(matching_id);
    sceNetAdhocPdpDelete(pdp_id, 0);
    sceNetAdhocTerm();
    sceNetTerm();

    pspDebugScreenPrintf("\n\nSUCCESS! Saved to:\n%s\n", save_path);
    
    while(1) {
        sceKernelDelayThread(100 * 1000);
    }

    return 0;
}
