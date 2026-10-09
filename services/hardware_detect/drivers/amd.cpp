/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <algorithm>
#include <cstdint>
#include <iterator>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>

#include <xf86drm.h>

#include "drivers.h"
#include "radeon_pci_ids.h"

namespace hardware_detect {

namespace {

template <size_t N>
bool RadeonIdsContain(const uint16_t (&ids)[N], uint16_t id) {
    return std::binary_search(std::begin(ids), std::end(ids), id);
}

}  // namespace

void DrmAmdgpuRender(Selections& selections, const DrmDevice&) {
    selections.gles_version = kGlesVersion32;
    selections.gralloc_apex = GrallocApex::GbmMesa;
    selections.hw_vulkan = HwVulkan::Radeon;
}

// Mesa supports the radeon kernel driver with r300g, r600g and radeonsi.
// radv requires the amdgpu kernel driver, so there is no Vulkan.
void DrmRadeonRender(Selections& selections, const DrmDevice& render) {
    selections.gralloc_apex = GrallocApex::GbmMesa;

    if (render.drm_device->bustype != DRM_BUS_PCI) {
        selections.gles_version = kGlesVersion31;
        return;
    }

    const uint16_t id = render.drm_device->deviceinfo.pci->device_id;
    LOG(INFO) << "Radeon device ID is 0x" << std::hex << id;

    if (RadeonIdsContain(radeon_ids::kSouthernIslands, id)) {
        selections.gles_version = kGlesVersion32;
    } else if (RadeonIdsContain(radeon_ids::kEvergreen, id)) {
        selections.gles_version = kGlesVersion31;
    } else if (RadeonIdsContain(radeon_ids::kR600, id)) {
        selections.gles_version = kGlesVersion30;
    } else if (RadeonIdsContain(radeon_ids::kR300, id)) {
        selections.gles_version = kGlesVersion20;
    } else {
        LOG(WARNING) << "Unknown Radeon device ID";
        selections.gles_version = kGlesVersion31;
    }
}

}  // namespace hardware_detect
