#include <windows.h>
#include <iostream>
#include <conio.h>
#include <thread>
#include <memory>
#include "PetiteMonitorLib.h"

PetiteMonitorLib::PetiteMonitorLib() {
    pBuf = nullptr;
}

PetiteMonitorLib::~PetiteMonitorLib() {
    Disconnect();
}

void PetiteMonitorLib::Connect() {
    // 既に接続済みの場合は例外を投げる
    if ( pBuf != nullptr ) {
        throw std::runtime_error( "Already connected" );
    }

    // メモリーマップトファイルの名前
    const char* nameMMF = "PetiteMonitorUserExtension";

    // メモリーマップドファイルのハンドル
    HANDLE hMapFile = NULL;

    // スコープガード
    std::unique_ptr<void, decltype( &CloseHandle )> hMapFileGuard( hMapFile, CloseHandle );
    auto pBufDeleter = [ this ]( uint8_t* ) { this->Disconnect(); };
    std::unique_ptr<uint8_t, decltype( pBufDeleter )> pBufGuard( pBuf, pBufDeleter );

    // メモリーマップトファイルを開く
    while ( hMapFile == NULL ) {
        hMapFile = OpenFileMappingA( FILE_MAP_READ | FILE_MAP_WRITE, FALSE, nameMMF );
        if ( hMapFile == NULL ) {
            // 見つからない場合は500ms待って再試行
            std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
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

void PetiteMonitorLib::Disconnect() {
    if ( pBuf != nullptr ) {
        UnmapViewOfFile( pBuf );
        pBuf = nullptr;
    }
}

void PetiteMonitorLib::WriteValue( int index, uint8_t value ) {
    if ( pBuf == nullptr ) {
        throw std::runtime_error( "Not connected" );
    }
    pBuf[ index + 4 ] = value;
}

uint8_t PetiteMonitorLib::ReadValue( int index ) {
    if ( pBuf == nullptr ) {
        throw std::runtime_error( "Not connected" );
    }
    return pBuf[ index + 4 ];
}
