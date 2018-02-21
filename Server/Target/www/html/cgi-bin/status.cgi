#!/bin/bash
echo "Content-Type: application/json;"
echo ""
echo "{"
echo -n '"nat": ';
if [ -e /boot/nat.txt ]; then
	echo -n "true"
else
	echo -n "false"
fi
awk -F = -- '{ printf(",\n\"%s\": \"%s\"", $1, $2); }' /boot/wifi.cfg
echo "}"
