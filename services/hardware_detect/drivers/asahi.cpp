/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include "drivers.h"

namespace hardware_detect {

void DrmAsahiRender(Selections& selections, const DrmDevice&, const DrmDevice&) {
    selections.gles_version = kGlesVersion32;
    selections.gralloc_apex = GrallocApex::GbmMesa;
    selections.hw_vulkan = HwVulkan::Asahi;
}

}  // namespace hardware_detect
