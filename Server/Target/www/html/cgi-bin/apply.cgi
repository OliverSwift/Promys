#!/bin/bash
read -N $CONTENT_LENGTH query
echo $query > /tmp/apply.log
eval `echo $query | sed -e 's:&:\n:g'`
cat > /boot/wifi.cfg <<EOF
interface=wlan0
driver=nl80211
ssid=$ssid
hw_mode=g
channel=$channel
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=$password
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF
if [ $nat = "On" ]; then
	touch /boot/nat.txt
else
	rm -f /boot/nat.txt
fi
sync
echo "Content-Type: text/html"
echo ""
echo "<script>"
echo "document.location='/settings.html';"
echo "</script>"
