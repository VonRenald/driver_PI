cmd_/home/pi/Documents/git/driver_PI/Module.symvers := sed 's/\.ko$$/\.o/' /home/pi/Documents/git/driver_PI/modules.order | scripts/mod/modpost -m -a  -o /home/pi/Documents/git/driver_PI/Module.symvers -e -i Module.symvers   -T -
