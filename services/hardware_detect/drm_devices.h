/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#pragma once

#include <optional>
#include <string>
#include <utility>

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

// Select the DRM card and its render. The other DRM nodes are hidden by
// replacing them with symlinks to /dev/null.
std::pair<std::optional<DrmDevice>, std::optional<DrmDevice>> GetDrmCardRenderDevicePair();

void CleanupDrmDevice(DrmDevice& dev);

}  // namespace hardware_detect
