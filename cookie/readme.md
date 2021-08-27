## A simpe example of socket communication between two Raspberry Pi.
Before you stared, make sure to set static IP on both Pi. The following steps may help you to achieve this.
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
       
