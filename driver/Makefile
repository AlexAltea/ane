ifneq ($(KERNELRELEASE),)
	obj-m := ane.o
	ane-objs := src/ane_drv.o src/ane_h13.o src/ane_h14.o
else
	KERNELDIR := /lib/modules/$(shell uname -r)/build
 	PWD := $(shell pwd)

default:
	sudo cp ./src/uapi/drm/ane_accel.h /lib/modules/$(shell uname -r)/build/include/uapi/drm/
	make -C $(KERNELDIR) M=$(PWD) modules

install:
	# sudo make -C $(KERNELDIR) M=$(PWD) modules_install
	# sudo depmod -a
	# sudo modprobe ane
	sudo insmod ./ane.ko
	sudo chmod 666 /dev/accel/accel0

uninstall:
	# sudo modprobe -r ane
	sudo rmmod ane

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

endif
