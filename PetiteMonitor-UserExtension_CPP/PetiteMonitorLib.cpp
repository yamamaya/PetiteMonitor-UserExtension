#include <windows.h>
#include <iostream>
#include <conio.h>
#include <thread>
#include <memory>
#include "PetiteMonitorLib.h"

// コンストラクタ
PetiteMonitorLib::PetiteMonitorLib() {
    pBuf = nullptr;
}

// デストラクタ
PetiteMonitorLib::~PetiteMonitorLib() {
    Disconnect();
}

// PetiteMonitorに接続
void PetiteMonitorLib::Connect( int retryAttempts ) {
    // 既に接続済みの場合は例外を投げる
    if ( pBuf != nullptr ) {
        throw std::runtime_error( "Already connected" );
    }

    // メモリーマップトファイルの名前
    const char* nameMMF = "PetiteMonitorUserExtension";

    // メモリーマップドファイルのハンドル
    HANDLE hMapFile = NULL;

    // hMapFileのスコープガード
    std::unique_ptr<void, decltype( &CloseHandle )> hMapFileGuard( hMapFile, CloseHandle );
    // pBufのスコープガード
    auto pBufDeleter = [ this ]( uint8_t* ) { this->Disconnect(); };
    std::unique_ptr<uint8_t, decltype( pBufDeleter )> pBufGuard( pBuf, pBufDeleter );

    // メモリーマップトファイルを開く
    int retryCount = 0;
    while ( hMapFile == NULL ) {
        hMapFile = OpenFileMappingA( FILE_MAP_READ | FILE_MAP_WRITE, FALSE, nameMMF );
        if ( hMapFile == NULL ) {
            // 見つからない場合は1s待って再試行
            retryCount++;
            if ( retryAttempts >= 0 && retryCount > retryAttempts ) {
                throw std::runtime_error( "Could not open file mapping" );
            }
            std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
        }
    }

    // メモリーマップトファイルのビューを作成
    pBuf = (uint8_t*)MapViewOfFile( hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0 );
    if ( pBuf == nullptr ) {
        throw std::runtime_error( "Could not map view of file" );
    }

    // メモリーマップトファイルを閉じる(もう不要)
    hMapFileGuard.reset();

    // シグネチャを確認
    uint32_t signature = *(uint32_t*)pBuf;
    if ( signature != 0x2141594E ) {
        // シグネチャが一致しない
        throw std::runtime_error( "Signature mismatch" );
    }

    // pBufGuardの管理を解除
    pBufGuard.release();
}

// PetiteMonitorから切断
void PetiteMonitorLib::Disconnect() {
    if ( pBuf != nullptr ) {
        UnmapViewOfFile( pBuf );
        pBuf = nullptr;
    }
}

// ユーザー拡張データを書き込み
void PetiteMonitorLib::WriteValue( int index, uint8_t value ) {
    if ( pBuf == nullptr ) {
        throw std::runtime_error( "Not connected" );
    }
    pBuf[ index + 4 ] = value;
}

// ユーザー拡張データを読み込み
uint8_t PetiteMonitorLib::ReadValue( int index ) {
    if ( pBuf == nullptr ) {
        throw std::runtime_error( "Not connected" );
    }
    return pBuf[ index + 4 ];
}
