#!/bin/bash
echo "Content-Type: application/json;"
echo ""
echo "{"
if [ -e /boot/password.txt ]; then
    hash=$(echo $QUERY_STRING | md5sum)
    if [ "$hash" = "`cat /boot/password.txt`" ]; then
        echo '"access_granted": true,'
    else
        echo '"access_granted": false,'
    fi
else
    echo '"access_granted": true,'
fi
echo -n '"nat": ';
if [ -e /boot/nat.txt ]; then
	echo "true,"
else
	echo "false,"
fi
echo -n '"info": "'
[ -e /etc/promys-issue ] && cat /etc/promys-issue | tr '\n' '"'
echo ','
echo -n '"mac": "'
ip link show eth0 | grep ether | awk -- '{ printf("%s",$2); }'
echo '",'
echo -n '"ip": "'
hostname -I | cut -d ' ' -f 1 | tr '\n' '"'
awk -F = -- '{ printf(",\n\"%s\": \"%s\"", $1, $2); }' /boot/wifi.cfg
echo "}"
