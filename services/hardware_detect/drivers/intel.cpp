/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <cstdint>
#include <optional>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>
#include <android-base/properties.h>

#include <i915_drm.h>
#include <xf86drm.h>

#include "drivers.h"
#include "intel_pci_ids.h"

using namespace android::base;

namespace hardware_detect {

namespace {

constexpr int kIntelGen2 = 2;
constexpr int kIntelGen5 = 5;
constexpr int kIntelGen6 = 6;
constexpr int kIntelGen7 = 7;
constexpr int kIntelGen8 = 8;
constexpr int kIntelGen9 = 9;

int GetIntelGen(int device_id) {
    auto it = intel_ids::kPreGen9.find(static_cast<uint16_t>(device_id));
    return it == intel_ids::kPreGen9.end() ? kIntelGen9 : it->second;
}

// Render side settings, according to what Mesa supports on each generation
void SetupIntelRender(Selections& selections, int gen) {
    if (gen <= kIntelGen2) {
        // No hardware driver in Mesa
        selections.UseSwiftshaderGraphics();
    } else if (gen <= kIntelGen5) {
        // i915 (Gen 3) / crocus (Gen 4~5): GLES 2.0
        selections.gles_version = kGlesVersion20;
    } else if (gen == kIntelGen6) {
        // crocus: GLES 3.0
        selections.gles_version = kGlesVersion30;
    } else if (gen <= kIntelGen7) {
        // crocus: GLES 3.1
        selections.gles_version = kGlesVersion31;
        selections.hw_vulkan = HwVulkan::Intel_hasvk;
    } else if (gen == kIntelGen8) {
        selections.gles_version = kGlesVersion32;
        selections.hw_vulkan = HwVulkan::Intel_hasvk;
    } else {
        selections.gles_version = kGlesVersion32;
        selections.hw_vulkan = HwVulkan::Intel;
    }
}

// Bay Trail (VLV), see INTEL_VLV_IDS in the kernel's include/drm/intel/pciids.h
bool IsIntelBayTrail(int device_id) {
    return device_id >= 0x0f30 && device_id <= 0x0f33;
}

// Card side settings, for display
void SetupIntelCard(int device_id, int gen) {
    // From Intel Core to pre-Skylake (HD Graphics 510)
    // Except for Atom Processor Z36xxx/Z37xxx (Bay Trail)
    if (gen < kIntelGen9 && !IsIntelBayTrail(device_id)) {
        SetProperty("ro.vendor.hwc.drm.avoid_using_alpha_bits_for_framebuffer", "1");
        SetProperty("ro.vendor.hwc.drm.disable_planes", "1");
    }
}

void DrmI915(Selections& selections, int fd, bool is_render) {
    int value = 0;
    drm_i915_getparam_t get_param = {
            .param = I915_PARAM_CHIPSET_ID,
            .value = &value,
    };

    if (drmIoctl(fd, DRM_IOCTL_I915_GETPARAM, &get_param)) {
        LOG(ERROR) << "Failed to get I915_PARAM_CHIPSET_ID";
        if (is_render) SetupIntelRender(selections, kIntelGen9);
        return;
    }

    const int gen = GetIntelGen(value);
    LOG(INFO) << "Intel device ID 0x" << std::hex << value << std::dec << " is Gen " << gen
              << (gen >= kIntelGen9 ? "+" : "");

    if (is_render) {
        SetupIntelRender(selections, gen);
    } else {
        SetupIntelCard(value, gen);
    }
}

}  // namespace

void DrmI915Card(Selections& selections, const DrmDevice& card, const std::optional<DrmDevice>&) {
    DrmI915(selections, card.fd.get(), false);
}

void DrmI915Render(Selections& selections, const DrmDevice& render, const DrmDevice&) {
    DrmI915(selections, render.fd.get(), true);
}

// The Xe driver supports only Tiger Lake and newer
void DrmXeRender(Selections& selections, const DrmDevice&, const DrmDevice&) {
    SetupIntelRender(selections, kIntelGen9);
}

}  // namespace hardware_detect
