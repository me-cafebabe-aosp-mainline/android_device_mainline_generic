/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <string>

#include <android-base/file.h>

#include "drivers.h"

using namespace android::base;

namespace hardware_detect {

namespace {

const std::string kDmiIdPath = "/sys/devices/virtual/dmi/id/";

}  // namespace

void DrmVmwgfxRender(Selections& selections, const DrmDevice&) {
    std::string smbios_product_name;
    ReadFileToString(kDmiIdPath + "product_name", &smbios_product_name);
    if (!smbios_product_name.empty()) smbios_product_name.pop_back();

    if (smbios_product_name == "VirtualBox") {
        selections.gles_version = kGlesVersion30;
    } else {
        selections.gles_version = kGlesVersion31;
    }
}

}  // namespace hardware_detect
