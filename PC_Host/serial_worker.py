import queue
import threading

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
            port=port,
            baudrate=115200,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.2,
            xonxoff=False,
            rtscts=False,
            dsrdtr=False,
        )
        self.serial.dtr = False
        self.serial.rts = False
        self.parser.reset()
        self.stop_event.clear()
        self.thread = threading.Thread(target=self._receive_loop, daemon=True)
        self.thread.start()

    def close(self):
        self.stop_event.set()
        if self.serial is not None:
            self.serial.close()
        self.serial = None

    def send(self, frame):
        if not self.is_open:
            raise serial.SerialException("串口尚未打开")
        with self.lock:
            self.serial.write(frame)

    def _receive_loop(self):
        while not self.stop_event.is_set() and self.is_open:
            raw = self.serial.read(1)
            if not raw:
                continue

            for command, payload in self.parser.feed(raw):
                self.events.put(("frame", command, payload))
