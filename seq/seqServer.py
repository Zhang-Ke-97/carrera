import socket
from time import localtime, strftime, sleep

host = ''
port = 5560

storedValue = "data stored in server"
counter = 0.01

def setupServer():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("sockt created")
    try:
        s.bind((host, port))
    except socket.error as msg:
        print(msg)
    print("socket bind complete")
    return s

def setupConnection():
    s.listen(1) # allows one connection at a time
    conn, address = s.accept()
    print("connected to client @"+address[0]+":"+str(address[1]))
    return conn

def GET():
    reply = storedValue
    return reply


def TIME():
    reply = strftime("%Y-%m-%d %H:%M:%S", localtime())
    return reply


def REPEAT(dataMessage):
    reply = dataMessage[1]
    return reply

def COUNT():
    reply = str(counter)
    return reply

def dataTransfer(conn):
    # a big loop that sends/receives data until told not to
    while True:
        # receive data
        data = conn.recv(1024) # buffer size: 1024
        data = data.decode('utf-8')
        # Split the data such that we separate the command
        # from the rest of the data
        dataMessage = data.split(' ', 1)
        command = dataMessage[0]
        if command == 'GET':
            reply = GET()
        elif command == 'TIME':
            reply = TIME()
        elif command == 'COUNT':
            reply = COUNT()
        elif command == 'REPEAT':
            reply = REPEAT(dataMessage)
        elif command == 'EXIT':
            print("our client has left us :(")
            break
        elif command == 'KILL':
            print("our server is shutting down")
            s.close()
            break
        else:
            reply = 'Unknown Command: '+command
        # Send the reply back to the client
        conn.sendall(str.encode(reply))
        print("Data has been sent: "+reply)
    conn.close()

s = setupServer()

while True:
    try:
        conn = setupConnection()
        dataTransfer(conn)
    except:
        break





















