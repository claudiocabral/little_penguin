
obj-m := bad_module.o
ifeq ($(KERNELRELEASE),)

NAME := $(basename, obj-m)
MODULE_NAME := $(NAME).ko
all:
	$(MAKE) -C /usr/src/linux-4.18.5 M=$(PWD)
endif
