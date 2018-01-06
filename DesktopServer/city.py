# For Python2 compatibility
try:
    import Tkinter as tk
except ImportError: # For Python3
    import tkinter as tk
import time

class App():

    def __init__(self):
        self.root = tk.Tk() # Main window

        self.label = tk.Label(self.root, text="", font=('DSEG7Classic-Italic', 40), bg='black', 
         fg='blue')
        self.label.pack(fill='x')
        self.update_clock()
        self.root.mainloop()

    def __exit__(self):
        self.root.destroy() # TODO: properly handle exit

    def update_clock(self):
        now = time.strftime("%H:%M:%S")
        self.label.configure(text=now)
        self.root.after(10, self.update_clock)

app=App()
