/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include "drivers.h"

namespace hardware_detect {

void DrmAmdgpuRender(Selections& selections, const DrmDevice&) {
    selections.gles_version = kGlesVersion32;
    selections.gralloc_apex = GrallocApex::GbmMesa;
    selections.hw_vulkan = HwVulkan::Radeon;
}

void DrmRadeonRender(Selections& selections, const DrmDevice&) {
    selections.gles_version = kGlesVersion31;
    selections.gralloc_apex = GrallocApex::GbmMesa;
}

}  // namespace hardware_detect
