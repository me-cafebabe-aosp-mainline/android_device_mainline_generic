#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

ifeq ($(USES_DEVICE_MAINLINE_GENERIC),true)
ifeq ($(TARGET_DEVICE),Generic_x86_64)

INSTALLED_LIVEISOIMAGE_TARGET := $(PRODUCT_OUT)/$(BOOTMGR_ARTIFACT_FILENAME_PREFIX)-live.iso

INSTALLED_LIVEISOIMAGE_TARGET_INCLUDE_FILES := \
    $(INSTALLED_RAMDISK_ALL_COMBINED_TARGET) \
    $(PRODUCT_OUT)/kernel \
    $(PRODUCT_OUT)/system_dlkm.img \
    $(PRODUCT_OUT)/vendor.img \
    $(PRODUCT_OUT)/vendor_dlkm.img

ifeq ($(MAINLINE_GENERIC_LIVEISOIMAGE_CUSTOM_SYSTEM_IMAGE),)
INSTALLED_LIVEISOIMAGE_TARGET_INCLUDE_FILES += \
    $(PRODUCT_OUT)/system.img
endif

INSTALLED_LIVEISOIMAGE_TARGET_DEPS := \
	$(INSTALLED_LIVEISOIMAGE_TARGET_INCLUDE_FILES)

ifeq ($(TARGET_LINUX_FIRMWARE_IMAGE),)
$(warning TARGET_LINUX_FIRMWARE_IMAGE is empty, please set it to path to image with linux-firmware on its root)
endif

ifeq ($(TARGET_BOOT_MANAGER),grub)
ifneq ($(TARGET_GRUB_ARCH),)

INSTALLED_LIVEISOIMAGE_TARGET_DEPS += \
	$(TARGET_GRUB_LIVE_CONFIGS)

# grub-mkrescue adds these platforms to the ISO if their prebuilts exist,
# which makes the ISO boot on 32-bit UEFI firmware (i386-efi, with a
# 64-bit kernel in mixed mode) and on legacy BIOS (i386-pc). Depend on
# them so the build fails instead of silently producing an ISO without them.
ifeq ($(TARGET_GRUB_ARCH),x86_64-efi)
INSTALLED_LIVEISOIMAGE_TARGET_DEPS += \
	prebuilts/bootmgr/grub/$(HOST_PREBUILT_TAG)/i386-efi/lib/grub/i386-efi/kernel.img \
	prebuilts/bootmgr/grub/$(HOST_PREBUILT_TAG)/i386-pc/lib/grub/i386-pc/boot_hybrid.img
endif

TARGET_GRUB_HOST_PREBUILT_TAG ?= $(HOST_PREBUILT_TAG)
TARGET_GRUB_2ND_ARCH_HOST_PREBUILT_TAG ?= $(HOST_PREBUILT_TAG)
GRUB_PREBUILT_DIR := prebuilts/bootmgr/grub/$(TARGET_GRUB_HOST_PREBUILT_TAG)/$(TARGET_GRUB_ARCH)
GRUB_2ND_ARCH_PREBUILT_DIR := prebuilts/bootmgr/grub/$(TARGET_GRUB_2ND_ARCH_HOST_PREBUILT_TAG)/$(TARGET_GRUB_2ND_ARCH)

GRUB_WORKDIR_BASE := $(TARGET_OUT_INTERMEDIATES)/GRUB_OBJ
GRUB_WORKDIR_LIVE := $(GRUB_WORKDIR_BASE)/live

# $(1): output file
# -publisher: Ventoy then boots the ISO as it is, without changing its initrd, and
# generic_init finds the ISO through Ventoy's ACPI table
define make-liveisoimage-target
	rm -rf $(GRUB_WORKDIR_LIVE)
	mkdir -p $(GRUB_WORKDIR_LIVE)/fsroot/$(BOOTMGR_ANDROID_DIR_NAME) $(GRUB_WORKDIR_LIVE)/fsroot/boot/grub
	$(foreach file,$(INSTALLED_LIVEISOIMAGE_TARGET_INCLUDE_FILES),\
		ln $(file) $(GRUB_WORKDIR_LIVE)/fsroot/$(BOOTMGR_ANDROID_DIR_NAME)/;)

	cat $(TARGET_GRUB_LIVE_CONFIGS) > $(GRUB_WORKDIR_LIVE)/fsroot/boot/grub/grub.cfg
	$(call process-bootmgr-cfg-common,$(GRUB_WORKDIR_LIVE)/fsroot/boot/grub/grub.cfg)

	$(BOOTMGR_PATH_OVERRIDE) $(BOOTMGR_TOOLS_64_EXEC_ENV) $(GRUB_PREBUILT_DIR)/bin/grub-mkrescue \
		-d $(GRUB_PREBUILT_DIR)/lib/grub/$(TARGET_GRUB_ARCH) \
		-o $(1) \
		--xorriso=$(BOOTMGR_XORRISO_EXEC) \
		-publisher "VENTOY COMPATIBLE" \
		$(GRUB_WORKDIR_LIVE)/fsroot \
		$(if $(TARGET_LINUX_FIRMWARE_IMAGE),\
			/$(BOOTMGR_ANDROID_DIR_NAME)/firmware.img=$(TARGET_LINUX_FIRMWARE_IMAGE)) \
		$(if $(MAINLINE_GENERIC_LIVEISOIMAGE_CUSTOM_SYSTEM_IMAGE),\
			/$(BOOTMGR_ANDROID_DIR_NAME)/system.img=$(MAINLINE_GENERIC_LIVEISOIMAGE_CUSTOM_SYSTEM_IMAGE))
endef

endif # TARGET_GRUB_ARCH
endif # TARGET_BOOT_MANAGER

$(INSTALLED_LIVEISOIMAGE_TARGET): $(INSTALLED_LIVEISOIMAGE_TARGET_DEPS)
	$(call pretty,"Target Live ISO image: $@")
	$(call make-liveisoimage-target,$@)

.PHONY: liveisoimage
liveisoimage: $(INSTALLED_LIVEISOIMAGE_TARGET)

.PHONY: liveisoimage-nodeps
liveisoimage-nodeps:
	@echo "make $(INSTALLED_LIVEISOIMAGE_TARGET): ignoring dependencies"
	$(call make-liveisoimage-target,$(INSTALLED_LIVEISOIMAGE_TARGET))

endif # TARGET_DEVICE == Generic_x86_64
endif # USES_DEVICE_MAINLINE_GENERIC
