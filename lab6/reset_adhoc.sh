sudo ifconfig adhoc0 down
sudo iwconfig adhoc0 mode ad-hoc essid bun-mesh-5 channel 5
sudo ip -6 addr add FE80::42:42:34/128 dev adhoc0
sudo ifconfig adhoc0 up
ifconfig
sudo iwconfig