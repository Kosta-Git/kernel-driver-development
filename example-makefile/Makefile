ifeq ($(KERNELRELEASE),)

    KERNELDIR ?= /mnt/f/code/kernel-development/WSL2-Linux-Kernel
    PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.mod modules.order *.symvers

.PHONY: modules modules_install clean

else
    # called from kernel build system: just declare what our modules are
    obj-m := hello.o
endif