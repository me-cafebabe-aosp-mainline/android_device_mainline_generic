/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <optional>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>
#include <android-base/properties.h>

#include <i915_drm.h>
#include <xf86drm.h>

#include "drivers.h"

using namespace android::base;

namespace hardware_detect {

namespace {

void DrmI915(Selections& selections, int fd, bool is_render) {
    int ret = 0;

    if (is_render) {
        selections.gles_version = kGlesVersion32;
        selections.hw_vulkan = HwVulkan::Intel;  // May get overridden later
    }

    int value;
    drm_i915_getparam_t get_param = {
            .value = &value,
    };

    get_param.param = I915_PARAM_CHIPSET_ID;
    ret = drmIoctl(fd, DRM_IOCTL_I915_GETPARAM, &get_param);
    if (!ret) {
        // Enable various workarounds
        /*
         * If the determination gets more complicated in future,
         * We can consider using minigbm's i915_info_from_device_id()
         */
        if (!is_render && (value < 0x1902 && value != 0x0f31)) {
            // From Intel Core to pre-Skylake (HD Graphics 510)
            // Except for Atom Processor Z36xxx/Z37xxx
            SetProperty("ro.vendor.hwc.drm.avoid_using_alpha_bits_for_framebuffer", "1");
            SetProperty("ro.vendor.hwc.drm.disable_planes", "1");
        }
        if (is_render && (value <= 0x0F33 || (value >= 0x1602 && value <= 0x162E) ||
                          (value >= 0x22B0 && value <= 0x22B3))) {
            // Approximate of gen7_ids and gen8_ids according to minigbm/i915.c
            selections.hw_vulkan = HwVulkan::Intel_hasvk;
        }
        // What about pre Intel Core? Those won't even boot...
    } else {
        LOG(ERROR) << "Failed to get I915_PARAM_CHIPSET_ID";
    }
}

}  // namespace

void DrmI915Card(Selections& selections, const DrmDevice& card, const std::optional<DrmDevice>&) {
    DrmI915(selections, card.fd, false);
}

void DrmI915Render(Selections& selections, const DrmDevice& render) {
    DrmI915(selections, render.fd, true);
}

}  // namespace hardware_detect
