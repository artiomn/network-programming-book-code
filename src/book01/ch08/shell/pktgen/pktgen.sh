#!/bin/sh

modprobe pktgen
cat /proc/net/pktgen/pgctrl
cat /proc/net/pktgen/kpktgend_5

echo "add_device lo" > /proc/net/pktgen/kpktgend_5

cat /proc/net/pktgen/lo

echo 'dst 127.0.0.1' > /proc/net/pktgen/lo
echo 'count 50' > /proc/net/pktgen/lo
echo 'delay 50' > /proc/net/pktgen/lo
echo 'pkt_size 1400' > /proc/net/pktgen/lo
echo 'udp_dst_min 11100' > /proc/net/pktgen/lo
echo 'udp_dst_max 11150' > /proc/net/pktgen/lo
echo '11100,11150 user pattern abcabc1234567890' > /proc/net/pktgen/lo

echo start > /proc/net/pktgen/pgctrl
sleep 3

echo stop > /proc/net/pktgen/pgctrl
#echo rem_device_all > /proc/net/pktgen/kpktgend_5

#modprobe -r pktgen

