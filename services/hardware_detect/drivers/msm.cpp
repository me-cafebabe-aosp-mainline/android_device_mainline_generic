/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <optional>
#include <string>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>
#include <android-base/properties.h>

extern "C" {
#include <freedreno_drmif.h>
}

#include "drivers.h"

using namespace android::base;

namespace hardware_detect {

void DrmMsmCard(Selections& selections, const DrmDevice& card, const std::optional<DrmDevice>&) {
    const int card_fd = card.fd;
    selections.hw_vulkan = HwVulkan::Freedreno;

    if (IsForcedSwiftshader()) {
        SetProperty("vendor.minigbm.debug", "nocompression");
    }

    // device/mainline/qcom-common/services/msm_drm_quirks begin
    struct fd_device* dev;
    struct fd_pipe* pipe;

    dev = fd_device_new(card_fd);
    if (!dev) {
        LOG(ERROR) << "fd_device_new() failed";
        goto err_fd_device_new;
    }

    pipe = fd_pipe_new(dev, FD_PIPE_3D);
    if (!pipe) {
        LOG(ERROR) << "fd_pipe_new() failed";
        goto err_fd_pipe_new;
    }

    uint64_t chip_id, gpu_id;
    fd_pipe_get_param(pipe, FD_CHIP_ID, &chip_id);
    fd_pipe_get_param(pipe, FD_GPU_ID, &gpu_id);
    LOG(INFO) << __FUNCTION__ << ": chip_id = " << std::to_string(chip_id)
              << " gpu_id = " << std::to_string(gpu_id);

    // Adreno 5xx Mesa Freedreno quirks
    if (gpu_id >= 500 && gpu_id <= 599) {
        SetProperty("vendor.mesa.fd.mesa.debug", "sysmem");
    }

    // Let minigbm avoid UBWC for pre Adreno 6xx
    if (gpu_id < 600) {
        SetProperty("vendor.minigbm.avoid_ubwc", "true");
    }

    fd_pipe_del(pipe);
err_fd_pipe_new:
    fd_device_del(dev);
err_fd_device_new:
    // device/mainline/qcom-common/services/msm_drm_quirks end
    return;
}

}  // namespace hardware_detect
