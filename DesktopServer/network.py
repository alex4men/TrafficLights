# Use only with Python 3
from socketserver import UDPServer, BaseRequestHandler, ThreadingMixIn

import socket
import threading
import ipaddress

class ThreadedUDPHandler(BaseRequestHandler):
    def handle(self):
        data = str(self.request[0][:100], 'ascii')
        socket = self.request[1]
        print("[Server]: {}:{} wrote: {}".format(self.client_address[0], self.client_address[1], data))

        # cur_thread = threading.current_thread()
        # response = bytes("{}: {}".format(cur_thread.name, data), 'ascii')
        # socket.sendto(response.upper(), self.client_address)

class ThreadedUDPServer(ThreadingMixIn, UDPServer):
    pass

def sendUDPmsg(ip, port, message):
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        # sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # delete
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        print("[Client]: Sending: {}".format(message))
        sock.sendto(bytes(message, 'ascii'), (ip, port))

        response = str(sock.recv(1024), 'ascii')
        print("[Client]: Received: {}".format(response))


if __name__ == "__main__":
    PORT = 4210
    HOST = ''
    # the public network interface
    localIP = socket.gethostbyname(socket.gethostname())
    server = ThreadedUDPServer((HOST, PORT), ThreadedUDPHandler)

    serverIP, port = server.server_address
    broadcastIP = str(ipaddress.ip_interface(localIP+'/24').network[-1])

    # Start a thread with the server -- that thread will then start one
    # more thread for each request
    server_thread = threading.Thread(target=server.serve_forever)
    # Exit the server thread when the main thread terminates
    server_thread.daemon = True
    server_thread.start()
    print("Server loop running in thread:", server_thread.name, serverIP, port)

    
    sendUDPmsg(broadcastIP, port, "S")
    # client(ip, port, "Hello World 2")
    # client(ip, port, "Hello World 3")