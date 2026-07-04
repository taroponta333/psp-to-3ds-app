#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_FILES 32
#define PSP_PACKET_SIZE 1448

char fileList[MAX_FILES][256];
int fileCount = 0;
int selectedIndex = 0;

// sdmc:/3ds フォルダ内をスキャン
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
            // .pbp または .prx ファイルのみをリストに追加
            if (strstr(ent->d_name, ".pbp") || strstr(ent->d_name, ".prx")) {
                strncpy(fileList[fileCount], ent->d_name, 256);
                fileCount++;
            }
        }
    }
    closedir(dir);
}

// 画面描画
void drawMenu() {
    consoleClear();
    printf("=== 3DS to PSP File Sender ===\n");
    printf("Up/Down: Select  |  A: Send  |  START: Exit\n\n");

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

// 送信処理（擬似エミュレート）
void sendFileToPSP(const char* fileName) {
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "sdmc:/3ds/%s", fileName);
    
    printf("\nOpening: %s\n", fileName);
    
    FILE *file = fopen(fullPath, "rb");
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        return;
    }

    printf("Emulating RAW Socket packet transfer...\n");
    // ここに実際のRAWソケット送信ロジックが入ります
    svcSleepThread(2000000000ULL); // 2秒待機
    
    fclose(file);
    printf("Transfer Complete successfully!\n");
}

int main(int argc, char **argv) {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

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
            svcSleepThread(3000000000ULL); // 3秒待機
            drawMenu();
        }

        if (kDown & KEY_START) break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
