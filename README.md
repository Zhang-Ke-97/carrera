# carrera
In this project, we implemented the data transfer between raspberry pi 0 and raspberry pi 4b. The raspberry pi 0 reads acceleration data from the accelermeter unit MPU6050 and send them to pi 4b via WLAN. Pi 4b receives acceleration data and feeds them into a Kalman filter to produce velocity and mileage estimates.

All software-relating setups are already done on both pi. You can jump to hardware conncetion sections if you didn't reset the system files on both Pi.

## Libraries needed
Install `Eigen` on Pi 4b by running
```
sudo apt-get install libeigen3-dev
```
Install `pigpio` library on Pi 4b by running
```
wget https://github.com/joan2937/pigpio/archive/master.zip
unzip master.zip
cd pigpio-master
make
sudo make install
```
A full reference to `pigpio` library can be found at http://abyz.me.uk/rpi/pigpio/cif.html#gpioSetISRFunc.

Install i2c library on Pi 0 by running 
```
sudo apt-get install libi2c-dev
```
    
## Set up IP on both Pi
Make sure that both Pi are connected to Wifi `socket`. Then, follow the next steps to set static IP on both Pi.
1. check the IP address of the router by running

    ```
    ifconfig
    ```
    
2. check the DNS server of the router by opening `resolv.conf`

    ```
    sudo nano /etc/resolv.conf
    ```
    
5. open the file `dhcpcd.conf` by running 

    ```
    sudo nano /etc/dhcpcd.conf
    ```
7. add the lines in the `dhcpcd.conf`

      ```
      interface <NETWORK>
      static ip_address=<STATIC_IP>
      static routers=<ROUTER_IP>
      static domain_name_servers=<DNS_IP>
      ```
      where `<NETWORK>` can be either `wlan0` or `eth0`. 
      `<STATIC_IP>` is the IP we want to set for the Pi.
      `<ROUTER_IP>` follows from the step 1.
      `<DNS_IP>` follows from the step 2 or can be set manually, e.g. Google DNS `8.8.8.8`.
       
## Connect MPU6050 to Pi 0
The connection between MPU6050 and Pi 0 is shown as follows
| MPU6050       | Pi 0           | description |
| :------------ |:---------------:| -----:|
| VCC           | 4                 | suppply voltage |
| GND           | 30                |   ground |
| SCL           | 5 (GPIO 3)        |    serial clock |
| SDA           | 3 (GPIO 2)        | data line|

Orientation: y-direction should be tangential to the track. i.e. the direction of the car.
Activate i2c-bus on server Pi

## Connect light sensor to Pi 4b
The characteristics of the light sensors is shown as bellow
| Output       | Lighting condition            | interpretation |
| :------------ |:---------------:| -----:|
| High           | dark (actual light < threshold)                 | car arrived |
| Low           | light (actual light > threshold)                |   no car |

One can tune the sensitivity of the light sensors by screwing the nut on it. 

The connection between the first light sensor and Pi 4b is shown as follows
| Light sensor       | Pi 4b            | description |
| :------------ |:---------------:| -----:|
| VCC           | 4                 | suppply voltage |
| GND           | 30                |   ground |
| DO           | 8 (GPIO 14)        |    digital output |

The connection between the second light sensor and Pi 4b is shown as follows
| Light sensor       | Pi 4b            | description |
| :------------ |:---------------:| -----:|
| VCC           | 4                 | suppply voltage |
| GND           | 30                |   ground |
| DO           | 10 (GPIO 15)        |    digital output |

To set up the Lichtschranke, two options were considered.
1. Use external light source (e.g. a laser unit) to provide a high light intensity. In this case, the threshold should be tuned higher. It is also shown that the refelection of the laser via the carrera track is strong enough as well.
2. Use sun light purely without external light source. In this case, the threshold should be tuned lower. As long as we put the light sensor close enough to the car, a "dark" output will indeed be produced. The relating code is already adjusted to this set-up.

## Connect motor controller to Pi 4b
The connection between the motor controller L298N and Pi 4b is shown as follows
| L298N         | Pi 4b         |
| :------------ |:---------------:|
| IN1           | GPIO 23       |
| IN2           | GPIO 24       |
| ENA           | GPIO 25       |
| GND           | GND           |

Moreover, connect L298N to the power supply 12 V.

## Complile and run
Pi 0 acts as a server while the Pi 4b acts as a client. We need to first run the program on Pi 0, after which we run the program on Pi 4b. Otherwise, Pi 4b will fail to connect to Pi 0.
To compile the binary file on Pi 0, go to the directory `pi0` and run
```
g++ acc_server.cpp mpu6050.cpp -o ../build/acc_server -li2c 
```
To compile the binary file on Pi 0, go to the directory `pi4b` and run
```
g++ acc_client.cpp KF.cpp -o ../build/acc_client -I /usr/include/eigen3 -Wno-psabi -lpthread -lpigpio -lrt
```
Make sure to add `sudo` when you run `acc_client` on Pi 4b.
