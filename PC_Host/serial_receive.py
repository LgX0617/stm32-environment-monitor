import serial
import threading

FRAME_HEAD = 0xAA

CMD_DATA = 0x01
CMD_SET_THRESHOLD = 0x02
CMD_ACK = 0x03
CMD_ERROR = 0x04

PARAM_TEMP_MAX = 0x01
PARAM_HUMI_MAX = 0x02
PARAM_LIGHT_DARK = 0x03

RX_WAIT_HEAD = 0
RX_GET_CMD = 1
RX_GET_DATA = 2
RX_GET_CHECK = 3

PAYLOAD_LENGTHS = {
    CMD_DATA: 7,
    CMD_ACK: 3,
    CMD_ERROR: 1,
}


def build_frame(cmd, payload):
    checksum = (cmd + sum(payload)) & 0xFF
    return bytes([FRAME_HEAD, cmd, *payload, checksum])


def handle_frame(cmd, payload):
    if cmd == CMD_DATA:
        temperature_x10 = (payload[0] << 8) | payload[1]
        if temperature_x10 & 0x8000:
            temperature_x10 -= 0x10000

        humidity_x10 = (payload[2] << 8) | payload[3]
        light = (payload[4] << 8) | payload[5]
        alarm = payload[6]

        print(
            f"\n温度：{temperature_x10 / 10:.1f} C，"
            f"湿度：{humidity_x10 / 10:.1f} %，"
            f"光照 ADC：{light}，报警：{alarm}"
        )

    elif cmd == CMD_ACK:
        parameter_id = payload[0]
        value = (payload[1] << 8) | payload[2]
        print(f"\nSTM32 确认：参数 {parameter_id} 已设置为 {value}")

    elif cmd == CMD_ERROR:
        error_code = payload[0]
        if error_code == 0x01:
            print("\nSTM32 拒绝命令：参数编号错误")
        elif error_code == 0x02:
            print("\nSTM32 拒绝命令：数值超出范围")
        else:
            print(f"\nSTM32 拒绝命令：未知错误码 {error_code}")


def receive_loop(ser):
    state = RX_WAIT_HEAD
    cmd = 0
    payload = bytearray()
    expected_length = 0
    checksum = 0

    while ser.is_open:
        raw = ser.read(1)
        if not raw:
            continue

        byte = raw[0]

        if state == RX_WAIT_HEAD:
            if byte == FRAME_HEAD:
                state = RX_GET_CMD

        elif state == RX_GET_CMD:
            if byte in PAYLOAD_LENGTHS:
                cmd = byte
                expected_length = PAYLOAD_LENGTHS[cmd]
                payload.clear()
                checksum = byte
                state = RX_GET_DATA
            else:
                state = RX_WAIT_HEAD

        elif state == RX_GET_DATA:
            payload.append(byte)
            checksum = (checksum + byte) & 0xFF

            if len(payload) >= expected_length:
                state = RX_GET_CHECK

        elif state == RX_GET_CHECK:
            if byte == checksum:
                handle_frame(cmd, payload)
            else:
                print(
                    f"\n校验错误：收到 {byte:02X}，"
                    f"期望 {checksum:02X}"
                )

            state = RX_WAIT_HEAD


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

# Open with modem-control lines disabled to avoid affecting the board reset state.
ser.dtr = False
ser.rts = False

try:
    ser.open()

    print(f"已打开：{ser.name}")
    print(f"DTR={ser.dtr}, RTS={ser.rts}")
    print("等待二进制数据，按 Ctrl+C 停止。")

    receiver_thread = threading.Thread(
        target=receive_loop,
        args=(ser,),
        daemon=True,
    )
    receiver_thread.start()

    while True:
        user_input = input("\n输入过暗阈值 0-4095，输入 q 退出：").strip()

        if user_input.lower() == "q":
            break

        if not user_input.isdigit():
            print("请输入 0 到 4095 的整数。")
            continue

        threshold = int(user_input)
        if threshold > 4095:
            print("阈值不能大于 4095。")
            continue

        payload = [
            PARAM_LIGHT_DARK,
            (threshold >> 8) & 0xFF,
            threshold & 0xFF,
        ]
        command = build_frame(CMD_SET_THRESHOLD, payload)
        bytes_sent = ser.write(command)

        print(f"已发送 {bytes_sent} 字节：{command.hex(' ').upper()}")

except serial.SerialException as error:
    print("串口错误：", error)

except KeyboardInterrupt:
    print("\n程序已停止。")

finally:
    if ser.is_open:
        ser.close()
