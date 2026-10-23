#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

ifeq ($(USES_DEVICE_MAINLINE_GENERIC),true)

DEBPACKAGE_ARCH_arm64 := arm64
DEBPACKAGE_ARCH_x86_64 := amd64
DEBPACKAGE_ARCH := $(DEBPACKAGE_ARCH_$(TARGET_ARCH))

ifneq ($(DEBPACKAGE_ARCH),)

DEBPACKAGE_NAME := android-system-$(subst _,-,$(call to-lower,$(TARGET_DEVICE)))
DEBPACKAGE_ANDROID_DIR := /usr/lib/android-system/$(TARGET_DEVICE)
MAINLINE_GENERIC_DEBPACKAGE_MAINTAINER ?= Unknown <unknown@localhost>

ifneq ($(LINEAGE_BUILD),)
DEBPACKAGE_VERSION_PREFIX := $(PRODUCT_VERSION_MAJOR).$(PRODUCT_VERSION_MINOR)
else
DEBPACKAGE_VERSION_PREFIX := $(PLATFORM_VERSION_LAST_STABLE)
endif

INSTALLED_DEBPACKAGE_TARGET := $(PRODUCT_OUT)/$(BOOTMGR_ARTIFACT_FILENAME_PREFIX)_$(DEBPACKAGE_ARCH).deb

INSTALLED_DEBPACKAGE_TARGET_INCLUDE_FILES := \
    $(PRODUCT_OUT)/system.img \
    $(PRODUCT_OUT)/system_dlkm.img \
    $(PRODUCT_OUT)/vendor.img \
    $(PRODUCT_OUT)/vendor_dlkm.img

# Without a kernel, Android boots with the kernel of the Debian installation
# that the package is installed on, which is not known at build time. The
# package has the pieces of the ramdisk, and the files to make the kernel and
# the ramdisk of it at installation time and whenever the Debian kernel changes.
ifeq ($(TARGET_NO_KERNEL),true)
DEBPACKAGE_HOST_KERNEL := true
DEBPACKAGE_RAMDISK_PIECE_FILES := $(INSTALLED_RAMDISK_ALL_COMBINED_TARGET_DEPS)
DEBPACKAGE_EXTRA_DEPENDS := , python3, initramfs-tools, kmod, cpio, lz4, xz-utils, zstd, dkms
DEBPACKAGE_EXTRA_RECOMMENDS := , linux-image-$(DEBPACKAGE_ARCH), linux-headers-$(DEBPACKAGE_ARCH)
INSTALLED_DEBPACKAGE_TARGET_INCLUDE_FILES += $(DEBPACKAGE_RAMDISK_PIECE_FILES)
else
DEBPACKAGE_HOST_KERNEL := false
INSTALLED_DEBPACKAGE_TARGET_INCLUDE_FILES += \
    $(INSTALLED_RAMDISK_ALL_COMBINED_TARGET) \
    $(PRODUCT_OUT)/kernel
endif

DEBPACKAGE_SRC_DIR := $(DEVICE_PATH)/configs/debpackage
DEBPACKAGE_SCRIPTS := preinst postinst prerm postrm
DEBPACKAGE_TEMPLATES := \
    $(addprefix $(DEBPACKAGE_SRC_DIR)/,control conffiles default grub $(DEBPACKAGE_SCRIPTS))

# The module lists: the modules which Android needs, from the kernel tree and
# the device, and the ones of the ramdisk
# (modules.load.debian: what Android needs that a Debian kernel builds as a
# module, and nothing loads on demand)
DEBPACKAGE_MODULES_LOAD_SRCS := \
    kernel/mainline/configs/modprobe/modules.load \
    $(DEVICE_PATH)/configs/modprobe/modules.load \
    $(DEVICE_PATH)/configs/modprobe/modules.load.debian
# ashmem, which Debian kernels do not have, as DKMS source; dkms.conf has its version
DEBPACKAGE_ASHMEM_DKMS_DIR := $(DEVICE_PATH)/dkms/ashmem-linux
DEBPACKAGE_ASHMEM_DKMS_VERSION := 1.0
DEBPACKAGE_HOST_KERNEL_FILES := \
    $(shell find $(DEBPACKAGE_ASHMEM_DKMS_DIR) -type f) \
    $(DEBPACKAGE_MODULES_LOAD_SRCS) \
    $(addprefix $(DEVICE_PATH)/configs/modprobe/,\
        modules.load.ramdisk modules.load_prefix.ramdisk modules.options) \
    $(addprefix $(DEBPACKAGE_SRC_DIR)/,update post-update kernel-postrm initramfs-hook)

