#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <malloc.h>

#define MAX_FILES 32
#define PSP_PACKET_SIZE 1448
#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x10000 // 64KB

char fileList[MAX_FILES][256];
int fileCount = 0;
int selectedIndex = 0;
static u32 *soc_buffer = NULL;

// sdmc:/3ds フォルダ内をスキャンしてリスト化
void scanDirectory() {
    DIR *dir;
    struct dirent *ent;
    fileCount = 0;

    dir = opendir("sdmc:/3ds");
    if (dir == NULL) {
        printf("sdmc:/3ds directory not found.\n");
        return;
    }

    while ((ent = readdir(dir)) != NULL && fileCount < MAX_FILES) {
        if (ent->d_type == DT_REG) {
            // PSP用のファイルだけを抽出
            if (strstr(ent->d_name, ".pbp") || strstr(ent->d_name, ".prx")) {
                strncpy(fileList[fileCount], ent->d_name, 256);
                fileCount++;
            }
        }
    }
    closedir(dir);
}

// 画面のメニュー描画
void drawMenu() {
    consoleClear();
    printf("=== 3DS to PSP File Sender ===\n");
    printf("Up/Down: Select  |  A: Broadcast Send  |  START: Exit\n\n");

    if (fileCount == 0) {
        printf("No compatible files found (.pbp / .prx)\n");
        printf("Please put files in sdmc:/3ds/\n");
        return;
    }

    for (int i = 0; i < fileCount; i++) {
        if (i == selectedIndex) {
            printf("> [%s]\n", fileList[i]);
        } else {
            printf("  %s\n", fileList[i]);
        }
    }
}

// ✨ IP入力不要！一斉バラマキ（ブロードキャスト）送信関数
void sendFileToPSP(const char* fileName) {
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "sdmc:/3ds/%s", fileName);
    
    printf("\nOpening: %s\n", fileName);
    
    FILE *file = fopen(fullPath, "rb");
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        return;
    }

    // UDPソケットの作成
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("Error: Could not create socket.\n");
        fclose(file);
        return;
    }

    // ルーター内に一斉送信（ブロードキャスト）する許可を出す設定
    int broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission)) < 0) {
        printf("Error: Broadcasting permission failed.\n");
        close(sock);
        fclose(file);
        return;
    }

    // 宛先IPを「255.255.255.255（全員宛て）」に固定！IP入力は不要！
    struct sockaddr_in psp_broadcast_addr;
    memset(&psp_broadcast_addr, 0, sizeof(psp_broadcast_addr));
    psp_broadcast_addr.sin_family = AF_INET;
    psp_broadcast_addr.sin_port = htons(8080); // PSP側が待ち受けるポート
    psp_broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    printf("Broadcasting file via Wi-Fi LAN...\n");

    char buffer[PSP_PACKET_SIZE];
    size_t bytesRead;
    int packetCount = 0;

    // ファイルを読み込みながら電波に放り投げるループ
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        ssize_t sentBytes = sendto(sock, buffer, bytesRead, 0, 
                                   (struct sockaddr*)&psp_broadcast_addr, 
                                   sizeof(psp_broadcast_addr));
        
        if (sentBytes < 0) {
            printf("Error during transfer!\n");
            break;
        }

        packetCount++;
        if (packetCount % 50 == 0) printf("."); // 進捗をドットで表示
        
        // PSPの処理落ち（パケットロス）を防ぐために5ミリ秒だけ休憩
        svcSleepThread(5000000ULL); 
    }
    
    printf("\nTotal Packets Sent: %d\n", packetCount);

    close(sock);
    fclose(file);
    printf("Transfer Complete successfully!\n");
}

int main(int argc, char **argv) {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    // 3DSのネットワーク機能（SOC）の初期化
    soc_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    if (soc_buffer != NULL) {
        socInit(soc_buffer, SOC_BUFFERSIZE);
    } else {
        printf("SOC Buffer allocation failed!\n");
    }

    scanDirectory() ;
    drawMenu();

    // メインループ
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_DOWN) {
            if (selectedIndex < fileCount - 1) {
                selectedIndex++;
                drawMenu();
            }
        }
        
        if (kDown & KEY_UP) {
            if (selectedIndex > 0) {
                selectedIndex--;
                drawMenu();
            }
        }
        
        if (kDown & KEY_A && fileCount > 0) {
            sendFileToPSP(fileList[selectedIndex]);
            svcSleepThread(3000000000ULL); // 完了画面を3秒見せる
            drawMenu();
        }

        if (kDown & KEY_START) break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    // アプリ終了時のネットワーク後片付け
    socExit();
    if (soc_buffer) free(soc_buffer);

    gfxExit();
    return 0;
}
