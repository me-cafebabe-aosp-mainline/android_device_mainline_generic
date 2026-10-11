#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

TARGET_DEVICE_PATH := device/mainline/generic/Generic_arm

# Inherit from parent
$(call inherit-product, device/mainline/generic/device.mk)

# 32-bit only
PRODUCT_PACKAGES += init.zygote32.rc
PRODUCT_VENDOR_PROPERTIES += ro.zygote=zygote32
TARGET_SUPPORTS_32_BIT_APPS := true
TARGET_SUPPORTS_64_BIT_APPS := false
