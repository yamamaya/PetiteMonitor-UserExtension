#pragma once

#include <stdint.h>

/// <summary>
/// PetiteMonitorユーザー拡張ライブラリ
/// </summary>
class PetiteMonitorLib {
public:
    PetiteMonitorLib();
    virtual ~PetiteMonitorLib();

    /// <summary>
    /// PetiteMonitorに接続
    /// </summary>
    void Connect();

    /// <summary>
    /// PetiteMonitorから切断
    /// </summary>
    void Disconnect();

    /// <summary>
    /// ユーザー拡張データを書き込みます。
    /// </summary>
    /// <remarks>
    /// indexは0から3までの値を指定します。
    /// </remarks>
    /// <param name="index"></param>
    /// <param name="value"></param>
    void WriteValue( int index, uint8_t value );

    /// <summary>
    /// ユーザー拡張データを読み込みます。
    /// </summary>
    /// <remarks>
    /// indexは0から3までの値を指定します。
    /// </remarks>
    /// <param name="index"></param>
    /// <returns></returns>
    uint8_t ReadValue( int index );

private:
    uint8_t* pBuf;
};

