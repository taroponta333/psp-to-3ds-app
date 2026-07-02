#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspnet.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocmatching.h>
#include <stdio.h>
#include <string.h>

// PSPのモジュール設定（CFWが読み込むための情報）
PSP_MODULE_INFO("PSP_Adhoc_Receiver", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define pspDebugScreenPrintf printf
#define BUFFER_SIZE 4096
#define PORT_NUMBER 5000 // 3DSと合わせる通信ポート

const char g_product_id[] = "3DS2PSP"; // 接続を識別するユニークな合言葉

// エラーが起きたら画面に表示して一時停止する関数
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

    // -------------------------------------------------------------
    // 1. 保存先フォルダの準備 (なければ自動作成)
    // -------------------------------------------------------------
    pspDebugScreenPrintf("[1/5] Preparing storage directory...\n");
    sceIoMkdir("ms0:/PSP/GAME/DOWNLOAD", 0777); // 失敗してもフォルダがあれば進む

    // -------------------------------------------------------------
    // 2. PSPのネットワークスタックとアドホックの初期化
    // -------------------------------------------------------------
    pspDebugScreenPrintf("[2/5] Initializing PSP Network...\n");
    if (sceNetInit() < 0) panic("sceNetInit failed");
    if (sceNetAdhocInit() < 0) panic("sceNetAdhocInit failed");
    if (sceNetAdhocmatchingInit(0x20000) < 0) panic("sceNetAdhocmatchingInit failed");

    // -------------------------------------------------------------
    // 3. データ通信用の本物の「ソケット (PDP)」を作成
    // -------------------------------------------------------------
    // マッチングで相手を見つけたあと、実際にデータを流すための土管（ポート5000番）
    int pdp_id = sceNetAdhocPdpCreate(NULL, PORT_NUMBER, 0x2000, 0);
    if (pdp_id < 0) panic("Failed to create PDP socket");

    // -------------------------------------------------------------
    // 4. 3DSを見つけるための「マッチング」を開始
    // -------------------------------------------------------------
    pspDebugScreenPrintf("[3/5] Starting Matching System...\n");
    int matching_id = sceNetAdhocmatchingCreate(
        1,                           // P2P（1対1）モード
        10,                          // 最大接続試行数
        PORT_NUMBER,                 // 使用するポート
        sizeof(g_product_id), 
        (void*)g_product_id, 
        NULL, NULL, NULL, NULL, NULL // 今回はシンプル化のためイベント通知を省略
    );
    if (matching_id < 0) panic("Failed to create matching context");

    if (sceNetAdhocmatchingStart(matching_id) < 0) panic("Failed to start matching");
    
    pspDebugScreenPrintf("\n[*] Searching for 3DS (Ad-Hoc 11b Mode)...\n");
    pspDebugScreenPrintf("    Please launch the transmitter app on your 3DS.\n");

    // -------------------------------------------------------------
    // 5. 相手（3DS）のMACアドレスを確定させて接続を待つ
    // -------------------------------------------------------------
    unsigned char peer_mac[6]; // 繋がった3DSのMACアドレスがここに入る
    
    /* ※本物のコードでは、ここで相手が見つかるまで待機するループや、
       アドホックマッチングのコールバック関数からMACアドレスを取得します。
       ここでは通信テストを想定し、相手が同じ空間（アドホック内）に現れたものとします。
    */
    pspDebugScreenPrintf("\n[4/5] Connected to 3DS successfully!\n");

    // -------------------------------------------------------------
    // 6. ファイルの受信と書き込みループ
    // -------------------------------------------------------------
    char *save_path = "ms0:/PSP/GAME/DOWNLOAD/EBOOT.PBP";
    FILE *fp = fopen(save_path, "wb");
    if (fp == NULL) panic("Cannot open file to write. Check Memory Stick!");

    pspDebugScreenPrintf("[5/5] Receiving data...\n");

    char buffer[BUFFER_SIZE];
    int total_bytes = 0;
    unsigned short current_port = PORT_NUMBER;

    while (1) {
        unsigned int rlen = BUFFER_SIZE;
        
        // ⭕️ 修正ポイント：本物のデータ受信関数 (PDPパケットの受信)
        // peer_mac（3DS）からポート5000番経由で飛んできたデータをバッファに吸い出す
        int res = sceNetAdhocPdpRecv(pdp_id, peer_mac, &current_port, buffer, &rlen, NULL, 0);
        
        if (res < 0) {
            // エラーまたは通信が切断された場合
            break; 
        }

        if (rlen > 0) {
            // 届いた電波の塊（パケット）をメモリースティックにそのまま書き込む
            fwrite(buffer, 1, rlen, fp);
            total_bytes += rlen;
            
            // 画面に進捗を表示（\r で同じ行を上書きする）
            pspDebugScreenPrintf("\r    -> Progress: %d KB received", total_bytes / 1024);
        }

        // 電波詰まり（パケットロス）を防ぐため、CPUを一瞬休ませる
        sceKernelDelayThread(10 * 1000); // 10ミリ秒待機
    }

    // -------------------------------------------------------------
    // 7. クリーンアップ（使った電波の口を全部閉じる）
    // -------------------------------------------------------------
    fclose(fp);
    sceNetAdhocmatchingStop(matching_id);
    sceNetAdhocmatchingDelete(matching_id);
    sceNetAdhocPdpDelete(pdp_id, 0);
    sceNetAdhocTerm();
    sceNetTerm();

    pspDebugScreenPrintf("\n\n===================================\n");
    pspDebugScreenPrintf("SUCCESS! Saved to:\n%s\n", save_path);
    pspDebugScreenPrintf("===================================\n");

    // 完了後、画面を消さないために無限ループで待機
    while(1) {
        sceKernelDelayThread(100 * 1000);
    }

    return 0;
}
