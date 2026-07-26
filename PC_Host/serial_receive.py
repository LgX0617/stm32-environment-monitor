import serial

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

try:
    ser.open()

    print(f"已打开：{ser.name}")
    print(f"DTR={ser.dtr}, RTS={ser.rts}")
    print("等待原始字节，按 Ctrl+C 停止。")

    while True:
        raw_data = ser.readline()

        if raw_data:
            message = raw_data.decode("utf-8", errors="replace").strip()
            print("收到：", message)

except serial.SerialException as error:
    print("串口错误：", error)

except KeyboardInterrupt:
    print("\n程序已停止。")

finally:
    if ser.is_open:
        ser.close()