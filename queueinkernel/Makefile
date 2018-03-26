	obj-m:=QueueInKernel.o
	d=`pwd`
all:
	make -C /lib/modules/$(shell uname -r)/build M=$d modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$d clean
	
add:
	insmod QueueInKernel.ko
del:
	rmmod QueueInKernel

	
