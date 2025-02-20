import ctypes
import time
import struct
import os
import msvcrt
from ctypes import *
from ctypes.wintypes import *

kernel32 = WinDLL('kernel32', use_last_error=True)

FILE_MAP_COPY       = 0x0001
FILE_MAP_WRITE      = 0x0002
FILE_MAP_READ       = 0x0004
FILE_MAP_ALL_ACCESS = 0x001f
FILE_MAP_EXECUTE    = 0x0020

PVOID = LPVOID
SIZE_T = c_size_t

class MEMORY_BASIC_INFORMATION(Structure):
    _fields_ = (('BaseAddress',       PVOID),
                ('AllocationBase',    PVOID),
                ('AllocationProtect', DWORD),
                ('RegionSize',        SIZE_T),
                ('State',             DWORD),
                ('Protect',           DWORD),
                ('Type',              DWORD))

PMEMORY_BASIC_INFORMATION = POINTER(MEMORY_BASIC_INFORMATION)

def errcheck_bool(result, func, args):
    if not result:
        raise WinError(get_last_error())
    return args

kernel32.VirtualQuery.errcheck = errcheck_bool
kernel32.VirtualQuery.restype = SIZE_T
kernel32.VirtualQuery.argtypes = (
    LPCVOID,                   # _In_opt_ lpAddress
    PMEMORY_BASIC_INFORMATION, # _Out_    lpBuffer
    SIZE_T)                    # _In_     dwLength

kernel32.OpenFileMappingW.errcheck = errcheck_bool
kernel32.OpenFileMappingW.restype = HANDLE
kernel32.OpenFileMappingW.argtypes = (
    DWORD,   # _In_ dwDesiredAccess
    BOOL,    # _In_ bInheritHandle
    LPCWSTR) # _In_ lpName

kernel32.MapViewOfFile.errcheck = errcheck_bool
kernel32.MapViewOfFile.restype = LPVOID
kernel32.MapViewOfFile.argtypes = (
    HANDLE, # _In_ hFileMappingObject
    DWORD,  # _In_ dwDesiredAccess
    DWORD,  # _In_ dwFileOffsetHigh
    DWORD,  # _In_ dwFileOffsetLow
    SIZE_T) # _In_ dwNumberOfBytesToMap

kernel32.CloseHandle.errcheck = errcheck_bool
kernel32.CloseHandle.argtypes = (HANDLE,)

def main():
    # Windows APIを使用してメモリーマップトファイルを開く
    nameMMF = "PetiteMonitorUserExtension"
    print("Trying to open MemoryMappedFile")
    mmf = None
    while mmf is None:
        try:
            # メモリーマップトファイルを開く
            hMap = kernel32.OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, False, nameMMF)
            if not hMap:
                raise FileNotFoundError("Could not open file mapping")

            # メモリーマップトファイルのビューを作成
            pBuf = kernel32.MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0)
            if not pBuf:
                raise FileNotFoundError("Could not map view of file")

            # メモリーマップトファイルを閉じる
            kernel32.CloseHandle(hMap)

            # メモリーマップトファイルの情報を取得
            mbi = MEMORY_BASIC_INFORMATION()
            kernel32.VirtualQuery(pBuf, byref(mbi), 128)

            # メモリーマップトファイルからバッファを作成
            mmf = (c_char * mbi.RegionSize).from_address(pBuf)

        except FileNotFoundError:
            # 見つからない場合は500ms待って再試行
            time.sleep(0.5)
    print("MemoryMappedFile opened successfully")

    # シグネチャを確認
    print("Reading signature")
    signature = struct.unpack('I', mmf[0:4])[0]
    if signature != 0x2141594E:
        # シグネチャが一致しない
        raise ValueError("Signature mismatch")
    print("Signature matched")


    # キーが押されるまでLEDを点滅させる
    state = False
    while not msvcrt.kbhit():
        # 状態値を書き込む(100と0を交互に書き込む)
        value = 100 if state else 0
        print(f"Writing {value}")
        mmf[4] = value
        state = not state

        # 1秒待つ
        time.sleep(1)

    # メモリーマップトファイルのビューを開放
    if pBuf:
        kernel32.UnmapViewOfFile(cast(pBuf, LPVOID))


if __name__ == "__main__":
    main()
