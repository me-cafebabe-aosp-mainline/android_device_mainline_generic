/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include "selections.h"

#include <string>
#include <unordered_map>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>
#include <android-base/properties.h>

using namespace android::base;

namespace hardware_detect {

namespace {

constexpr char kGlesVersionProp[] = "ro.opengles.version";
constexpr char kHwEglProp[] = "ro.hardware.egl";
constexpr char kHwVulkanProp[] = "ro.hardware.vulkan";

constexpr char kGrallocApexProp[] = "ro.boot.vendor.apex.com.android.hardware.graphics.allocator";
constexpr char kHwcApexProp[] = "ro.boot.vendor.apex.com.android.hardware.graphics.composer";
constexpr char kUsbGadgetApexProp[] = "ro.boot.vendor.apex.com.android.hardware.usb.gadget";
constexpr char kVulkanApexProp[] = "ro.boot.vendor.apex.org.lineageos.device.graphics.vulkan";

constexpr char kBootGraphicsProp[] = "ro.boot.graphics";
constexpr char kBootUseFbDisplayProp[] = "ro.boot.use_fb_display";

constexpr char kMinigbmGenericBackendProp[] = "vendor.minigbm.generic_backend";

// constexpr char kSfNativeWindowBuffersFormatProp[] =
//         "ro.surface_flinger.native_window_buffers_format";
constexpr char kSfSupportsBackgroundBlurProp[] = "ro.surface_flinger.supports_background_blur";

constexpr char kUsbAdbDisabledProp[] = "vendor.sys.usb.adb.disabled";

const std::unordered_map<HwEgl, std::string> kHwEglMap = {
        {HwEgl::Angle, "angle"},
        {HwEgl::Mesa, "mesa"},
};

const std::unordered_map<HwVulkan, std::string> kHwVulkanMap = {
        {HwVulkan::Intel, "intel"},         {HwVulkan::Intel_hasvk, "intel_hasvk"},
        {HwVulkan::Nouveau, "nouveau"},     {HwVulkan::Radeon, "radeon"},
        {HwVulkan::Virtio, "virtio"},       {HwVulkan::Lvp, "lvp_mesa3d"},
        {HwVulkan::Freedreno, "freedreno"}, {HwVulkan::Asahi, "asahi"},
        {HwVulkan::Broadcom, "broadcom"},   {HwVulkan::Imagination, "imagination"},
        {HwVulkan::Panfrost, "panfrost"},
};

const std::unordered_map<GrallocApex, std::string> kGrallocApexMap = {
        {GrallocApex::Minigbm, "com.android.hardware.graphics.allocator.minigbm_upstream"},
        {GrallocApex::Fb, "com.android.hardware.graphics.allocator.fb"},
        {GrallocApex::GbmMesa, "com.android.hardware.graphics.allocator.gm"},
};

const std::unordered_map<HwcApex, std::string> kHwcApexMap = {
        {HwcApex::Drm, "com.android.hardware.graphics.composer.drm_hwcomposer_upstream"},
        {HwcApex::DrmFb, "com.android.hardware.graphics.composer.drmfb"},
        {HwcApex::Empty, "com.android.hardware.graphics.composer.empty"},
};

const std::unordered_map<UsbGadgetApex, std::string> kUsbGadgetApexMap = {
        {UsbGadgetApex::Mainline, "com.android.hardware.usb.gadget.mainline"},
        {UsbGadgetApex::None, "com.android.hardware.usb.gadget.none"},
};

const std::unordered_map<VulkanApex, std::string> kVulkanApexMap = {
        {VulkanApex::No_apex, "org.lineageos.device.graphics.vulkan.no_apex"},
        {VulkanApex::Swiftshader, "org.lineageos.device.graphics.vulkan.swiftshader"},
};

const std::unordered_map<MinigbmGenericBackend, std::string> kMinigbmGenericBackendMap = {
        {MinigbmGenericBackend::DmabufHeap, "dmabuf_heap"},
        {MinigbmGenericBackend::DumbGeneric, "dumb_generic"},
        {MinigbmGenericBackend::GbmMesa, "gbm_mesa"},
        {MinigbmGenericBackend::Staging, "staging"},
};

void ProcessBootOverrides(Selections& selections) {
    const std::unordered_map<std::string, int*> kBootOverridesProp = {
            {"gles_version", &selections.gles_version},
            {"hw_egl", reinterpret_cast<int*>(&selections.hw_egl)},
            {"hw_vulkan", reinterpret_cast<int*>(&selections.hw_vulkan)},
            {"gralloc_apex", reinterpret_cast<int*>(&selections.gralloc_apex)},
            {"hwc_apex", reinterpret_cast<int*>(&selections.hwc_apex)},
            {"usb_gadget_apex", reinterpret_cast<int*>(&selections.usb_gadget_apex)},
            {"vulkan_apex", reinterpret_cast<int*>(&selections.vulkan_apex)},
            {"minigbm_generic_backend",
             reinterpret_cast<int*>(&selections.minigbm_generic_backend)},
            {"sf_native_window_buffers_format",
             reinterpret_cast<int*>(&selections.sf_native_window_buffers_format)},
    };

    for (const auto& [name, pvar] : kBootOverridesProp) {
        std::string prop = "ro.boot." + name;
        int new_value = GetIntProperty<int>(prop, -1);
        if (new_value != -1) {
            LOG(INFO) << __FUNCTION__ << "(): Override " << name << " from "
                      << std::to_string(*pvar) << " to " << std::to_string(new_value);
            *pvar = new_value;
        }
    }
}

}  // namespace

void Selections::UseSwiftshaderGraphics() {
    uses_sw_rendered_graphics = true;
    hw_egl = HwEgl::Angle;
    gles_version = kGlesVersion31;
    vulkan_apex = VulkanApex::Swiftshader;
}

void Selections::SetupFramebufferDisplay() {
    gralloc_apex = GrallocApex::Fb;
    hwc_apex = HwcApex::Empty;

    UseSwiftshaderGraphics();
}

bool IsForcedSwiftshader() {
    return GetProperty(kBootGraphicsProp, "") == "swiftshader";
}

bool IsForcedFramebufferDisplay() {
    return GetBoolProperty(kBootUseFbDisplayProp, false);
}

bool ApplySelections(Selections& selections) {
    ProcessBootOverrides(selections);

    bool ret = true;
    const std::string* strp;

    if (selections.usb_gadget_apex != UsbGadgetApex::Unset) {
        strp = &kUsbGadgetApexMap.at(selections.usb_gadget_apex);
        LOG(INFO) << "Set USB Gadget APEX to " << *strp;
        ret &= SetProperty(kUsbGadgetApexProp, *strp);
        if (selections.usb_gadget_apex == UsbGadgetApex::None) {
            ret &= SetProperty(kUsbAdbDisabledProp, "true");
        }
    } else {
        LOG(WARNING) << "USB Gadget APEX is unset";
        ret &= SetProperty(kUsbAdbDisabledProp, "true");
    }

    if (selections.sf_native_window_buffers_format &&
        selections.sf_native_window_buffers_format != HAL_PIXEL_FORMAT_RGBA_8888) {
        LOG(INFO) << "Set surfaceflinger native window buffers format to "
                  << std::to_string(selections.sf_native_window_buffers_format) << " (no-op)";
    }

    LOG(INFO) << "Set OpenGLES version to " << std::to_string(selections.gles_version);
    ret &= SetProperty(kGlesVersionProp, std::to_string(selections.gles_version));

    if (selections.hw_egl != HwEgl::Unset) {
        strp = &kHwEglMap.at(selections.hw_egl);
        LOG(INFO) << "Set EGL to " << *strp;
        ret &= SetProperty(kHwEglProp, *strp);
    } else {
        LOG(WARNING) << "EGL is unset";
    }

    if (selections.minigbm_generic_backend != MinigbmGenericBackend::Unset) {
        strp = &kMinigbmGenericBackendMap.at(selections.minigbm_generic_backend);
        LOG(INFO) << "Set minigbm generic backend to " << *strp;
        ret &= SetProperty(kMinigbmGenericBackendProp, *strp);
    } else {
        LOG(WARNING) << "Minigbm generic backend is unset";
    }

    switch (selections.vulkan_apex) {
        case VulkanApex::Unset:
            selections.vulkan_apex = VulkanApex::No_apex;
            break;
        case VulkanApex::No_apex:
            break;
        case VulkanApex::Swiftshader:
            // Swiftshader APEX will set vulkan module by itself
            selections.hw_vulkan = HwVulkan::Unset;
            break;
    }

    if (selections.hw_vulkan != HwVulkan::Unset) {
        strp = &kHwVulkanMap.at(selections.hw_vulkan);
        LOG(INFO) << "Set Vulkan module to " << *strp;
        ret &= SetProperty(kHwVulkanProp, *strp);
    } else {
        LOG(WARNING) << "Vulkan module is unset";
    }

    if (selections.vulkan_apex != VulkanApex::Unset) {
        strp = &kVulkanApexMap.at(selections.vulkan_apex);
        LOG(INFO) << "Set Vulkan APEX to " << *strp;
        ret &= SetProperty(kVulkanApexProp, *strp);
    } else {
        LOG(WARNING) << "Vulkan APEX is unset";
    }

    if (selections.gralloc_apex != GrallocApex::Unset) {
        strp = &kGrallocApexMap.at(selections.gralloc_apex);
        LOG(INFO) << "Set Gralloc APEX to " << *strp;
        ret &= SetProperty(kGrallocApexProp, *strp);
    } else {
        LOG(WARNING) << "Gralloc APEX is unset";
    }

    if (selections.hwc_apex != HwcApex::Unset) {
        strp = &kHwcApexMap.at(selections.hwc_apex);
        LOG(INFO) << "Set HWComposer APEX to " << *strp;
        ret &= SetProperty(kHwcApexProp, *strp);
    } else {
        LOG(WARNING) << "HWComposer APEX is unset";
    }

    // Enablue blur if not using Swiftshader graphics
    if (selections.vulkan_apex != VulkanApex::Swiftshader) {
        LOG(INFO) << "Enable blur";
        ret &= SetProperty(kSfSupportsBackgroundBlurProp, "1");
    }

    if (!ret) LOG(ERROR) << __FUNCTION__ << "(): Failed to set some properties";

    return ret;
}

}  // namespace hardware_detect
