
BUILD_DIR := build
ISO_DIR := iso
KERNEL_DIR := kernel
LIMINE_DIR := bootloader/limine
LIMINE_CONF := bootloader/limine.conf

OBJCOPY = /opt/homebrew/opt/binutils/bin/objcopy

LIMINE_BRANCH := v8.x-binary

X86_BUILD_DIR := $(BUILD_DIR)/x86_64
X86_TEST_BUILD_DIR := $(BUILD_DIR)/x86_64_test
X86_TEST_DIR := tests/kernel/arch/x86_64
KERN_TEST_DIR := tests/kernel
X86_ISO_DIR := $(ISO_DIR)/x86_64
ISO_NAME = waltos.iso

X86_CFLAGS = -ffreestanding \
		-fno-stack-protector \
		-fno-pic \
		-mno-red-zone \
		-mcmodel=kernel \
		-I./$(KERNEL_DIR)/include \

limine_setup:
	cd $(LIMINE_DIR) && git checkout $(LIMINE_BRANCH)
	cd $(LIMINE_DIR) && make
	cp $(LIMINE_DIR)/limine.h $(KERNEL_DIR)/include/

setup: limine_setup
	mkdir -p $(BUILD_DIR)
	mkdir -p $(ISO_DIR)

# limine:

x_86_setup: setup
	mkdir -p $(X86_BUILD_DIR)
	mkdir -p $(X86_ISO_DIR)
	mkdir -p $(X86_BUILD_DIR)/boot
	mkdir -p $(X86_BUILD_DIR)/memory_manager
	mkdir -p $(X86_BUILD_DIR)/libc
	mkdir -p $(X86_BUILD_DIR)/process
	mkdir -p $(X86_BUILD_DIR)/drivers
	mkdir -p $(X86_BUILD_DIR)/vfs
	mkdir -p $(X86_BUILD_DIR)/lib

X86_CLANG = clang --target=x86_64-elf $(X86_CFLAGS) -c $< -o $@

