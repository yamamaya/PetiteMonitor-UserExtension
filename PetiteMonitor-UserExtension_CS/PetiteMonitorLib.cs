using System;
using System.IO.MemoryMappedFiles;

namespace OaktreeLab.PetiteMonitor {
    public class PetiteMonitorLib : IDisposable {
        public PetiteMonitorLib() {
        }

        private MemoryMappedViewAccessor? accessor;
        private bool disposedValue;

        protected virtual void Dispose( bool disposing ) {
            if ( !disposedValue ) {
                if ( disposing ) {
                    accessor?.Dispose();
                    accessor = null;
                }
                disposedValue = true;
            }
        }

        public void Dispose() {
            Dispose( disposing: true );
            GC.SuppressFinalize( this );
        }

        /// <summary>
        /// PetiteMonitorに接続します。
        /// </summary>
        /// <remarks>
        /// 無制限に再試行します。
        /// </remarks>
        /// <exception cref="ApplicationException">接続に失敗した場合</exception>
        public void Connect() {
            Connect( null );
        }

        /// <summary>
        /// PetiteMonitorに接続します。
        /// </summary>
        /// <remarks>
        /// 再試行回数を指定することができます。1回1秒、nullの場合は無制限に再試行。
        /// </remarks>
        /// <param name="retryAttempts">再試行回数(nullまたは0以上の整数)</param>
        /// <exception cref="ApplicationException">接続に失敗した場合</exception>
        public void Connect( int? retryAttempts ) {
            if ( accessor != null ) {
                throw new ApplicationException( "Already connected" );
            }

            // メモリーマップドファイルの名前
            string nameMMF = "PetiteMonitorUserExtension";

            // メモリーマップトファイルを開く
            MemoryMappedFile? mmf = null;
            int attemptCount = 0;
            while ( mmf == null ) {
                try {
                    mmf = MemoryMappedFile.OpenExisting( nameMMF, MemoryMappedFileRights.ReadWrite );
                } catch ( FileNotFoundException ) {
                    // 見つからない場合は1s待って再試行
                    attemptCount++;
                    if ( retryAttempts != null && attemptCount > retryAttempts ) {
                        throw new ApplicationException( "MemoryMappedFile not found" );
                    }
                    Thread.Sleep( 1000 );
                }
            }

            // メモリーマップトファイルのビューを作成
            accessor = mmf.CreateViewAccessor();

            // メモリーマップトファイルを閉じる(もう不要)
            mmf.Dispose();

            // シグネチャを確認
            UInt32 signature = accessor.ReadUInt32( 0 );
            if ( signature != 0x2141594E ) {
                // シグネチャが一致しない
                throw new ApplicationException( "Signature mismatch" );
            }
        }

        /// <summary>
        /// ユーザー拡張データを書き込みます。
        /// </summary>
        /// <remarks>
        /// indexは0から3までの値を指定します。
        /// valueはデジタルの場合は0か0以外の値、アナログの場合は0から100の値を指定します。
        /// </remarks>
        /// <param name="index">0から3のインデックス</param>
        /// <param name="value">デジタルの場合は0か0以外、アナログの場合は0から100</param>
        /// <exception cref="ApplicationException"></exception>
        public void WriteValue( int index, byte value ) {
            if ( accessor == null ) {
                throw new ApplicationException( "Not connected" );
            }
            if ( index < 0 || index > 3 ) {
                throw new IndexOutOfRangeException( "Index out of range" );
            }
            accessor.Write( 4 + index, value );
        }

        /// <summary>
        /// ユーザー拡張データを読み込みます。
        /// </summary>
        /// <remarks>
        /// indexは0から3までの値を指定します。
        /// </remarks>
        /// <param name="index">0から3のインデックス</param>
        /// <returns></returns>
        /// <exception cref="ApplicationException"></exception>
        public byte ReadValue( int index ) {
            if ( accessor == null ) {
                throw new ApplicationException( "Not connected" );
            }
            return accessor.ReadByte( 4 + index );
        }
    }
}
