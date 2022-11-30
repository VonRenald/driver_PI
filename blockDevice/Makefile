KDIR = /lib/modules/5.15.61-v7l+/build
obj-m := dummy.o
EXTRA_CFLAGS = -Wall -g
default:
	make -C $(KDIR) M=$(shell pwd)
	sudo rmmod dummy.ko || echo 'dummy already clean'
	sudo insmod dummy.ko
	sudo mknod /dev/TpTemp c 42 0 || echo 'file exist'
	gcc main.c -o main
clean:
	make -C $(KDIR) M=$(shell pwd) clean
	sudo rmmod dummy.ko; true
	rm main