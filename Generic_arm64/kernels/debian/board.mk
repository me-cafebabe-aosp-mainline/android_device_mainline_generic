#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Debian kernel config files + Android kernel config fragments
#
# Debian does not use a defconfig. It merges its config files and fills
# the rest with the default values, so do the same: there is no
# TARGET_KERNEL_CONFIG, and all the fragments are merged at once, so that
# the options which depend on others in the fragments are not dropped.

TARGET_KERNEL_SOURCE ?= kernel/mainline/android-mainline
BOARD_KERNEL_IMAGE_NAME := Image
MERGE_ALL_KERNEL_CONFIGS_AT_ONCE := true
TARGET_KERNEL_CONFIG_EXT := \
    $(PRODUCT_OUT)/obj/KCONFIG_OBJ/debian.config \
    $(DEVICE_PATH)/configs/kernel/fix-build.config \
    kernel/mainline/configs/fragments/android-base-pre/common.config \
    kernel/mainline/configs/fragments/android-base-pre/arm64.config \
    kernel/configs/b/android-6.12/android-base.config \
    kernel/mainline/configs/fragments/android-base-conditional/CONFIG_ARM64-y.config \
    kernel/mainline/configs/fragments/common.config \
    kernel/mainline/configs/fragments/y/fbcon.config \
    $(DEVICE_PATH)/configs/kernel/customizations.config
