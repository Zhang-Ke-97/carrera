# carrera
overview...

## Libraries needed
Install Eigen on client Pi by running
    ```
    sudo apt-get install libeigen3-dev
    ```
Install i2c library on your server Pi by running 
    ```
    sudo apt-get install libi2c-dev
    ```
    
## Set up socket on both Pi
Make sure that both Pi are connected to Wifi. Then, follow the next steps to set static IP on both Pi.
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
      `<STATIC_IP>` is the IP we want to set for the Pi and it has to be out side of the pool of DHCP-addresses of the router.
      `<ROUTER_IP>` follows from the step 1.
      `<DNS_IP>` follows from the step 2 or can be set manually, e.g. Google DNS `8.8.8.8`.
       
## Set up MPU6050 and i2c-bus

connection: ...
orientation: y-direction should be tangential to the track. i.e. the direction of the car.
activate i2c-bus on server Pi

## Set up laser unit
connection: ...

## Set up light sensor
connection: ...
adjust sensitivity: ...
