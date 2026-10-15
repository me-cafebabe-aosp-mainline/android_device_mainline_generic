/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <cstdint>
#include <optional>
#include <string>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>

#include <virtgpu_drm.h>
#include <xf86drm.h>

#include "drivers.h"

namespace hardware_detect {

namespace {

void DrmVirtiogpu(Selections& selections, int fd, bool is_render, const std::string& render_name) {
    bool value_3d_features = false;
    int ret = 0;

    uint32_t value;
    struct drm_virtgpu_getparam get_param = {
            .value = (uint64_t)(uintptr_t)&value,
    };

    get_param.param = VIRTGPU_PARAM_3D_FEATURES;
    ret = drmIoctl(fd, DRM_IOCTL_VIRTGPU_GETPARAM, &get_param);
    if (!ret) {
        value_3d_features = !!value;
    } else {
        LOG(ERROR) << "Failed to get 3D features parameter from virtio_gpu";
    }

    if (render_name.empty() || (render_name == "virtio_gpu" && !value_3d_features)) {
        selections.sf_native_window_buffers_format = HAL_PIXEL_FORMAT_BGRA_8888;
    }

    if (is_render) {
        if (value_3d_features) {
            selections.hw_egl = HwEgl::Mesa;
            selections.hw_vulkan = HwVulkan::Virtio;
            selections.gles_version = kGlesVersion32;
        } else {
            selections.UseSwiftshaderGraphics();
        }
    }
}

}  // namespace

void DrmVirtiogpuCard(Selections& selections, const DrmDevice& card,
                      const std::optional<DrmDevice>& render) {
    DrmVirtiogpu(selections, card.fd.get(), false, render.has_value() ? render.value().name : "");
}

void DrmVirtiogpuRender(Selections& selections, const DrmDevice& render, const DrmDevice&) {
    DrmVirtiogpu(selections, render.fd.get(), true, render.name);
}

}  // namespace hardware_detect
