using OaktreeLab.PetiteMonitor;
using System.IO.MemoryMappedFiles;

namespace PetiteMonitor_UserExtension_CS {
    internal class Program {
        static void Main( string[] args ) {
            Console.WriteLine( "PetiteMonitor User Extension C# sample" );

            // PetiteMonitorLibのインスタンスを作成
            using PetiteMonitorLib monitor = new();
            // PetiteMonitorに接続
            monitor.Connect();

            Console.WriteLine( "Connected to PetiteMonitor successfully" );
            Console.WriteLine( "Press any key to exit" );

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
                monitor.WriteValue( 0, value );
                state = !state;

                // 1秒待つ
                Thread.Sleep( 1000 );
            }
        }
    }
}
