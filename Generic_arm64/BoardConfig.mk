#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from parent
include device/mainline/generic/BoardConfig.mk

# Architecture
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=
TARGET_CPU_VARIANT := generic

# Boot manager
TARGET_GRUB_ARCH := arm64-efi

# Graphics (Mesa)
BOARD_MESA3D_GALLIUM_DRIVERS += \
    asahi \
    etnaviv \
    freedreno \
    lima \
    nouveau \
    panfrost \
    tegra \
    v3d \
    vc4
#    ethosu
#    rocket

BOARD_MESA3D_VULKAN_DRIVERS += \
    asahi \
    broadcom \
    freedreno \
    imagination \
    panfrost

# Graphics allocator (minigbm)
$(call soong_config_set,minigbm_upstream,platform,all_arm)
