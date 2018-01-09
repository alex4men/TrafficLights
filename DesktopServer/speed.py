# Use only with Python 3
import tkinter as tk
import time
from datetime import datetime
from config import *
from network import *


class App():
    def __init__(self):
        self.defTime = datetime(1970,1,1,0,0,0)

        self.root = tk.Tk() # Main window
        self.root.title("Robotraffic speed timer")
        self.frame = tk.Frame(self.root) # TODO: Add black background
        self.frame.pack(fill='x')

        self.labelA = tk.Label(self.frame, text=self.defTime.strftime("%M:%S.%f")[:-tail], font=('DSEG7Classic-Italic', 240), 
         fg='blue')
        self.labelA.pack()

        self.labelB = tk.Label(self.frame, text=self.defTime.strftime("%M:%S.%f")[:-tail], font=('DSEG7Classic-Italic', 240), 
         fg='red')
        self.labelB.pack()

        # Buttons
        self.startButton = tk.Button(self.frame, text='START', command=self.start)
        self.startButton.pack(side='left')

        self.stopButton = tk.Button(self.frame, text='STOP', command=self.stop)
        self.stopButton.pack(side='left')

        self.resetButton = tk.Button(self.frame, text='RESET', command=self.reset)
        self.resetButton.pack(side='left')

        self.isStarted = False
        self.isFinishedA = False
        self.isFinishedB = False
        self.startTime = time.time()

    def start(self):
        self.isStarted = True
        self.isFinishedA = False
        self.isFinishedB = False
        self.startTime = time.time()
        self.update_clock()
        try:
            sendUDPmsg(broadcastIP, port, startCmd)
        except:
            pass

    def stop(self):
        self.isStarted = False
        try:
            sendUDPmsg(broadcastIP, port, stopCmd)
        except:
            pass

    def reset(self):
        self.isStarted = False
        self.labelA.configure(text=self.defTime.strftime("%M:%S.%f")[:-tail])
        self.labelB.configure(text=self.defTime.strftime("%M:%S.%f")[:-tail])
        try:
            sendUDPmsg(broadcastIP, port, resetCmd)
        except:
            pass

    def msgCallbackHandler(self, data):
        if data == aFinished:
            self.isFinishedA = True
        elif data == bFinished:
            self.isFinishedB = True
        print("[App]: " + data)

    def update_clock(self):
        if self.isStarted and (not self.isFinishedA or not self.isFinishedB):
            elapsedTime = datetime.utcfromtimestamp(time.time() - self.startTime)
            if not self.isFinishedA:
                self.labelA.configure(text=elapsedTime.strftime("%M:%S.%f")[:-tail])
            if not self.isFinishedB:
                self.labelB.configure(text=elapsedTime.strftime("%M:%S.%f")[:-tail])
            self.root.after(10, self.update_clock) # Add function call in schedule queue

if __name__ == "__main__":

    app=App()

    # the public network interface
    localIP = socket.gethostbyname(socket.gethostname())
    server = ThreadedUDPServer((localIP, port), handlerFactoryMethod(app.msgCallbackHandler))

    serverIP, port = server.server_address
    broadcastIP = str(ipaddress.ip_interface(localIP+'/24').network[-1])

    # Start a thread with the server -- that thread will then start one
    # more thread for each request
    server_thread = threading.Thread(target=server.serve_forever)
    # Exit the server thread when the main thread terminates
    server_thread.daemon = True
    server_thread.start()
    print("Server loop running in thread:", server_thread.name, serverIP, port)


    app.root.mainloop()
