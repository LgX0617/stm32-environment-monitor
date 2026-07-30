import tkinter as tk
from decimal import Decimal, InvalidOperation
from tkinter import messagebox, ttk

import serial
from serial.tools import list_ports

from protocol import (
    CMD_ACK,
    CMD_DATA,
    CMD_ERROR,
    CMD_SET_THRESHOLD,
    PARAM_HUMI_MAX,
    PARAM_LIGHT_DARK,
    PARAM_TEMP_MAX,
    build_frame,
    decode_signed16,
)
from serial_worker import SerialWorker


def scaled_value(text, minimum, maximum):
    value = Decimal(text)
    if value < Decimal(str(minimum)) or value > Decimal(str(maximum)):
        raise ValueError
    return int(value * 10)


class MonitorApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("STM32 环境监测终端")
        self.geometry("720x520")
        self.minsize(620, 450)

        self.worker = SerialWorker()
        self.data_vars = {
            "temperature": tk.StringVar(value="--.- C"),
            "humidity": tk.StringVar(value="--.- %"),
            "light": tk.StringVar(value="---- ADC"),
            "alarm": tk.StringVar(value="未知"),
        }
        self.threshold_vars = {
            PARAM_TEMP_MAX: tk.StringVar(value="--.- C"),
            PARAM_HUMI_MAX: tk.StringVar(value="--.- %"),
            PARAM_LIGHT_DARK: tk.StringVar(value="---- ADC"),
        }
        self.entries = {}
        self.port_var = tk.StringVar()
        self.status_var = tk.StringVar(value="未连接")
        self._build_ui()
        self.refresh_ports()
        self.after(100, self.poll_events)
        self.protocol("WM_DELETE_WINDOW", self.on_close)

    def _build_ui(self):
        root = ttk.Frame(self, padding=14)
        root.pack(fill="both", expand=True)

        connection = ttk.LabelFrame(root, text="串口连接", padding=10)
        connection.pack(fill="x")
        ttk.Label(connection, text="端口").pack(side="left")
        self.port_box = ttk.Combobox(
            connection, textvariable=self.port_var, state="readonly", width=16
        )
        self.port_box.pack(side="left", padx=(6, 10))
        ttk.Button(connection, text="刷新", command=self.refresh_ports).pack(side="left")
        self.open_button = ttk.Button(
            connection, text="打开串口", command=self.toggle_serial
        )
        self.open_button.pack(side="left", padx=10)
        ttk.Label(connection, textvariable=self.status_var).pack(side="left")

        realtime = ttk.LabelFrame(root, text="实时数据", padding=10)
        realtime.pack(fill="x", pady=(12, 0))
        for column, (key, title) in enumerate(
            (("temperature", "温度"), ("humidity", "湿度"), ("light", "光照"), ("alarm", "报警"))
        ):
            panel = ttk.Frame(realtime, padding=8)
            panel.grid(row=0, column=column, sticky="nsew")
            realtime.columnconfigure(column, weight=1)
            ttk.Label(panel, text=title).pack()
            ttk.Label(panel, textvariable=self.data_vars[key], font=("Segoe UI", 16)).pack(pady=(8, 0))

        thresholds = ttk.LabelFrame(root, text="阈值设置", padding=10)
        thresholds.pack(fill="x", pady=(12, 0))
        rows = (
            (PARAM_TEMP_MAX, "温度上限", "-40.0 到 85.0 C"),
            (PARAM_HUMI_MAX, "湿度上限", "0.0 到 100.0 %"),
            (PARAM_LIGHT_DARK, "光照过暗", "0 到 4095 ADC"),
        )
        for row, (parameter, title, hint) in enumerate(rows):
            ttk.Label(thresholds, text=title, width=12).grid(row=row, column=0, sticky="w", pady=4)
            ttk.Label(thresholds, textvariable=self.threshold_vars[parameter], width=14).grid(
                row=row, column=1, sticky="w", padx=8
            )
            entry = ttk.Entry(thresholds, width=14)
            entry.grid(row=row, column=2, sticky="w")
            self.entries[parameter] = entry
            ttk.Button(
                thresholds,
                text="设置",
                command=lambda p=parameter: self.set_threshold(p),
            ).grid(row=row, column=3, padx=8)
            ttk.Label(thresholds, text=hint).grid(row=row, column=4, sticky="w")

        log_frame = ttk.LabelFrame(root, text="通信日志", padding=8)
        log_frame.pack(fill="both", expand=True, pady=(12, 0))
        self.log_text = tk.Text(log_frame, height=7, state="disabled", wrap="word")
        self.log_text.pack(side="left", fill="both", expand=True)
        scrollbar = ttk.Scrollbar(log_frame, command=self.log_text.yview)
        scrollbar.pack(side="right", fill="y")
        self.log_text.configure(yscrollcommand=scrollbar.set)

    def refresh_ports(self):
        ports = [port.device for port in list_ports.comports()]
        self.port_box["values"] = ports
        if ports and self.port_var.get() not in ports:
            self.port_var.set(ports[0])

    def toggle_serial(self):
        if self.worker.is_open:
            self.worker.close()
            self.open_button.configure(text="打开串口")
            self.status_var.set("未连接")
            self.log("串口已关闭")
            return

        if not self.port_var.get():
            messagebox.showwarning("提示", "请先选择串口")
            return

        try:
            self.worker.open(self.port_var.get())
        except serial.SerialException as error:
            messagebox.showerror("串口错误", str(error))
            return

        self.open_button.configure(text="关闭串口")
        self.status_var.set("已连接")
        self.log(f"已打开 {self.port_var.get()}，115200 8N1")

    def set_threshold(self, parameter):
        text = self.entries[parameter].get().strip()
        try:
            if parameter == PARAM_TEMP_MAX:
                raw = scaled_value(text, -40, 85)
            elif parameter == PARAM_HUMI_MAX:
                raw = scaled_value(text, 0, 100)
            else:
                raw = int(text)
                if raw < 0 or raw > 4095:
                    raise ValueError
        except (ValueError, InvalidOperation):
            messagebox.showwarning("输入错误", "请输入有效范围内的数值")
            return

        raw16 = raw & 0xFFFF
        payload = [parameter, (raw16 >> 8) & 0xFF, raw16 & 0xFF]
        try:
            frame = build_frame(CMD_SET_THRESHOLD, payload)
            self.worker.send(frame)
        except serial.SerialException as error:
            messagebox.showerror("串口错误", str(error))
            return

        self.log(f"发送设置：{frame.hex(' ').upper()}")

    def poll_events(self):
        try:
            while True:
                event = self.worker.events.get_nowait()
                if event[0] == "frame":
                    self.handle_frame(event[1], event[2])
        except Exception:
            pass
        self.after(100, self.poll_events)

    def handle_frame(self, command, payload):
        if command == CMD_DATA:
            temperature = decode_signed16(payload[0], payload[1])
            humidity = (payload[2] << 8) | payload[3]
            light = (payload[4] << 8) | payload[5]
            alarm = payload[6]
            temp_limit = decode_signed16(payload[7], payload[8])
            humidity_limit = (payload[9] << 8) | payload[10]
            light_limit = (payload[11] << 8) | payload[12]

            self.data_vars["temperature"].set(f"{temperature / 10:.1f} C")
            self.data_vars["humidity"].set(f"{humidity / 10:.1f} %")
            self.data_vars["light"].set(f"{light} ADC")
            self.data_vars["alarm"].set("报警" if alarm else "正常")
            self.threshold_vars[PARAM_TEMP_MAX].set(f"{temp_limit / 10:.1f} C")
            self.threshold_vars[PARAM_HUMI_MAX].set(f"{humidity_limit / 10:.1f} %")
            self.threshold_vars[PARAM_LIGHT_DARK].set(f"{light_limit} ADC")

        elif command == CMD_ACK:
            parameter = payload[0]
            if parameter == PARAM_TEMP_MAX:
                value = decode_signed16(payload[1], payload[2]) / 10
            elif parameter == PARAM_HUMI_MAX:
                value = ((payload[1] << 8) | payload[2]) / 10
            else:
                value = (payload[1] << 8) | payload[2]
            self.log(f"收到 ACK：参数 {parameter} 已设置为 {value}")

        elif command == CMD_ERROR:
            errors = {1: "参数编号错误", 2: "数值超出范围"}
            self.log(f"收到 ERROR：{errors.get(payload[0], '未知错误')}")

    def log(self, message):
        self.log_text.configure(state="normal")
        self.log_text.insert("end", message + "\n")
        self.log_text.see("end")
        self.log_text.configure(state="disabled")

    def on_close(self):
        self.worker.close()
        self.destroy()


if __name__ == "__main__":
    app = MonitorApp()
    app.mainloop()
