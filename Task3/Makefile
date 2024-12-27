obj-m += tsulab.o

PWD := $(shell pwd)
KDIR := /lib/modules/$(shell uname -r)/build

all:
\tmake -C $(KDIR) M=$(PWD) modules

clean:
\tmake -C $(KDIR) M=$(PWD) clean
