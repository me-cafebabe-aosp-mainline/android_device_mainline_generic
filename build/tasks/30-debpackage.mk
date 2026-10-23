#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

ifeq ($(USES_DEVICE_MAINLINE_GENERIC),true)
ifneq ($(TARGET_NO_KERNEL),true)

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
    $(INSTALLED_RAMDISK_ALL_COMBINED_TARGET) \
    $(PRODUCT_OUT)/kernel \
    $(PRODUCT_OUT)/system.img \
    $(PRODUCT_OUT)/system_dlkm.img \
    $(PRODUCT_OUT)/vendor.img \
    $(PRODUCT_OUT)/vendor_dlkm.img

DEBPACKAGE_SRC_DIR := $(DEVICE_PATH)/configs/debpackage
DEBPACKAGE_SCRIPTS := preinst postinst postrm
DEBPACKAGE_TEMPLATES := \
    $(addprefix $(DEBPACKAGE_SRC_DIR)/,control conffiles default grub $(DEBPACKAGE_SCRIPTS))

DEBPACKAGE_WORKDIR := $(TARGET_OUT_INTERMEDIATES)/DEBPACKAGE_OBJ
DEBPACKAGE_ROOT := $(DEBPACKAGE_WORKDIR)/root

# $(1): files to process
define process-debpackage-templates
	$(call process-bootmgr-cfg-common,$(1))
	sed -i \
		-e "s|@ANDROID_DIR@|$(DEBPACKAGE_ANDROID_DIR)|g" \
		-e "s|@DEB_ARCH@|$(DEBPACKAGE_ARCH)|g" \
		-e "s|@DEB_INSTALLED_SIZE@|$$(du -sk $(DEBPACKAGE_ROOT)/etc $(DEBPACKAGE_ROOT)/usr | awk '{s += $$1} END {print s}')|g" \
		-e "s|@DEB_MAINTAINER@|$(MAINLINE_GENERIC_DEBPACKAGE_MAINTAINER)|g" \
		-e "s|@DEB_PACKAGE@|$(DEBPACKAGE_NAME)|g" \
		-e "s|@DEB_VERSION@|$(DEBPACKAGE_VERSION_PREFIX)+$$($(DATE_FROM_FILE) +%Y%m%d%H%M)|g" \
		-e "s|@TARGET_DEVICE@|$(TARGET_DEVICE)|g" \
		$(1)
endef

# The Android directory is 0700: Android chowns its user data to its own
# UIDs, which are ordinary user UIDs on the host.
# The package is not compressed: the kernel, ramdisk and images are already.
# $(1): output file
define make-debpackage-target
	rm -rf $(DEBPACKAGE_WORKDIR)
	mkdir -p $(DEBPACKAGE_ROOT)/DEBIAN $(DEBPACKAGE_ROOT)$(DEBPACKAGE_ANDROID_DIR) \
		$(DEBPACKAGE_ROOT)/etc/default $(DEBPACKAGE_ROOT)/etc/grub.d
	$(foreach file,$(INSTALLED_DEBPACKAGE_TARGET_INCLUDE_FILES),\
		ln $(file) $(DEBPACKAGE_ROOT)$(DEBPACKAGE_ANDROID_DIR)/;)
	cp $(DEBPACKAGE_SRC_DIR)/default $(DEBPACKAGE_ROOT)/etc/default/$(DEBPACKAGE_NAME)
	cp $(DEBPACKAGE_SRC_DIR)/grub $(DEBPACKAGE_ROOT)/etc/grub.d/35_$(DEBPACKAGE_NAME)
	cp $(addprefix $(DEBPACKAGE_SRC_DIR)/,control conffiles $(DEBPACKAGE_SCRIPTS)) $(DEBPACKAGE_ROOT)/DEBIAN/

	chmod -R u=rwX,go=rX $(DEBPACKAGE_ROOT)
	chmod 0700 $(DEBPACKAGE_ROOT)$(DEBPACKAGE_ANDROID_DIR)
	chmod 0755 $(DEBPACKAGE_ROOT)/etc/grub.d/35_$(DEBPACKAGE_NAME) \
		$(addprefix $(DEBPACKAGE_ROOT)/DEBIAN/,$(DEBPACKAGE_SCRIPTS))
	$(call process-debpackage-templates,\
		$(DEBPACKAGE_ROOT)/etc/default/$(DEBPACKAGE_NAME) \
		$(DEBPACKAGE_ROOT)/etc/grub.d/35_$(DEBPACKAGE_NAME) \
		$(DEBPACKAGE_ROOT)/DEBIAN/*)

	PATH=/usr/local/bin:/usr/bin:/bin dpkg-deb --root-owner-group -Znone --build $(DEBPACKAGE_ROOT) $(1)
endef

$(INSTALLED_DEBPACKAGE_TARGET): $(INSTALLED_DEBPACKAGE_TARGET_INCLUDE_FILES) $(DEBPACKAGE_TEMPLATES)
	$(call pretty,"Target Debian package: $@")
	$(call make-debpackage-target,$@)

.PHONY: debpackage
debpackage: $(INSTALLED_DEBPACKAGE_TARGET)

.PHONY: debpackage-nodeps
debpackage-nodeps:
	@echo "make $(INSTALLED_DEBPACKAGE_TARGET): ignoring dependencies"
	$(call make-debpackage-target,$(INSTALLED_DEBPACKAGE_TARGET))

endif # DEBPACKAGE_ARCH

endif # TARGET_NO_KERNEL
endif # USES_DEVICE_MAINLINE_GENERIC
