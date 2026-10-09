/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <cstdint>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>

#include <xf86drm.h>

#include "drivers.h"

namespace hardware_detect {

namespace {

// From nouveau_drm.h, which is not usable from C++ (member named "class")
constexpr unsigned long kDrmNouveauGetparam = 0x00;
constexpr uint64_t kNouveauGetparamChipsetId = 11;

struct NouveauGetparam {
    uint64_t param;
    uint64_t value;
};

// Chipset ID as in nouveau's NV_DEVICE_INFO (e.g. 0x117 for GM107)
constexpr uint32_t kNouveauChipsetNv30 = 0x30;  // Rankine, no gallium driver before this
constexpr uint32_t kNouveauChipsetNv50 = 0x50;  // Tesla
constexpr uint32_t kNouveauChipsetNvc0 = 0xc0;  // Fermi
constexpr uint32_t kNouveauChipsetNve0 = 0xe0;  // Kepler, minimum of NVK

}  // namespace

void DrmNouveauRender(Selections& selections, const DrmDevice& render) {
    selections.gralloc_apex = GrallocApex::GbmMesa;

    NouveauGetparam get_param = {
            .param = kNouveauGetparamChipsetId,
    };
    if (drmCommandWriteRead(render.fd.get(), kDrmNouveauGetparam, &get_param, sizeof(get_param))) {
        LOG(ERROR) << "Failed to get nouveau chipset ID";
        selections.gles_version = kGlesVersion31;
        return;
    }

    const uint32_t chipset = static_cast<uint32_t>(get_param.value);
    LOG(INFO) << "Nouveau chipset ID is 0x" << std::hex << chipset;

    if (chipset < kNouveauChipsetNv30) {
        // Mesa has no driver for NV04 ~ NV20
        selections.UseSwiftshaderGraphics();
    } else if (chipset < kNouveauChipsetNv50) {
        // nv30 (GL 2.1)
        selections.gles_version = kGlesVersion20;
    } else if (chipset < kNouveauChipsetNvc0) {
        // nv50 (GL 3.3)
        selections.gles_version = kGlesVersion30;
    } else if (chipset < kNouveauChipsetNve0) {
        // nvc0 on Fermi
        selections.gles_version = kGlesVersion31;
    } else {
        // nvc0 on Kepler and newer
        selections.gles_version = kGlesVersion31;
        selections.hw_vulkan = HwVulkan::Nouveau;
    }
}

}  // namespace hardware_detect
