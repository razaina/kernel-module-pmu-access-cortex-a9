obj-m := cyclecounter.o
KDIR := /home/razaina/Android-kernel-source/android_kernel_samsung_smdk4412/
PWD := $(shell pwd)
CCPATH := /home/razaina/android-ndk-r10d/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86/bin/
default:
	$(MAKE) CFLAGS_MODULE=-fno-pic ARCH=arm CROSS_COMPILE=$(CCPATH)/arm-linux-androideabi- -C $(KDIR) M=$(PWD) modules

clean:
	rm *.ko *.o *.mod.c *.order *.symvers 
