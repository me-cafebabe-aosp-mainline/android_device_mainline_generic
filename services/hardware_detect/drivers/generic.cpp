/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <string>
#include <unordered_map>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>

#include "drivers.h"

namespace hardware_detect {

void DrmBasicCard(Selections& selections) {
    selections.hwc_apex = HwcApex::DrmFb;
}

void DrmUnknownCard(Selections& selections, const std::string& card_name) {
    LOG(WARNING) << "DRM card is not directly supported";

    const std::unordered_map<std::string, HwVulkan> kCardNameToHwVulkanMap = {
            // TODO: HwVulkan::Imagination require matching card&render pair, see pvr_drm_configs[]
            // in mesa
            {"mediatek", HwVulkan::Imagination},
            {"tidss", HwVulkan::Imagination},
    };
    if (kCardNameToHwVulkanMap.contains(card_name)) {
        selections.hw_vulkan = kCardNameToHwVulkanMap.at(card_name);
    }
}

void DrmUnknownRender(Selections& selections, const std::string& render_name) {
    LOG(WARNING) << "DRM render is not directly supported";

    selections.gralloc_apex = GrallocApex::GbmMesa;

    const std::unordered_map<std::string, HwVulkan> kRenderNameToHwVulkanMap = {
            {"panfrost", HwVulkan::Panfrost},
            {"panthor", HwVulkan::Panfrost},
            {"v3d", HwVulkan::Broadcom},
    };
    if (kRenderNameToHwVulkanMap.contains(render_name)) {
        selections.hw_vulkan = kRenderNameToHwVulkanMap.at(render_name);
    }
}

}  // namespace hardware_detect
