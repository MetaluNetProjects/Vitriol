#!/bin/bash
cd $(dirname $0)

# allow user to use port 80:
#sudo apt-get install libcap2-bin
#sudo setcap cap_net_bind_service=+ep `readlink -f \`which node\``

#sudo route add default gw 192.168.5.1
sudo route add -net 192.168.5.0 netmask 255.255.255.0 gw 192.168.5.1
sudo killall dnsmasq
sudo dnsmasq -C dnsmasq.conf

