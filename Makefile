KDIR = /lib/modules/5.15.61-v7l+/build
obj-m := dummy.o
EXTRA_CFLAGS = -Wall -g
default:
	make -C $(KDIR) M=$(shell pwd)
	sudo rmmod dummy.ko || echo 'dummy already clean'
	sudo insmod dummy.ko
	sudo mknod /dev/TP1 c 42 0 || echo 'file exist'
	gcc tp0_2.c -o tp0_2
clean:
	make -C $(KDIR) M=$(shell pwd) clean
	sudo rmmod dummy.ko; true
	rm tp0_2