# Use only with Python 3
from socketserver import UDPServer, BaseRequestHandler, ThreadingMixIn
import socket
import threading
import ipaddress

class ThreadedUDPHandler(BaseRequestHandler):
    def __init__(self, callback, *args, **keys): # Need a Factory to pass this class with a callback in UDPServer instance
        self.callback = callback
        BaseRequestHandler.__init__(self, *args, **keys)

    def handle(self):
        data = str(self.request[0][:100], 'ascii')
        socket = self.request[1]
        print("[Server]: {}:{} wrote: {}".format(self.client_address[0], self.client_address[1], data))
        self.callback(data)


class ThreadedUDPServer(ThreadingMixIn, UDPServer):
    pass


def sendUDPmsg(ip, port, message):
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        # Gain permission to broadcast
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        print("[Client]: Sending: {}".format(message))
        sock.sendto(bytes(message, 'ascii'), (ip, port))


def handlerFactoryMethod(callback):
    def createHandler(*args, **keys):
        return ThreadedUDPHandler(callback, *args, **keys)
    return createHandler


if __name__ == "__main__":
    PORT = 4210
    HOST = ''
    # the public network interface
    localIP = socket.gethostbyname(socket.gethostname())

    def test(data):
        print(data)

    server = ThreadedUDPServer((HOST, PORT), handlerFactoryMethod(test))

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
