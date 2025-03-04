#include <windows.h>
#include <iostream>
#include <conio.h>
#include <thread>
#include "PetiteMonitorLib.h"

int main() {
    std::cout << "PetiteMonitor User Extension C++ sample" << std::endl;

    // PetiteMonitorLibのインスタンス
    PetiteMonitorLib monitor;
    // PetiteMonitorに接続
    monitor.Connect();

    std::cout << "Connected to PetiteMonitor successfully" << std::endl;
    std::cout << "Press any key to exit" << std::endl;

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
        monitor.WriteValue( 0, value );
        state = !state;

        // 1秒待つ
        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    }

    // PetiteMonitorから切断
    monitor.Disconnect();

    return 0;
}
