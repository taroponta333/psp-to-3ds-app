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

// 進捗表示用の行クリア＆再描画関数
void printStatusLine(int x, int y, const char* msg) {
    printf("\x1b[%d;%dH                                                                ", y + 1, x + 1); // 行消去
    printf("\x1b[%d;%dH%s", y + 1, x + 1, msg);
}

// IP入力不要！一斉バラマキ（ブロードキャスト）送信関数
void sendFileToPSP(const char* fileName) {
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "sdmc:/3ds/%s", fileName);
    
    consoleClear();
    printf("=== Transmission Mode ===\n\n");
    printf("Opening file: %s\n", fileName);
    
    FILE *file = fopen(fullPath, "rb");
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        return;
    }

    // 💡 ファイルの総サイズを取得
    fseek(file, 0, SEEK_END);
    unsigned int total_file_size = (unsigned int)ftell(file);
    fseek(file, 0, SEEK_SET);

    // UDPソケットの作成
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("Error: Could not create socket.\n");
        fclose(file);
        return;
    }

    // ブロードキャスト許可
    int broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission, sizeof(broadcastPermission)) < 0) {
        printf("Error: Broadcasting permission failed.\n");
        close(sock);
        fclose(file);
        return;
    }

    // 宛先IP（全員宛て）固定
    struct sockaddr_in psp_broadcast_addr;
    memset(&psp_broadcast_addr, 0, sizeof(psp_broadcast_addr));
    psp_broadcast_addr.sin_family = AF_INET;
    psp_broadcast_addr.sin_port = htons(8080);
    psp_broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    printf("Connecting to network...\n");

    char buffer[PSP_PACKET_SIZE];
    size_t bytesRead;
    unsigned int sent_bytes_sum = 0;
    int is_first_packet = 1;

    // 表示行の固定
    const int STATUS_Y = 5;
    const int PROGRESS_Y = 7;

    printStatusLine(0, STATUS_Y, "[STATUS] Broadcasting file via Wi-Fi LAN...");

    // ファイルを読み込みながら送信するループ
    while (1) {
        int packet_data_size = PSP_PACKET_SIZE;
        char *send_ptr = buffer;

        // 💡 【重要】最初のパケットの先頭4バイトに総ファイルサイズを埋め込む
        if (is_first_packet) {
            memcpy(buffer, &total_file_size, sizeof(unsigned int));
            send_ptr += sizeof(unsigned int);
            packet_data_size -= sizeof(unsigned int); // データ領域を4バイト削る
        }

        bytesRead = fread(send_ptr, 1, packet_data_size, file);
        
        // 読み込むデータがなくなったら終了
        if (bytesRead <= 0 && !is_first_packet) {
            break; 
        }

        // 実際に送信するパケットのサイズを計算
        size_t send_size = bytesRead;
        if (is_first_packet) {
            send_size += sizeof(unsigned int); // 最初のパケットだけサイズ情報を足す
            is_first_packet = 0;
        }

        ssize_t sentBytes = sendto(sock, buffer, send_size, 0, 
                                   (struct sockaddr*)&psp_broadcast_addr, 
                                   sizeof(psp_broadcast_addr));
        
        if (sentBytes < 0) {
            printStatusLine(0, STATUS_Y, "[ERR] Error occurred during transfer!");
            break;
        }

        // 送信した「純粋なファイルデータ」のバイト数を加算
        sent_bytes_sum += bytesRead;

        // 📊 リアルタイム進捗表示（画面のチラつきなし）
        char progress_msg[128];
        int percent = (int)(((long long)sent_bytes_sum * 100) / total_file_size);
        if (percent > 100) percent = 100;

        snprintf(progress_msg, sizeof(progress_msg), "Progress: %d%% (%u / %u bytes)", percent, sent_bytes_sum, total_file_size);
        printStatusLine(0, PROGRESS_Y, progress_msg);
        
        // PSPのパケットロス（処理落ち）防止用待ち時間（5ミリ秒）
        svcSleepThread(5000000ULL); 
    }
    
    close(sock);
    fclose(file);

    printStatusLine(0, STATUS_Y, "[STATUS] Transfer Complete successfully!");
}

int main(int argc, char **argv) {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    soc_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    if (soc_buffer != NULL) {
        socInit(soc_buffer, SOC_BUFFERSIZE);
    } else {
        printf("SOC Buffer allocation failed!\n");
    }

    scanDirectory();
    drawMenu();

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
            svcSleepThread(3000000000ULL); // 完了画面を3秒保持
            drawMenu();
        }

        if (kDown & KEY_START) break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    socExit();
    if (soc_buffer) free(soc_buffer);

    gfxExit();
    return 0;
}
