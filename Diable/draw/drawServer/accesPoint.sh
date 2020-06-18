# make access point
apt install dnsmasq hostapd
systemctl stop dnsmasq
systemctl stop hostapd
bash -c 'echo "interface wlan0" >> /etc/dhcpcd.conf'
bash -c 'echo "    static ip_address=193.168.4.1/24" >> /etc/dhcpcd.conf'
bash -c 'echo "    nohook wpa_supplicant" >> /etc/dhcpcd.conf'
service dhcpcd restart
mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
bash -c 'echo "interface=wlan0      # Use the require wireless interface - usually wlan0" >> /etc/dnsmasq.conf'
bash -c 'echo "dhcp-range=193.168.4.2,193.168.4.20,255.255.255.0,24h" >> /etc/dnsmasq.conf'
systemctl start dnsmasq
systemctl reload dnsmasq
bash -c 'echo "interface=wlan0" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "driver=nl80211" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "ssid=mintDraw" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "hw_mode=g" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "channel=7" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "wmm_enabled=0" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "macaddr_acl=0" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "auth_algs=1" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "ignore_broadcast_ssid=0" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "wpa=2" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "wpa_passphrase=bandederelous" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "wpa_key_mgmt=WPA-PSK" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "wpa_pairwise=TKIP" >> /etc/hostapd/hostapd.conf'
bash -c 'echo "rsn_pairwise=CCMP" >> /etc/hostapd/hostapd.conf'
sed -i '/DAEMON_CONF=/c\DAEMON_CONF="/etc/hostapd/hostapd.conf"' /etc/default/hostapd
systemctl unmask hostapd
systemctl enable hostapd
systemctl start hostapd
reboot
