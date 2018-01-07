# For Python2 compatibility
try:
    import Tkinter as tk
except ImportError: # For Python3
    import tkinter as tk
import time

class App():
    def __init__(self):
        self.defLabel = "00:00.00"

        self.root = tk.Tk() # Main window
        self.root.title("Robotraffic city timer")
        self.frame = tk.Frame(self.root) # TODO: Add black background
        self.frame.pack(fill='x')

        self.label = tk.Label(self.frame, text=self.defLabel, font=('DSEG7Classic-Italic', 240), 
         fg='blue')
        self.label.pack(fill='x')

        self.startButton = tk.Button(self.frame, text='START', command=self.start)
        self.startButton.pack(side='left')

        self.stopButton = tk.Button(self.frame, text='STOP', command=self.stop)
        self.stopButton.pack(side='left')

        self.resetButton = tk.Button(self.frame, text='RESET', command=self.reset)
        self.resetButton.pack(side='left')

        self.isStarted = 0

    def start(self):
        self.isStarted = 1
        self.update_clock()

    def stop(self):
        self.isStarted = 0

    def reset(self):
        self.isStarted = 0
        self.label.configure(text=self.defLabel)

    def update_clock(self):
        if self.isStarted:
            ms = int((time.time() % 1) * 100)
            now = time.strftime("%M:%S.") + str(ms) if ms > 9 else time.strftime("%M:%S.0") + str(ms) # Fix jiggling
            self.label.configure(text=now)
            self.root.after(10, self.update_clock)
            print(time.time())

app=App()
app.root.mainloop()
