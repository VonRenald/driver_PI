cmd_/home/pi/Documents/git/driver_PI/modules.order := {   echo /home/pi/Documents/git/driver_PI/dummy.ko; :; } | awk '!x[$$0]++' - > /home/pi/Documents/git/driver_PI/modules.order
