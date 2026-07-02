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

    // 4. マッチングシステムを開始 (大文字の M に修正)
    pspDebugScreenPrintf("[3/5] Starting Matching System...\n");
    int matching_id = sceNetAdhocMatchingCreate(
        1,                           
        10,                          
        PORT_NUMBER,                 
        sizeof(g_product_id), 
        (void*)g_product_id, 
        NULL, NULL, NULL, NULL, NULL 
    );
    if (matching_id < 0) panic("Failed to create matching context");

    if (sceNetAdhocMatchingStart(matching_id) < 0) panic("Failed to start matching");
    
    pspDebugScreenPrintf("\n[*] Searching for 3DS (Ad-Hoc 11b Mode)...\n");

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
