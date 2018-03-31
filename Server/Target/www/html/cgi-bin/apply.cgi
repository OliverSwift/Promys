#!/bin/bash
export TMPDIR=/var/run/lighttpd

function error {
    echo "Content-Type: text/html"
    echo ""
    echo "<html>"
    echo "Error: " $1
    echo "</html>"
    exit
}

# POST vars parsing
read -N $CONTENT_LENGTH query
echo $query > $TMPDIR/apply.log
# Escape dangerous values. Attacker can exploit easily the eval part
eval `echo $query | tr '\`\$' "__" | sed -e 's:&:\n:g'`

# Password checking
if [ -e /boot/password.txt ]; then
    hash=$(echo $password | md5sum)
    if [ "$hash" != "`cat /boot/password.txt`" ]; then
        error "Wrong password";
    fi
else
    if [ "$new_password" = "" ]; then
        error "Password must bet set";
    fi
fi
if [ "$new_password" != "$password" -a "$new_password" != "" ]; then
    echo $new_password | md5sum > /boot/password.txt
fi

# WIFI configuration file generation
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
wpa_passphrase=$wifi_pass
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF

# NAT file activation creation or deletion
if [ $nat = "On" ]; then
	touch /boot/nat.txt
else
	rm -f /boot/nat.txt
fi
sync

# HTLM Redirection to settings.html
echo "Content-Type: text/html"
echo ""
echo "<script>"
echo "document.location='/settings.html';"
echo "</script>"
