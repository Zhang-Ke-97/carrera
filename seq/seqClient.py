import socket

host = '172.20.10.100'
port = 5560

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))

command = 'COUNT'
while True:
    s.send(str.encode(command))
    reply = s.recv(1024)
    print(reply.decode('utf-8'))

s.close()
