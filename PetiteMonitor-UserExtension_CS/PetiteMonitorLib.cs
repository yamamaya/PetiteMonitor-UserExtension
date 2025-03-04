using System;
using System.Collections.Generic;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
        /// <exception cref="ApplicationException"></exception>
        public void Connect() {
            if ( accessor != null ) {
                throw new ApplicationException( "Already connected" );
            }

            // メモリーマップドファイルの名前
            string nameMMF = "PetiteMonitorUserExtension";

            // メモリーマップトファイルを開く
            MemoryMappedFile? mmf = null;
            while ( mmf == null ) {
                try {
                    mmf = MemoryMappedFile.OpenExisting( nameMMF, MemoryMappedFileRights.ReadWrite );
                } catch ( FileNotFoundException ) {
                    // 見つからない場合は500ms待って再試行
                    Thread.Sleep( 500 );
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
        /// </remarks>
        /// <param name="index"></param>
        /// <param name="value"></param>
        /// <exception cref="ApplicationException"></exception>
        public void WriteValue( int index, byte value ) {
            if ( accessor == null ) {
                throw new ApplicationException( "Not connected" );
            }
            accessor.Write( 4 + index, value );
        }

        /// <summary>
        /// ユーザー拡張データを読み込みます。
        /// </summary>
        /// <remarks>
        /// indexは0から3までの値を指定します。
        /// </remarks>
        /// <param name="index"></param>
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
