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
echo -n '"image": "'
[ -e /etc/promys-issue ] && cat /etc/promys-issue
echo -n '"'
awk -F = -- '{ printf(",\n\"%s\": \"%s\"", $1, $2); }' /boot/wifi.cfg
echo "}"
