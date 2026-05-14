
BUILD_DIR := build
ISO_DIR := iso
KERNEL_DIR := kernel
LIMINE_DIR := bootloader/limine
LIMINE_CONF := bootloader/limine.conf

LIMINE_BRANCH := v8.x-binary

X86_BUILD_DIR := $(BUILD_DIR)/x86_64
X86_ISO_DIR := $(ISO_DIR)/x86_64
ISO_NAME = waltos.iso

X86_CFLAGS = -ffreestanding \
		-fno-stack-protector \
		-fno-pic \
		-mno-red-zone \
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

# boot_x86: x_86_setup
# 	nasm -f elf64 $(KERNEL_DIR)/arch/x86_64/boot.asm -o  $(X86_BUILD_DIR)/boot.o

# kernel_x86: boot_x86
# 	clang --target=x86_64-elf \
# 		$(X86_CFLAGS) \
# 		-c $(KERNEL_DIR)/kmain.c  \
# 		-o $(X86_BUILD_DIR)/kmain.o

$(X86_BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	clang --target=x86_64-elf \
		$(X86_CFLAGS) \
		-c $<  \
		-o $@

$(X86_BUILD_DIR)/%.o: $(KERNEL_DIR)/arch/x86_64/%.asm
	nasm -f elf64 $< -o $@

$(X86_BUILD_DIR)/%.o: $(KERNEL_DIR)/arch/x86_64/%.c
	clang --target=x86_64-elf \
		$(X86_CFLAGS) \
		-c $<  \
		-o $@

X86_OBJS = $(X86_BUILD_DIR)/kmain.o $(X86_BUILD_DIR)/boot.o $(X86_BUILD_DIR)/debug.o

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
    -m 256M

clean:
	rm -rf $(BUILD_DIR)/*
	rm -rf $(ISO_DIR)/*