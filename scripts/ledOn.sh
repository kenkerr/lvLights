sudo su
cd /sys/class/gpio
echo 27 > export
cd gpio27
echo out > direction
echo 1 > value
exit
cd ~/projects/lightController