$(X86_BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/memory_manager/%.o: $(KERNEL_DIR)/memory_manager/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/process/%.o: $(KERNEL_DIR)/process/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/libc/%.o: $(KERNEL_DIR)/libc/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/lib/%.o: $(KERNEL_DIR)/lib/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/drivers/%.o: $(KERNEL_DIR)/drivers/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/%.o: $(KERNEL_DIR)/arch/x86_64/%.asm
	nasm -f elf64 $< -o $@

$(X86_BUILD_DIR)/%.o: $(KERNEL_DIR)/arch/x86_64/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/boot/%.o: $(KERNEL_DIR)/arch/x86_64/boot/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/%.o: $(KERNEL_DIR)/arch/x86_64/syscalls/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/drivers/%.o: $(KERNEL_DIR)/drivers/framebuffer/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/drivers/%.o: $(KERNEL_DIR)/drivers/ramfs/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/vfs/%.o: $(KERNEL_DIR)/vfs/%.c
	$(X86_CLANG)

$(X86_BUILD_DIR)/user/bin/%.bin:
	mkdir -p $(X86_BUILD_DIR)/user/bin
	nasm -f bin user/loop_app/loop.asm -o $(X86_BUILD_DIR)/user/bin/loop.bin

$(X86_BUILD_DIR)/user/bin/%.o: $(X86_BUILD_DIR)/user/bin/%.bin
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 \
		$< $@

X86_OBJS = \
	$(X86_BUILD_DIR)/kmain.o \
	$(X86_BUILD_DIR)/boot.o \
	$(X86_BUILD_DIR)/debug.o \
	$(X86_BUILD_DIR)/arch.o \
	$(X86_BUILD_DIR)/pit.o \
	$(X86_BUILD_DIR)/gdt.o \
	$(X86_BUILD_DIR)/gdt_flush.o \
	$(X86_BUILD_DIR)/idt.o \
	$(X86_BUILD_DIR)/isr_table.o \
	$(X86_BUILD_DIR)/isr_timer.o \
	$(X86_BUILD_DIR)/isr_yield.o \
	$(X86_BUILD_DIR)/isr.o \
	$(X86_BUILD_DIR)/pic.o \
	$(X86_BUILD_DIR)/boot/limine.o \
	$(X86_BUILD_DIR)/memory_manager/pmm.o \
	$(X86_BUILD_DIR)/libc/string.o \
	$(X86_BUILD_DIR)/vmm.o \
	$(X86_BUILD_DIR)/memory_manager/kmalloc.o \
	$(X86_BUILD_DIR)/process/process.o \
	$(X86_BUILD_DIR)/process.o \
	$(X86_BUILD_DIR)/task.o \
	$(X86_BUILD_DIR)/drivers/drivers.o \
	$(X86_BUILD_DIR)/drivers/fb.o \
	$(X86_BUILD_DIR)/drivers/fonts8x16.o \
	$(X86_BUILD_DIR)/process/scheduler.o \
	$(X86_BUILD_DIR)/tss.o \
	$(X86_BUILD_DIR)/syscall.o \
	$(X86_BUILD_DIR)/syscall_proc.o \
	$(X86_BUILD_DIR)/syscall_vfs.o \
	$(X86_BUILD_DIR)/syscall_handler.o \
	$(X86_BUILD_DIR)/vfs/vfs.o \
	$(X86_BUILD_DIR)/vfs/vfs_dentry.o \
	$(X86_BUILD_DIR)/lib/kstring.o \
	$(X86_BUILD_DIR)/drivers/ramfs.o \
	$(X86_BUILD_DIR)/user/bin/loop.o 



# user_bin_x86:
# 	mkdir -p $(X86_BUILD_DIR)/user/bin
# 	nasm -f bin user/loop_app/loop.asm -o $(X86_BUILD_DIR)/user/bin/loop.bin
# 	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 \
# 		$(X86_BUILD_DIR)/user/bin/loop.bin $(X86_BUILD_DIR)/user/bin/loop.o


link_kernel_x86: $(X86_OBJS)
	ld.lld \
    -T $(KERNEL_DIR)/linker.ld \
    -o $(X86_BUILD_DIR)/kernel.elf \
    $(X86_OBJS) 

os_iso_x86: link_kernel_x86
	mkdir -p $(X86_ISO_DIR)/boot
	mkdir -p $(X86_ISO_DIR)/boot/limine
	cp $(X86_BUILD_DIR)/kernel.elf $(X86_ISO_DIR)/
	cp $(LIMINE_CONF) $(X86_ISO_DIR)/boot/limine
	cp $(LIMINE_DIR)/limine-bios.sys $(X86_ISO_DIR)/boot/limine
	cp $(LIMINE_DIR)/limine-bios-cd.bin $(X86_ISO_DIR)/boot/limine
	xorriso -as mkisofs \
		-b boot/limine/limine-bios-cd.bin \
		-no-emul-boot \
		-boot-load-size 4 \
		-boot-info-table \
		$(X86_ISO_DIR) -o $(ISO_NAME)
	$(LIMINE_DIR)/limine bios-install $(ISO_NAME)

emulate_x86: 
	qemu-system-x86_64 \
    -cdrom $(ISO_NAME) \
    -serial stdio \
	-display cocoa,zoom-to-fit=on \
    -m 256M 

X86_TEST_SRCS = $(wildcard $(X86_TEST_DIR)/test_*.c)
X86_TEST_BINS = $(patsubst $(X86_TEST_DIR)/%.c, $(X86_TEST_BUILD_DIR)/%, $(X86_TEST_SRCS))
KERN_TEST_SRCS = $(wildcard $(KERN_TEST_DIR)/test_*.c)
KERN_TEST_BINS = $(patsubst $(KERN_TEST_DIR)/%.c, $(KERN_TEST_DIR)/%, $(KERN_TEST_SRCS))

X86_ARCH_SRCS = $(wildcard $(KERNEL_DIR)/arch/x86_64/*.c) \
	$(wildcard $(KERNEL_DIR)/arch/x86_64/boot/*.c) \
	$(wildcard $(KERNEL_DIR)/arch/x86_64/syscalls/*.c) \
	$(wildcard $(KERNEL_DIR)/*.c) \
	$(wildcard $(KERNEL_DIR)/libc/*.c) \
	$(wildcard $(KERNEL_DIR)/lib/*.c) \
	$(wildcard $(KERNEL_DIR)/memory_manager/*.c) \
	$(wildcard $(KERNEL_DIR)/process/*.c) \
	$(wildcard $(KERNEL_DIR)/drivers/*.c) \
	$(wildcard $(KERNEL_DIR)/drivers/framebuffer/*.c) \
	$(wildcard $(KERNEL_DIR)/drivers/ramfs/*.c) \
	$(wildcard $(KERNEL_DIR)/vfs/*.c)



# X86_TEST_KERN_OBJS = $(patsubst $(KERNEL_DIR)/arch/x86_64/%.c, $(X86_TEST_BUILD_DIR)/%.o, $(X86_ARCH_SRCS))
X86_TEST_KERN_OBJS = $(patsubst $(KERNEL_DIR)/%.c, $(X86_TEST_BUILD_DIR)/%.o, $(X86_ARCH_SRCS))

test_x86_setup: setup
	mkdir -p $(X86_TEST_BUILD_DIR)
	mkdir -p $(X86_TEST_BUILD_DIR)/arch/x86_64/boot
	mkdir -p $(X86_TEST_BUILD_DIR)/arch/x86_64/syscalls
	mkdir -p $(X86_TEST_BUILD_DIR)/libc
	mkdir -p $(X86_TEST_BUILD_DIR)/lib
	mkdir -p $(X86_TEST_BUILD_DIR)/memory_manager
	mkdir -p $(X86_TEST_BUILD_DIR)/vfs
	mkdir -p $(X86_TEST_BUILD_DIR)/process
	mkdir -p $(X86_TEST_BUILD_DIR)/drivers/framebuffer
	mkdir -p $(X86_TEST_BUILD_DIR)/drivers/ramfs

test_kern_setup: setup
	mkdir -p $(KERN_TEST_DIR)

X86_TEST_CLANG = clang -DUNIT_TEST -I./$(KERNEL_DIR)/include -I./$(KERNEL_DIR)/arch/x86_64 -c $< -o $@

$(X86_TEST_BUILD_DIR)/arch/x86_64/%.o: $(KERNEL_DIR)/arch/x86_64/%.c test_x86_setup
	$(X86_TEST_CLANG)

$(X86_TEST_BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c test_x86_setup
	$(X86_TEST_CLANG)

$(KERN_TEST_DIR)/%: $(KERN_TEST_DIR)/%.c $(X86_TEST_KERN_OBJS)
	clang -DUNIT_TEST -I./$(KERNEL_DIR)/include $< $(X86_TEST_KERN_OBJS) -o $@
	./$@

$(X86_TEST_BUILD_DIR)/%: $(X86_TEST_DIR)/%.c $(X86_TEST_KERN_OBJS)
	clang -DUNIT_TEST -I./$(KERNEL_DIR)/include $< $(X86_TEST_KERN_OBJS) -o $@
	./$@

test_x86: $(X86_TEST_BINS)
	echo "test suite completed"

test_kern: $(KERN_TEST_BINS)
	echo "test suite completed"

# test_kern: 
# 	echo $(KERN_TEST_BINS)

# test_x86:
# 	echo $(X86_TEST_SRCS)
# 	echo $(X86_TEST_BINS)

clean:
	rm -rf $(BUILD_DIR)/*
	rm -rf $(ISO_DIR)/*

