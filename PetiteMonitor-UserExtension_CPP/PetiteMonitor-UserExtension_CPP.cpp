#include <windows.h>
#include <iostream>
#include <conio.h>
#include <thread>

int main() {
    // メモリーマップトファイルを開く
    const char* nameMMF = "PetiteMonitorUserExtension";
    HANDLE hMapFile = NULL;
    std::cout << "Trying to open MemoryMappedFile" << std::endl;
    while ( hMapFile == NULL ) {
        hMapFile = OpenFileMappingA( FILE_MAP_READ | FILE_MAP_WRITE, FALSE, nameMMF );
        if ( hMapFile == NULL ) {
            // 見つからない場合は500ms待って再試行
            std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
        }
    }
    std::cout << "MemoryMappedFile opened successfully" << std::endl;

    // メモリーマップトファイルのビューを作成
    uint8_t* pBuf = (uint8_t*)MapViewOfFile( hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0 );
    if ( pBuf == NULL ) {
        std::cerr << "Could not map view of file" << std::endl;
        CloseHandle( hMapFile );
        return 1;
    }

    // メモリーマップトファイルを閉じる
    CloseHandle( hMapFile );

    // シグネチャを確認
    std::cout << "Reading signature" << std::endl;
    uint32_t signature = *(uint32_t*)pBuf;
    if ( signature != 0x2141594E ) {
        // シグネチャが一致しない
        std::cout << "Signature mismatch" << std::endl;
        UnmapViewOfFile( pBuf );
        return 1;
    }
    std::cout << "Signature matched" << std::endl;

    // キーが押されるまでLEDを点滅させる
    bool state = false;
    while ( !_kbhit() ) {
        // 状態値を書き込む(100と0を交互に書き込む)
        uint8_t value;
        if ( state ) {
            value = 100;
        } else {
            value = 0;
        }
        std::cout << "Writing " << (int)value << std::endl;
        pBuf[ 4 ] = value;
        state = !state;

        // 1秒待つ
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    }

    // メモリーマップトファイルのビューを解放
    UnmapViewOfFile( pBuf );

    return 0;
}
