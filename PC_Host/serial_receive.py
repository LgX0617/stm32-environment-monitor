import serial
import threading

ser = serial.Serial()

ser.port = "COM15"
ser.baudrate = 115200
ser.bytesize = serial.EIGHTBITS
ser.parity = serial.PARITY_NONE
ser.stopbits = serial.STOPBITS_ONE
ser.timeout = 1

ser.xonxoff = False
ser.rtscts = False
ser.dsrdtr = False

# 在打开串口前明确关闭控制线，避免影响开发板复位/启动状态。
ser.dtr = False
ser.rts = False

def parse_data_frame(message):
    if not message.startswith("$DATA,"):
        return None

    fields = {}

    payload = message[len("$DATA,"):]
    items = payload.split(",")

    for item in items:
        key, value = item.split("=")
        fields[key] = int(value)

    return fields

def receive_loop(ser):
    while ser.is_open:
        raw_data = ser.readline()

        if raw_data:
            message = raw_data.decode("utf-8", errors="replace").strip()

            if message.startswith("$DATA,"):
                data = parse_data_frame(message)
                print(f"\n光照 ADC：{data['L']}，报警：{data['A']}")

            elif message.startswith("$ACK,"):
                print("\nSTM32 确认：", message)

            elif message.startswith("$ERR,"):
                print("\nSTM32 拒绝命令：", message)

            else:
                print("\n未知帧：", message)

try:
    ser.open()

    print(f"已打开：{ser.name}")
    print(f"DTR={ser.dtr}, RTS={ser.rts}")
    print("等待原始字节，按 Ctrl+C 停止。")

    receiver_thread = threading.Thread(
    target=receive_loop,
    args=(ser,),
    daemon=True
)

    receiver_thread.start()



    
    while True:
        user_input = input("\n输入过暗阈值 0-4095,输入 q 退出：").strip()

        if user_input.lower() == "q":
            break

        if not user_input.isdigit():
            print("请输入 0 到 4095 的整数。")
            continue

        threshold = int(user_input)

        if threshold > 4095:
            print("阈值不能大于 4095。")
            continue

        command = f"$SET,L_thr,{threshold}\r\n"
        ser.write(command.encode("ascii"))

        print("已发送：", command.strip())

except serial.SerialException as error:
    print("串口错误：", error)

except KeyboardInterrupt:
    print("\n程序已停止。")

finally:
    if ser.is_open:
        ser.close()