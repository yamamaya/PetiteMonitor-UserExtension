# PetiteMonitor_UserExtension_Py.py
import time
import msvcrt
from PetiteMonitorLib import PetiteMonitor

def main():
    print("PetiteMonitor User Extension Python sample")

    # PetiteMonitorに接続
    with PetiteMonitor() as monitor:
        print("Connected to PetiteMonitor successfully")
        print("Press any key to exit")

        # キーが押されるまでLEDを点滅させる
        state = False
        while not msvcrt.kbhit():
            # 状態値を書き込む(100と0を交互に書き込む)
            value = 100 if state else 0
            print(f"Writing {value}")
            monitor.write_byte(0, value)
            state = not state

            # 1秒待つ
            time.sleep(1)

if __name__ == "__main__":
    main()
