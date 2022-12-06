sudo hciconfig hci0 leadv3
sudo hciconfig hci0 noscan
sudo hcitool -i hci0 cmd 0x08 0x0008 1D 02 01 1a 03 03 aa fe 15 16 aa fe 10 ed 01 6E 79 63 75 2F 33 31 31 35 31 31 30 33 34 0A 00 00
# sudo hcitool -i hci0 cmd 0x08 0x0008 14 02 01 1a 03 03 aa fe 0c 16 aa fe 10 ed 02 70 74 74 2e 63 63 00 00 00 00 00 00 00 00 00 00 00