/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include "drivers.h"

namespace hardware_detect {

void DrmNouveauRender(Selections& selections, const DrmDevice&) {
    selections.gles_version = kGlesVersion31;
    selections.gralloc_apex = GrallocApex::GbmMesa;
    selections.hw_vulkan = HwVulkan::Nouveau;
}

}  // namespace hardware_detect
