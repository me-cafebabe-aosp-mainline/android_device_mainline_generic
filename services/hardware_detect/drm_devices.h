/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <android-base/unique_fd.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

namespace hardware_detect {

struct DrmDevice {
    std::string path;
    bool is_render = false;
    android::base::unique_fd fd;
    std::string name;

    drmDevicePtr drm_device = nullptr;

    // card specific
    bool have_connector_connected = false;
    bool is_sysfb = false;
    std::string card_have_render_path;
    drmModeRes* drm_mode_res = nullptr;
};

struct DrmDevices {
    std::vector<DrmDevice> cards;
    std::vector<DrmDevice> renders;
};

// Both devices are on the platform bus: an SoC, where the display and the GPU
// are separate DRM devices (Apple, Allwinner, Rockchip...) that share memory.
bool IsSocDrmDevicePair(const DrmDevice& a, const DrmDevice& b);

// Open and probe every DRM node. A card gets the path of its render if it
// has one.
DrmDevices EnumerateDrmDevices();

// Select the DRM card and its render. The other DRM devices are closed.
std::pair<std::optional<DrmDevice>, std::optional<DrmDevice>> GetDrmCardRenderDevicePair();

void CleanupDrmDevice(DrmDevice& dev);

}  // namespace hardware_detect
