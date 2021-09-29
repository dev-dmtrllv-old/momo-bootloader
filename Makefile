OUT_DIR = out
INCL_DIR = include

CPP_CORE_SRCS = $(wildcard src/core/*.cpp)
ASM_CORE_SRCS = $(wildcard src/core/*.asm)
CPP_CORE_HEADERS = $(wildcard include/core/*.hpp)
CPP_CORE_OBJS = $(patsubst src/%.cpp,$(OUT_DIR)/%.o,$(CPP_CORE_SRCS))
ASM_CORE_OBJS = $(patsubst src/core/%.asm,$(OUT_DIR)/core/%_asm.o,$(ASM_CORE_SRCS))
CORE_BIOS_SRCS = $(wildcard src/core/bios/*.asm)
CORE_BIOS_OBJS = $(patsubst src/%.asm,$(OUT_DIR)/%.o,$(CORE_BIOS_SRCS))

OPTIMIZATION = -O0

TARGET = i686

C_FLAGS = -ffreestanding $(OPTIMIZATION) -g -m32 -Wall -Wextra -fno-use-cxa-atexit -fno-exceptions -fno-rtti -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -fno-common -I$(INCL_DIR)
CC = $(TARGET)-elf-g++
OBJCPY = $(TARGET)-elf-objcopy
LD_FLAGS = -nostdlib -nolibc -nostartfiles -nodefaultlibs -fno-common -ffreestanding $(OPTIMIZATION)

ASM_BOOT_SRCS = $(wildcard src/boot/*.asm)
ASM_BOOT_INCL_DIR = src/boot/lib
ASM_BOOT_INCL_FILES = $(wildcard $(ASM_BOOT_INCL_DIR)/*.asm)
ASM_BOOT_OBJS = $(patsubst src/%.asm,$(OUT_DIR)/%.o,$(ASM_BOOT_SRCS))

BOOT_OFFSET = 90

# START Mount options
LOOP_DEV_ID1 = 50
LOOP_DEV_ID2 = 51
LOOP_DEV1 = /dev/loop$(LOOP_DEV_ID1)
LOOP_DEV2 = /dev/loop$(LOOP_DEV_ID2)
PART1_OFF = 1048576
PART2_OFF = 127926272
FS1_MOUNT_POINT = part1
FS2_MOUNT_POINT = part2
# END Mount options

QEMU = qemu-system-i386
QEMU_FLAGS = -M pc -no-reboot -m 512M -no-shutdown

DISK_IMG = out/disk.img
USB = /dev/sdb

all: $(ASM_BOOT_OBJS) out/core.bin

$(ASM_BOOT_SRCS): $(ASM_BOOT_INCL_FILES)
$(CPP_CORE_OBJS): $(CPP_CORE_SRCS)
$(ASM_CORE_OBJS): $(ASM_CORE_SRCS)
$(CORE_BIOS_OBJS): $(CORE_BIOS_SRCS)

out/boot/%.o: src/boot/%.asm $(ASM_BOOT_SRCS) $(ASM_BOOT_INCL_FILES)
	@mkdir -p $(@D)
	nasm -f bin $< -isrc/boot -o $@ $(NASM_DEFINES)


out/core.bin: $(CPP_CORE_OBJS) $(ASM_CORE_OBJS) $(CORE_BIOS_OBJS) linker.ld
	$(CC) -Tlinker.ld -o out/core.elf $(LD_FLAGS) $(CPP_CORE_OBJS) $(ASM_CORE_OBJS) $(CORE_BIOS_OBJS) -lgcc
	$(OBJCPY) --only-keep-debug out/core.elf out/core.sym
	$(OBJCPY) --strip-debug out/core.elf
	$(OBJCPY) -O binary out/core.elf $@
	rm -rf out/core.elf


out/core/%_asm.o: src/core/%.asm
	@mkdir -p $(@D)
	nasm -f elf32 -g $< -o $@

out/core/bios/%.o: src/core/bios/%.asm
	@mkdir -p $(@D)
	nasm -f elf32 -g $< -o $@

out/core/%.o: src/core/%.cpp $(CPP_CORE_HEADERS)
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) -c $< -o $@


$(DISK_IMG):
	@mkdir -p $(@D)
	test -e $(LOOP_DEV1) || sudo mknod -m 660 $(LOOP_DEV1) b 7 $(LOOP_DEV_ID1) && sudo chown root:disk $(LOOP_DEV1)
	test -e $(LOOP_DEV2) || sudo mknod -m 660 $(LOOP_DEV2) b 7 $(LOOP_DEV_ID2) && sudo chown root:disk $(LOOP_DEV2)
	dd if=/dev/zero of=$@ bs=512M count=1
	parted $@ --script \
	mklabel msdos \
	mkpart primary 1MB 128MB \
	mkpart primary 128MB 512MB \
	set 1 boot on
	sudo losetup -o $(PART1_OFF) $(LOOP_DEV1) $(DISK_IMG)
	sudo mkfs.fat -F 32 $(LOOP_DEV1)
	sudo losetup -d $(LOOP_DEV1)
	sudo losetup -o $(PART2_OFF) $(LOOP_DEV2) $(DISK_IMG)
	sudo mkfs.fat -F 32 $(LOOP_DEV2)
	sudo losetup -d $(LOOP_DEV2)


write-disk: $(DISK_IMG) $(ASM_BOOT_OBJS) out/core.bin
	make mount
	dd bs=1 if=out/boot/mbr.o of=$(DISK_IMG) conv=notrunc status=progress
	sudo dd bs=1 if=out/boot/boot1.o count=3 of=$(LOOP_DEV1) conv=notrunc
	sudo dd bs=1 skip=$(BOOT_OFFSET) if=out/boot/boot1.o iflag=skip_bytes of=$(LOOP_DEV1) seek=$(BOOT_OFFSET) conv=notrunc
	sudo dd bs=1 seek=1024 if=out/boot/boot2.o iflag=skip_bytes of=$(LOOP_DEV1) conv=notrunc
	sudo mkdir $(FS1_MOUNT_POINT)/momo || true
	sudo cp test-config.cfg $(FS1_MOUNT_POINT)/momo/boot.cfg
	sudo cp out/core.bin $(FS1_MOUNT_POINT)/momo/core.bin
	make unmount


mount:
	sudo losetup -o $(PART1_OFF) $(LOOP_DEV1) $(DISK_IMG)
	sudo losetup -o $(PART2_OFF) $(LOOP_DEV2) $(DISK_IMG)
	sudo mkdir $(FS1_MOUNT_POINT)
	sudo mount $(LOOP_DEV1) $(FS1_MOUNT_POINT)
	sudo mkdir $(FS2_MOUNT_POINT)
	sudo mount $(LOOP_DEV2) $(FS2_MOUNT_POINT)
	

unmount:
	sudo umount $(FS1_MOUNT_POINT) || true
	sudo umount $(FS2_MOUNT_POINT) || true
	sudo losetup -d $(LOOP_DEV1) || true
	sudo losetup -d $(LOOP_DEV2) || true
	sudo rmdir $(FS1_MOUNT_POINT) || true
	sudo rmdir $(FS2_MOUNT_POINT) || true


run:
	make write-disk
	$(QEMU) $(QEMU_FLAGS) -drive format=raw,file=$(DISK_IMG) 

debug:
	make write-disk
	$(QEMU) $(QEMU_FLAGS) -drive format=raw,file=$(DISK_IMG) -s -S 

clear:
	make clean
	clear


clean:
	rm -rf out
	rm -rf $(DISK_IMG)
	rm -rf *.mem


usb: $(ASM_BOOT_OBJS) out/core.bin
# make write-disk
	sudo umount $(USB) || true
	sudo umount $(USB)1 || true
	sudo umount $(USB)2 || true
	sudo parted $(USB) --script \
	mklabel msdos \
	mkpart primary 1MB 128MB \
	mkpart primary 128MB 512MB \
	set 1 boot on
	sudo mkfs.fat -F 32 $(USB)1
	sudo mkfs.fat -F 32 $(USB)2

	sudo dd bs=1 if=out/boot/mbr.o of=$(USB) conv=notrunc status=progress
	sudo dd bs=1 if=out/boot/boot1.o count=3 of=$(USB)1 conv=notrunc status=progress
	sudo dd bs=1 skip=$(BOOT_OFFSET) if=out/boot/boot1.o iflag=skip_bytes of=$(USB)1 seek=$(BOOT_OFFSET) conv=notrunc status=progress
	sudo dd bs=1 seek=1024 if=out/boot/boot2.o iflag=skip_bytes of=$(USB)1 conv=notrunc status=progress

	mkdir temp || true
	sudo mount /dev/sdb1 temp
	sudo mkdir temp/momo || true
	sudo cp test-config.cfg temp/momo/boot.cfg
	sudo cp out/core.bin temp/momo/core.bin
	sync
	sudo umount temp
	rmdir temp

test-usb: usb
	sleep 1
	sudo $(QEMU) $(QEMU_FLAGS) -drive format=raw,file=$(USB)