DEBPACKAGE_WORKDIR := $(TARGET_OUT_INTERMEDIATES)/DEBPACKAGE_OBJ
DEBPACKAGE_ROOT := $(DEBPACKAGE_WORKDIR)/root

# $(1): files to process
define process-debpackage-templates
	$(call process-bootmgr-cfg-common,$(1))
	sed -i \
		-e "$(if $(filter true,$(DEBPACKAGE_HOST_KERNEL)),s|^@HOST_KERNEL@||,/^@HOST_KERNEL@/d)" \
		-e "s|@ANDROID_DIR@|$(DEBPACKAGE_ANDROID_DIR)|g" \
		-e "s|@DEB_ARCH@|$(DEBPACKAGE_ARCH)|g" \
		-e "s|@DEB_EXTRA_DEPENDS@|$(DEBPACKAGE_EXTRA_DEPENDS)|g" \
		-e "s|@DEB_EXTRA_RECOMMENDS@|$(DEBPACKAGE_EXTRA_RECOMMENDS)|g" \
		-e "s|@DEB_HOST_KERNEL@|$(DEBPACKAGE_HOST_KERNEL)|g" \
		-e "s|@DEB_INSTALLED_SIZE@|$$(du -sk $(DEBPACKAGE_ROOT)/etc $(DEBPACKAGE_ROOT)/usr | awk '{s += $$1} END {print s}')|g" \
		-e "s|@DEB_MAINTAINER@|$(MAINLINE_GENERIC_DEBPACKAGE_MAINTAINER)|g" \
		-e "s|@DEB_PACKAGE@|$(DEBPACKAGE_NAME)|g" \
		-e "s|@DEB_VERSION@|$(DEBPACKAGE_VERSION_PREFIX)+$$($(DATE_FROM_FILE) +%Y%m%d%H%M)|g" \
		-e "s|@TARGET_DEVICE@|$(TARGET_DEVICE)|g" \
		$(1)
endef

# Without a kernel: the pieces of the ramdisk, which the installation merges
# with the Debian initramfs, and what makes the kernel and the ramdisk at
# installation time and whenever the Debian kernel changes
DEBPACKAGE_SHARE_DIR := $(DEBPACKAGE_ROOT)/usr/share/$(DEBPACKAGE_NAME)
DEBPACKAGE_INITRAMFS_HOOK := $(DEBPACKAGE_ROOT)/usr/share/initramfs-tools/hooks/$(DEBPACKAGE_NAME)

ifeq ($(DEBPACKAGE_HOST_KERNEL),true)
define make-debpackage-host-kernel-files
	mkdir -p $(DEBPACKAGE_ROOT)$(DEBPACKAGE_ANDROID_DIR)/ramdisk $(DEBPACKAGE_SHARE_DIR) \
		$(dir $(DEBPACKAGE_INITRAMFS_HOOK)) $(DEBPACKAGE_ROOT)/etc/initramfs/post-update.d \
		$(DEBPACKAGE_ROOT)/etc/kernel/postrm.d
	n=0; for piece in $(DEBPACKAGE_RAMDISK_PIECE_FILES); do \
		n=$$((n + 1)); \
		ln $$piece $(DEBPACKAGE_ROOT)$(DEBPACKAGE_ANDROID_DIR)/ramdisk/$$n-$$(basename $$piece); \
	done
	cat $(DEBPACKAGE_MODULES_LOAD_SRCS) > $(DEBPACKAGE_SHARE_DIR)/modules.load
	cp $(addprefix $(DEVICE_PATH)/configs/modprobe/,\
		modules.load.ramdisk modules.load_prefix.ramdisk modules.options) $(DEBPACKAGE_SHARE_DIR)/
	cp $(addprefix $(DEBPACKAGE_SRC_DIR)/,update post-update kernel-postrm) $(DEBPACKAGE_SHARE_DIR)/
	cp $(DEBPACKAGE_SRC_DIR)/initramfs-hook $(DEBPACKAGE_INITRAMFS_HOOK)
	mkdir -p $(DEBPACKAGE_ROOT)/usr/src/ashmem-linux-$(DEBPACKAGE_ASHMEM_DKMS_VERSION)
	cp -r $(DEBPACKAGE_ASHMEM_DKMS_DIR)/. $(DEBPACKAGE_ROOT)/usr/src/ashmem-linux-$(DEBPACKAGE_ASHMEM_DKMS_VERSION)/
	ln -s /usr/share/$(DEBPACKAGE_NAME)/post-update \
		$(DEBPACKAGE_ROOT)/etc/initramfs/post-update.d/$(DEBPACKAGE_NAME)
	ln -s /usr/share/$(DEBPACKAGE_NAME)/kernel-postrm \
		$(DEBPACKAGE_ROOT)/etc/kernel/postrm.d/yy-$(DEBPACKAGE_NAME)
	chmod -R u=rwX,go=rX $(DEBPACKAGE_ROOT)/usr $(DEBPACKAGE_ROOT)/etc
	chmod 0755 $(addprefix $(DEBPACKAGE_SHARE_DIR)/,update post-update kernel-postrm) \
		$(DEBPACKAGE_INITRAMFS_HOOK)
	$(call process-debpackage-templates,\
		$(addprefix $(DEBPACKAGE_SHARE_DIR)/,update post-update kernel-postrm) \
		$(DEBPACKAGE_INITRAMFS_HOOK))
