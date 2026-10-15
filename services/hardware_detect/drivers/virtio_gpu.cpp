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

bool HasVirtiogpu3d(int fd) {
    uint32_t value;
    struct drm_virtgpu_getparam get_param = {
            .param = VIRTGPU_PARAM_3D_FEATURES,
            .value = (uint64_t)(uintptr_t)&value,
    };

    if (drmIoctl(fd, DRM_IOCTL_VIRTGPU_GETPARAM, &get_param)) {
        LOG(ERROR) << "Failed to get 3D features parameter from virtio_gpu";
        return false;
    }
    return !!value;
}

}  // namespace

bool DrmVirtiogpuRenderUsable(const DrmDevice& render) {
    return HasVirtiogpu3d(render.fd.get());
}

// Software rendering needs BGRA buffers if the virtio_gpu display cannot do 3D
void DrmVirtiogpuCard(Selections& selections, const DrmDevice& card,
                      const std::optional<DrmDevice>& render) {
    if (!render.has_value() ||
        (render.value().name == "virtio_gpu" && !HasVirtiogpu3d(card.fd.get()))) {
        selections.sf_native_window_buffers_format = HAL_PIXEL_FORMAT_BGRA_8888;
    }
}

void DrmVirtiogpuRender(Selections& selections, const DrmDevice&, const DrmDevice&) {
    selections.hw_egl = HwEgl::Mesa;
    selections.hw_vulkan = HwVulkan::Virtio;
    selections.gles_version = kGlesVersion32;
}

}  // namespace hardware_detect
