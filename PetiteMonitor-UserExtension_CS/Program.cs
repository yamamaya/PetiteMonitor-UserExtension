using System.IO.MemoryMappedFiles;

namespace PetiteMonitor_UserExtension_CS {
    internal class Program {
        static void Main( string[] args ) {
            // メモリーマップトファイルを開く
            string nameMMF = "PetiteMonitorUserExtension";
            MemoryMappedFile? mmf = null;
            Console.WriteLine( "Trying to open MemoryMappedFile" );
            while ( mmf == null ) {
                try {
                    mmf = MemoryMappedFile.OpenExisting( nameMMF, MemoryMappedFileRights.ReadWrite );
                } catch ( FileNotFoundException ) {
                    // 見つからない場合は500ms待って再試行
                    Thread.Sleep( 500 );
                }
            }
            Console.WriteLine( "MemoryMappedFile opened successfully" );

            // メモリーマップトファイルのビューを作成
            MemoryMappedViewAccessor accessor = mmf.CreateViewAccessor();

            // メモリーマップトファイルを閉じる
            mmf.Dispose();

            // シグネチャを確認
            Console.WriteLine( "Reading signature" );
            UInt32 signature = accessor.ReadUInt32( 0 );
            if ( signature != 0x2141594E ) {
                // シグネチャが一致しない
                Console.WriteLine( "Signature mismatch" );
                return;
            }
            Console.WriteLine( "Signature matched" );

            // キーが押されるまでLEDを点滅させる
            bool state = false;
            while ( Console.KeyAvailable == false ) {
                // 状態値を書き込む(100と0を交互に書き込む)
                byte value;
                if ( state ) {
                    value = 100;
                } else {
                    value = 0;
                }
                Console.WriteLine( $"Writing {value}" );
                accessor.Write( 4, value );
                state = !state;

                // 1秒待つ
                Thread.Sleep( 1000 );
            }

            // メモリーマップトファイルのビューを解放
            accessor.Dispose();
        }
    }
}
