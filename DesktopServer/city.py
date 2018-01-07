# For Python2 compatibility
try:
    import Tkinter as tk
except ImportError: # For Python3
    import tkinter as tk
import time
from datetime import datetime
from config import *
from SocketServer import UDPServer, BaseRequestHandler

class App():
    def __init__(self):
        self.defTime = datetime(1970,1,1,0,4,0)

        self.root = tk.Tk() # Main window
        self.root.title("Robotraffic city timer")
        self.frame = tk.Frame(self.root) # TODO: Add black background
        self.frame.pack(fill='x')

        self.label = tk.Label(self.frame, text=self.defTime.strftime("%M:%S.%f")[:-tail], font=('DSEG7Classic-Italic', 240), 
         fg='blue')
        self.label.pack()

        # Buttons
        self.startButton = tk.Button(self.frame, text='START', command=self.start)
        self.startButton.pack(side='left')

        self.stopButton = tk.Button(self.frame, text='STOP', command=self.stop)
        self.stopButton.pack(side='left')

        self.resetButton = tk.Button(self.frame, text='RESET', command=self.reset)
        self.resetButton.pack(side='left')

        self.isStarted = 0
        self.startTime = time.time()

    def start(self):
        self.isStarted = 1
        self.startTime = time.time()
        self.update_clock()

    def stop(self):
        self.isStarted = 0

    def reset(self):
        self.isStarted = 0
        self.label.configure(text=self.defTime.strftime("%M:%S.%f")[:-tail])

    def update_clock(self):
        if self.isStarted:
            elapsedTime = datetime.utcfromtimestamp(time.time() - self.startTime)
            remainingTime = self.defTime - elapsedTime
            if remainingTime.total_seconds() > 0:
                self.label.configure(text=str(remainingTime)[2:-tail])
            else:
                self.label.configure(text="00:00.00")
                self.stop()
            self.root.after(10, self.update_clock)
            print(time.time())

class UDPHandler(BaseRequestHandler):
    def handle(self):
        data = self.request[0].strip()
        socket = self.request[1]
        print("{}:{} wrote:".format(self.client_address[0], self.client_address[1]))
        print(data)
        socket.sendto(data.upper(), self.client_address)

app=App()
app.root.mainloop()
