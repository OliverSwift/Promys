#!/bin/bash
echo "Content-Type: application/json;"
echo ""
echo "{"
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
