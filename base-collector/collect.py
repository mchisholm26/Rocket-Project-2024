from time import sleep
from rich.console import Console
from rich.live import Live
from rich.layout import Layout
from rich.panel import Panel
from rich.table import Table
from serial import Serial

console = Console()

# class Serial: # for mocking
#     def __init__(self, *args, **kwargs):
#         pass

#     def readline(self):
#         from random import randint
#         sleep(.25)
#         msg = f"{randint(1, 9)}" + ", ".join([f"{randint(1, 9)}" for _ in range(8)])
#         return msg.encode("utf-8")

arduino = Serial(port="/dev/cu.usbmodem151894701", baudrate=9600)

def status_to_str(status):
    if status == 0:
        return "[purple]Unknown"
    elif status == 1:
        return "[bold green]Okay"
    elif status == 2:
        return "[red]Error"
    else:
        return "[yellow]Unknown"

class ConsoleUI:
    def __init__(self):
        self.__data_csv = open("data.csv", "w")
        self.__logfile = open("log.txt", "w")
        # rocket.ino, line 47
        self.__data_csv.write("DeltaTime,AccelAge,AccelX,AccelY,AccelZ,BaroAge,Altitude,Pressure,Temperature,BnoAge,BnoAccelX,BnoAccelY,BnoAccelZ,BnoOrientationW,BnoOrientationX,BnoOrientationY,BnoOrientationZ,BnoAngularX,BnoAngularY,BnoAngularZ,BnoSysHealth,BnoGyroHealth,BnoAccelHealth\n")
        self.messages = 0
        self.status = {
            "base": 0,
            "rocket": 0
            # 0 = unknown (purple), 1 = okay (green), 2 = error (red)
        }
        self.stats = {
            "temperature": 0,
            "altitude": 0,
            "rssi": 0
        }

    def render_panels(self):
        status_table = Table(title="", show_header=False, show_edge=False, expand=True)
        status_table.add_column("Component")
        status_table.add_column("Status")
        status_table.add_row("Base", f"{status_to_str(self.status['base'])}")
        status_table.add_row("Rocket", f"{status_to_str(self.status['rocket'])}")
        self.__status_panel = Panel(status_table, title="Status", border_style="green")

        stats_table = Table(title="", show_header=False, show_edge=False, expand=True)
        stats_table.add_column("Stat")
        stats_table.add_column("Value")
        stats_table.add_row("Temperature", f"{self.stats['temperature']} °C")
        stats_table.add_row("Temperature (American)", f"{self.stats['temperature'] * 9/5 + 32} °F")
        stats_table.add_row("Altitude", f"{self.stats['altitude']} m")
        stats_table.add_row("RSSI", f"{self.stats['rssi']} dBm")
        stats_table.add_row("Messages", f"{self.messages}")
        self.__stats_panel = Panel(stats_table, title="Stats", border_style="blue")
        
        self.__live_data_panel = Panel("Live Data: [bold yellow]Some live data", title="Live Data", border_style="yellow")

    def render(self):
        l = Layout()
        l.split_column(Layout(name="top"), Layout(name="bottom"))
        l["top"].split_row(Layout(name="status"), Layout(name="stats"))
        l["bottom"].split_row(Layout(name="live_data"))

        self.render_panels()

        l["status"].update(self.__status_panel)
        l["stats"].update(self.__stats_panel)
        l["live_data"].update(self.__live_data_panel)

        return l
    
    def update_data(self, message):
        self.__logfile.write(message + "\n")
        self.messages += 1
        try:
            if message.startswith("rssi"):
                _, b = message.split(": ")
                self.stats["rssi"] = int(b)
            elif message.startswith("ok"):
                self.status["base"] = 1
            elif message.startswith("error"):
                self.status["base"] = 2
            elif message.startswith("message"):
                _, type, contents = message.split(": ")
                if type.startswith("d:"):
                    self.stats["temperature"] = contents
                    self.__data_csv.write(contents + "\n")
                elif type.startswith("ok"):
                    self.status["rocket"] = 1
                elif type.startswith("error"):
                    self.status["rocket"] = 2
        except:
            pass

    def __rich_console__(self, console, options):
        yield self.render()
    
    def close(self):
        self.__data_csv.close()
        self.__logfile.close()

def main():
    ui = ConsoleUI()
    try:
        with Live(ui, console=console, screen=True):
            while True:
                msg = arduino.readline().decode("utf-8").strip()
                ui.update_data(msg)
    finally:
        ui.close()

if __name__ == "__main__":
    main()