endef
else
define make-debpackage-host-kernel-files
endef
endif

# The Android directory is 0700: Android chowns its user data to its own
# UIDs, which are ordinary user UIDs on the host.
# The package is not compressed: the kernel, ramdisk and images are already.
# $(1): output file
define make-debpackage-target
	rm -rf $(DEBPACKAGE_WORKDIR)
	mkdir -p $(DEBPACKAGE_ROOT)/DEBIAN $(DEBPACKAGE_ROOT)$(DEBPACKAGE_ANDROID_DIR) \
		$(DEBPACKAGE_ROOT)/etc/default $(DEBPACKAGE_ROOT)/etc/grub.d
	$(foreach file,$(filter-out $(DEBPACKAGE_RAMDISK_PIECE_FILES),$(INSTALLED_DEBPACKAGE_TARGET_INCLUDE_FILES)),\
		ln $(file) $(DEBPACKAGE_ROOT)$(DEBPACKAGE_ANDROID_DIR)/;)
	cp $(DEBPACKAGE_SRC_DIR)/default $(DEBPACKAGE_ROOT)/etc/default/$(DEBPACKAGE_NAME)
	cp $(DEBPACKAGE_SRC_DIR)/grub $(DEBPACKAGE_ROOT)/etc/grub.d/35_$(DEBPACKAGE_NAME)
	cp $(addprefix $(DEBPACKAGE_SRC_DIR)/,control conffiles $(DEBPACKAGE_SCRIPTS)) $(DEBPACKAGE_ROOT)/DEBIAN/

	chmod -R u=rwX,go=rX $(DEBPACKAGE_ROOT)
	chmod 0700 $(DEBPACKAGE_ROOT)$(DEBPACKAGE_ANDROID_DIR)
	chmod 0755 $(DEBPACKAGE_ROOT)/etc/grub.d/35_$(DEBPACKAGE_NAME) \
		$(addprefix $(DEBPACKAGE_ROOT)/DEBIAN/,$(DEBPACKAGE_SCRIPTS))
	$(call make-debpackage-host-kernel-files)
	$(call process-debpackage-templates,\
		$(DEBPACKAGE_ROOT)/etc/default/$(DEBPACKAGE_NAME) \
		$(DEBPACKAGE_ROOT)/etc/grub.d/35_$(DEBPACKAGE_NAME) \
		$(DEBPACKAGE_ROOT)/DEBIAN/*)

	PATH=/usr/local/bin:/usr/bin:/bin dpkg-deb --root-owner-group -Znone --build $(DEBPACKAGE_ROOT) $(1)
endef

$(INSTALLED_DEBPACKAGE_TARGET): $(INSTALLED_DEBPACKAGE_TARGET_INCLUDE_FILES) $(DEBPACKAGE_TEMPLATES) \
    $(if $(filter true,$(DEBPACKAGE_HOST_KERNEL)),$(DEBPACKAGE_HOST_KERNEL_FILES))
	$(call pretty,"Target Debian package: $@")
	$(call make-debpackage-target,$@)

.PHONY: debpackage
debpackage: $(INSTALLED_DEBPACKAGE_TARGET)

.PHONY: debpackage-nodeps
debpackage-nodeps:
	@echo "make $(INSTALLED_DEBPACKAGE_TARGET): ignoring dependencies"
	$(call make-debpackage-target,$(INSTALLED_DEBPACKAGE_TARGET))

endif # DEBPACKAGE_ARCH

endif # USES_DEVICE_MAINLINE_GENERIC
