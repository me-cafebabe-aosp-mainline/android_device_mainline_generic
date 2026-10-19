#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

BOARD_KERNEL_IMAGE_NAME := bzImage
TARGET_KERNEL_SOURCE ?= kernel/mainline/android-mainline

TARGET_KERNEL_CONFIG := gki_defconfig
TARGET_KERNEL_CONFIG_EXT := \
    $(TARGET_DEVICE_PATH)/kernels/gki_debian/pre-debian.config \
    $(PRODUCT_OUT)/obj/KCONFIG_OBJ/debian-filtered.config \
    $(DEVICE_PATH)/configs/kernel/fix-build.config \
    kernel/mainline/configs/fragments/y/fbcon.config \
    $(DEVICE_PATH)/configs/kernel/customizations.config
