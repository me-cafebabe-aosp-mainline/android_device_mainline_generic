/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#pragma once

namespace hardware_detect {

// From system/core/libsystem/include/system/graphics-base-v1.0.h
typedef enum {
    HAL_PIXEL_FORMAT_RGBA_8888 = 1,
    HAL_PIXEL_FORMAT_RGBX_8888 = 2,
    HAL_PIXEL_FORMAT_RGB_888 = 3,
    HAL_PIXEL_FORMAT_RGB_565 = 4,
    HAL_PIXEL_FORMAT_BGRA_8888 = 5,
} android_pixel_format_t;

constexpr int kGlesVersion20 = 131072;
constexpr int kGlesVersion30 = 196608;
constexpr int kGlesVersion31 = 196609;
constexpr int kGlesVersion32 = 196610;

enum class HwEgl : int {
    Unset = 0,
    Angle,
    Mesa,
};

enum class HwVulkan : int {
    Unset = 0,
    Asahi,
    Broadcom,
    Freedreno,
    Imagination,
    Intel,
    Intel_hasvk,
    Lvp,
    Nouveau,
    Panfrost,
    Radeon,
    Virtio,
};

enum class GrallocApex : int {
    Unset = 0,
    Minigbm,
    Fb,
    GbmMesa,
};

enum class HwcApex : int {
    Unset = 0,
    Drm,
    DrmFb,
    Empty,
};

enum class UsbGadgetApex : int {
    Unset = 0,
    Mainline,
    None,
};

enum class VulkanApex : int {
    Unset = 0,
    No_apex,
    Swiftshader,
};

enum class MinigbmGenericBackend : int {
    Unset = 0,
    DmabufHeap,
    DumbGeneric,
    GbmMesa,
    Staging,
};

// The graphics and USB gadget selections made by the detection, which
// are applied as system properties by ApplySelections()
struct Selections {
    bool uses_sw_rendered_graphics = false;

    // The DRM card (display) and the DRM render are different devices
    bool hybrid_graphics = false;

    int gles_version = kGlesVersion20;
    HwEgl hw_egl = HwEgl::Unset;
    HwVulkan hw_vulkan = HwVulkan::Unset;
    GrallocApex gralloc_apex = GrallocApex::Unset;
    HwcApex hwc_apex = HwcApex::Unset;
    UsbGadgetApex usb_gadget_apex = UsbGadgetApex::Unset;
    VulkanApex vulkan_apex = VulkanApex::Unset;
    MinigbmGenericBackend minigbm_generic_backend = MinigbmGenericBackend::Unset;
    android_pixel_format_t sf_native_window_buffers_format = HAL_PIXEL_FORMAT_RGBA_8888;

    void UseSwiftshaderGraphics();
    void UseMesaSoftwareGraphics();
    void SetupFramebufferDisplay();
};

bool IsForcedSwiftshader();
bool IsForcedFramebufferDisplay();

// Process the boot overrides, and then set the properties according to
// the selections
bool ApplySelections(Selections& selections);

}  // namespace hardware_detect
