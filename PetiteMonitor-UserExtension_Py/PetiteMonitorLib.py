# PetiteMonitorLib.py
import ctypes
import time
import struct
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

class PetiteMonitor:
    # コンストラクタ
    def __init__(self):
        self.__mmf = None
        self.__pBuf = None

    # PetiteMonitorと接続
    def connect(self):
        # メモリマップ度ファイルの名前
        nameMMF = "PetiteMonitorUserExtension"

        # メモリマップドファイルを開く(成功するまで繰り返す)
        while self.__mmf is None:
            try:
                # メモリマップドファイルを既存モードで開く
                hMap = kernel32.OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, False, nameMMF)
                if not hMap:
                    raise FileNotFoundError("Could not open file mapping")

                # メモリマップドファイルのビューをメモリにマップする
                self.__pBuf = kernel32.MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0)
                if not self.__pBuf:
                    raise OSError("Could not map view of file")

                # メモリマップドファイルを閉じる(もう不要)
                kernel32.CloseHandle(hMap)

                # メモリマップドファイルの情報を取得
                mbi = MEMORY_BASIC_INFORMATION()
                kernel32.VirtualQuery(self.__pBuf, byref(mbi), 128)

                # メモリマップドファイルのアドレスを取得
                self.__mmf = (c_char * mbi.RegionSize).from_address(self.__pBuf)

                # シグネチャを確認
                signature = struct.unpack('I', self.__mmf[0:4])[0]
                if signature != 0x2141594E:
                    raise Exception("Signature mismatch")

            except FileNotFoundError:
                # メモリマップドファイルが存在しない場合は、0.5秒待って再試行
                time.sleep(0.5)

            except Exception as e:
                # その他のエラーの場合は例外を送出
                raise e

    # PetiteMonitorと切断
    def disconnect(self):
        # ビューを閉じる
        if self.__pBuf:
            kernel32.UnmapViewOfFile(cast(self.__pBuf, LPVOID))

    # バイト単位で読み取る
    def read_byte(self, index):
        if self.__mmf is None:
            raise Exception("Not connected to memory-mapped file")
        return self.__mmf[4 + index]

    # バイト単位で書き込む
    def write_byte(self, index, value):
        if self.__mmf is None:
            raise Exception("Not connected to memory-mapped file")
        self.__mmf[4 + index] = value

    # with文のために
    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.disconnect()
