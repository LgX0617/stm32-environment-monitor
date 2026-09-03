import queue
import threading
import time

import serial

from protocol import FrameParser


class SerialWorker:
    def __init__(self):
        self.serial = None
        self.events = queue.Queue()
        self.parser = FrameParser()
        self.lock = threading.Lock()
        self.thread = None
        self.stop_event = threading.Event()

    @property
    def is_open(self):
        return self.serial is not None and self.serial.is_open

    def open(self, port):
        if self.is_open:
            return

        self.serial = serial.Serial(
            port=None,
            baudrate=115200,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.2,
            xonxoff=False,
            rtscts=False,
            dsrdtr=False,
        )
        # Configure modem-control lines before opening.  Some USB-UART adapters
        # reset the STM32 when DTR changes during Serial.open().
        self.serial.dtr = False
        self.serial.rts = False
        self.serial.port = port
        self.serial.open()
        self.serial.reset_input_buffer()
        self.serial.reset_output_buffer()
        # If DTR is wired to the board's reset circuit, perform one controlled
        # reset so the firmware starts its UART DMA reception from a clean state.
        # Adapters without DTR wiring are unaffected.
        try:
            self.serial.dtr = True
            time.sleep(0.05)
            self.serial.dtr = False
            time.sleep(0.15)
        except serial.SerialException:
            pass
        self.parser.reset()
        self.stop_event.clear()
        self.thread = threading.Thread(target=self._receive_loop, daemon=True)
        self.thread.start()

    def close(self):
        self.stop_event.set()
        ser = self.serial
        if ser is not None:
            ser.close()
        self.serial = None
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=0.5)
        self.thread = None

    def send(self, frame):
        if not self.is_open:
            raise serial.SerialException("串口尚未打开")
        with self.lock:
            self.serial.write(frame)

    def _receive_loop(self):
        while not self.stop_event.is_set():
            ser = self.serial
            if ser is None or not ser.is_open:
                break
            raw = ser.read(64)
            if not raw:
                continue
            try:
                frames = self.parser.feed(raw)
            except Exception as error:
                self.events.put(("error", f"协议解析异常: {error}"))
                continue
            for command, payload in frames:
                self.events.put(("frame", command, payload))
