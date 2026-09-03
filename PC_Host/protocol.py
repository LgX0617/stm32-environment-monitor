"""协议与下位机 App/Src/protocol.c 对应。"""

FRAME_HEAD = 0xAA
CMD_DATA, CMD_SET_THRESHOLD, CMD_ACK, CMD_ERROR = 0x01, 0x02, 0x03, 0x04
PARAM_TEMP_FAN_ON_MAX = 0x01
PARAM_TEMP_HIGH_ALARM_MAX = 0x02
PARAM_HUMI_HIGH_ALARM_MAX = 0x03
PARAM_LIGHT_DARK = 0x04
PAYLOAD_LENGTHS = {CMD_DATA: 17, CMD_ACK: 3, CMD_ERROR: 1}


def build_frame(command, payload):
    payload = bytes(payload)
    if command == CMD_SET_THRESHOLD and len(payload) != 3:
        raise ValueError("设置阈值命令的 payload 必须为 3 字节")
    checksum = (command + sum(payload)) & 0xFF
    return bytes((FRAME_HEAD, command)) + payload + bytes((checksum,))


def decode_signed16(high, low):
    value = (high << 8) | low
    return value - 0x10000 if value & 0x8000 else value


class FrameParser:
    def __init__(self):
        self.reset()

    def reset(self):
        self.state = 0
        self.command = 0
        self.expected = 0
        self.payload = bytearray()
        self.checksum = 0

    def feed(self, data):
        frames = []
        for byte in bytes(data):
            if self.state == 0:  # WAIT_HEAD
                if byte == FRAME_HEAD:
                    self.state = 1
            elif self.state == 1:  # GET_COMMAND
                length = PAYLOAD_LENGTHS.get(byte)
                if length is None:
                    self.reset()
                    if byte == FRAME_HEAD:
                        self.state = 1
                else:
                    self.command, self.expected = byte, length
                    self.payload.clear()
                    self.checksum = byte
                    self.state = 2
            elif self.state == 2:  # GET_PAYLOAD
                self.payload.append(byte)
                self.checksum = (self.checksum + byte) & 0xFF
                if len(self.payload) == self.expected:
                    self.state = 3
            else:  # GET_CHECKSUM
                if byte == self.checksum:
                    frames.append((self.command, bytes(self.payload)))
                self.reset()
        return frames
