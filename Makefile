OUT_DIR = out
INCL_DIR = include

CPP_SRCS = $(wildcard src/core/*.cpp)
ASM_CORE_SRCS = $(wildcard src/core/*.asm)
CPP_HEADERS = $(wildcard include/core/*.hpp)
CPP_OBJ = $(patsubst src/%.cpp,$(OUT_DIR)/%.o,$(CPP_SRCS))
ASM_CORE_OBJ = $(patsubst src/core/%.asm,$(OUT_DIR)/core/%_asm.o,$(ASM_CORE_SRCS))

OPTIMIZATION = -O2

TARGET = i686

C_FLAGS = -ffreestanding $(OPTIMIZATION) -m16 -Wall -Wextra -fno-use-cxa-atexit -fno-exceptions -fno-rtti -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -fno-common -I$(INCL_DIR)
CC = $(TARGET)-elf-g++
OBJCPY = $(TARGET)-elf-objcopy
LD_FLAGS = -nostdlib -nolibc -nostartfiles -nodefaultlibs -fno-common -ffreestanding -lgcc $(OPTIMIZATION)

ASM_SRCS = $(wildcard src/*.asm)
ASM_INCLUDE_DIR = src/lib
ASM_INCLUDE_FILES = $(wildcard $(ASM_INCLUDE_DIR)/*.asm)
ASM_OBJ = $(patsubst src/%.asm,$(OUT_DIR)/%.o,$(ASM_SRCS))

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
QEMU_FLAGS = -M pc -no-reboot -m 512M

DISK_IMG = out/disk.img
USB = /dev/sdb

all: $(ASM_OBJ) out/core.bin

out/core.bin: $(CPP_OBJ) $(ASM_CORE_OBJ) linker.ld
	$(CC) -Tlinker.ld -o $@ $(LD_FLAGS) $(CPP_OBJ) $(ASM_CORE_OBJ)

out/core/%_asm.o: src/core/%.asm
	@mkdir -p $(@D)
	nasm -f elf32 $< -o $@

out/%.o: src/%.asm $(ASM_SRCS) $(ASM_INCLUDE_FILES)
	@mkdir -p $(@D)
	nasm -f bin $< -isrc -o $@ $(NASM_DEFINES)

out/%.o: src/%.cpp $(CPP_HEADERS)
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

write-disk: $(DISK_IMG) $(ASM_OBJ) out/core.bin
	make mount
	dd bs=1 if=out/mbr.o of=$(DISK_IMG) conv=notrunc status=progress
	sudo dd bs=1 if=out/boot1.o count=3 of=$(LOOP_DEV1) conv=notrunc
	sudo dd bs=1 skip=$(BOOT_OFFSET) if=out/boot1.o iflag=skip_bytes of=$(LOOP_DEV1) seek=$(BOOT_OFFSET) conv=notrunc
	sudo dd bs=1 seek=1024 if=out/boot2.o iflag=skip_bytes of=$(LOOP_DEV1) conv=notrunc
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

clear:
	make clean
	clear

clean:
	rm -rf out
	rm -rf $(DISK_IMG)
	rm -rf *.mem

usb: $(ASM_OBJ) out/core.bin
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

	sudo dd bs=1 if=out/mbr.o of=$(USB) conv=notrunc status=progress
	sudo dd bs=1 if=out/boot1.o count=3 of=$(USB)1 conv=notrunc status=progress
	sudo dd bs=1 skip=$(BOOT_OFFSET) if=out/boot1.o iflag=skip_bytes of=$(USB)1 seek=$(BOOT_OFFSET) conv=notrunc status=progress
	sudo dd bs=1 seek=1024 if=out/boot2.o iflag=skip_bytes of=$(USB)1 conv=notrunc status=progress

	mkdir temp || true
	sudo mount /dev/sdb1 temp
	sudo mkdir temp/momo || true
	sudo cp test-config.cfg temp/momo/boot.cfg
	sudo cp out/core.bin temp/momo/core.bin
	sync
	sudo umount temp
	rmdir temp
# sudo mkdir $(FS1_MOUNT_POINT)/momo || true
# sudo cp test-config.cfg $(FS1_MOUNT_POINT)/momo/boot.cfg
# sudo cp out/core.bin $(FS1_MOUNT_POINT)/momo/core.bin
# make unmount
	
test-usb: usb
	sleep 1
	sudo $(QEMU) $(QEMU_FLAGS) -drive format=raw,file=$(USB)
