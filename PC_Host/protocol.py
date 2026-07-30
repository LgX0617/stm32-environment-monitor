FRAME_HEAD = 0xAA

CMD_DATA = 0x01
CMD_SET_THRESHOLD = 0x02
CMD_ACK = 0x03
CMD_ERROR = 0x04

PARAM_TEMP_MAX = 0x01
PARAM_HUMI_MAX = 0x02
PARAM_LIGHT_DARK = 0x03

PAYLOAD_LENGTHS = {
    CMD_DATA: 13,
    CMD_ACK: 3,
    CMD_ERROR: 1,
}


def build_frame(command, payload):
    payload = bytes(payload)
    checksum = (command + sum(payload)) & 0xFF
    return bytes([FRAME_HEAD, command]) + payload + bytes([checksum])


def decode_signed16(high, low):
    value = (high << 8) | low
    if value & 0x8000:
        value -= 0x10000
    return value


class FrameParser:
    WAIT_HEAD = 0
    GET_COMMAND = 1
    GET_PAYLOAD = 2
    GET_CHECKSUM = 3

    def __init__(self):
        self.reset()

    def reset(self):
        self.state = self.WAIT_HEAD
        self.command = 0
        self.payload = bytearray()
        self.expected_length = 0
        self.checksum = 0

    def feed(self, data):
        frames = []

        for byte in data:
            if self.state == self.WAIT_HEAD:
                if byte == FRAME_HEAD:
                    self.state = self.GET_COMMAND

            elif self.state == self.GET_COMMAND:
                if byte in PAYLOAD_LENGTHS:
                    self.command = byte
                    self.expected_length = PAYLOAD_LENGTHS[byte]
                    self.payload.clear()
                    self.checksum = byte
                    self.state = self.GET_PAYLOAD
                else:
                    self.reset()

            elif self.state == self.GET_PAYLOAD:
                self.payload.append(byte)
                self.checksum = (self.checksum + byte) & 0xFF
                if len(self.payload) == self.expected_length:
                    self.state = self.GET_CHECKSUM

            elif self.state == self.GET_CHECKSUM:
                if byte == self.checksum:
                    frames.append((self.command, bytes(self.payload)))
                self.reset()

        return frames
