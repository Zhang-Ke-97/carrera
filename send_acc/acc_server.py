import smbus                #import SMBus module of I2C     
import socket
from time import localtime, strftime, sleep

#some MPU6050 Registers and their Address
PWR_MGMT_1   = 0x6B
SMPLRT_DIV   = 0x19
CONFIG       = 0x1A
ACCEL_CONFIG = 0x1C
INT_ENABLE   = 0x38
ACCEL_XOUT_H = 0x3B
ACCEL_YOUT_H = 0x3D 
ACCEL_ZOUT_H = 0x3F

# Configure the registers in MPU6050
def MPU_Init():
    #write to sample rate register
    bus.write_byte_data(Device_Address, SMPLRT_DIV, 7)

    #Write to power management register
    bus.write_byte_data(Device_Address, PWR_MGMT_1, 1)

    #Write to Configuration register
    bus.write_byte_data(Device_Address, CONFIG, 0)

    #Write to Accel configuration register
    bus.write_byte_data(Device_Address, ACCEL_CONFIG, 0)

    #Write to interrupt enable register
    bus.write_byte_data(Device_Address, INT_ENABLE, 1)

# Read 16 bit raw data in MPU6050
def read_raw_accel(addr):
    #Accelero and Gyro value are 16-bit
    high = bus.read_byte_data(Device_Address, addr)
    low = bus.read_byte_data(Device_Address, addr+1)

    #concatenate higher and lower value
    value = ((high << 8) | low)

    #to get signed value from mpu6050
    if(value > 32768):
            value = value - 65536
    return value

# Set up server
def setup_server():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("sockt created")
    try:
        s.bind((host, port))
    except socket.error as msg:
        print(msg)
    print("socket bind complete")
    return s

# Set up transmission
def set_up_connection():
    s.listen(1) # allows one connection at a time
    conn, address = s.accept()
    print("connected to client @"+address[0]+":"+str(address[1]))
    return conn

# Helper, read the sensor data
def ACCEL():
    #Read Accelerometer raw value
    acc_x = read_raw_accel(ACCEL_XOUT_H)
    acc_y = read_raw_accel(ACCEL_YOUT_H)
    acc_z = read_raw_accel(ACCEL_ZOUT_H)

    # scale and round
    Ax = round(acc_x/16384.0, 3)
    Ay = round(acc_y/16384.0, 3)
    Az = round(acc_z/16384.0, 3)

    # pack all values into string
    reply = "\t".join([str(Ax), str(Ay), str(Az)])
    return reply

# Helper, read the current time
def TIME():
    reply = strftime("%Y-%m-%d %H:%M:%S", localtime())
    return reply

# A big loop that sends/receives data
def data_transfer(conn):
    while True:
        # received the data from the client
        data = conn.recv(1024) # buffer size: 1024
        data = data.decode('utf-8')

        # Split the data. The first word is the command from the client
        data_msg = data.split(' ', 1)
        command = data_msg[0]
        if command == 'TIME':
            reply = TIME()
        elif command == 'ACCEL':
            reply = ACCEL()
        else:
            reply = 'Unknown command'+command
        # Send the reply back to the client
        conn.sendall(str.encode(reply))
        print("Data has been sent: "+reply)
    conn.close()


######### MAIN PROGRAMM STARTS HERE #########
# Create bus and ready to read data
bus = smbus.SMBus(1) 	# or bus = smbus.SMBus(0) for older version boards
Device_Address = 0x68   # MPU6050 device address
MPU_Init()
print ("MPU6050 ready")

# Set up Socket connection
host = ''
port = 5560
s = setup_server()

# Transmitting
while True:
    try:
        conn = set_up_connection()
        data_transfer()
    except:
        break

