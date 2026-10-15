/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <cstdint>
#include <optional>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>
#include <android-base/properties.h>

#include <xf86drm.h>

#include "drivers.h"

using namespace android::base;

namespace hardware_detect {

namespace {

// From msm_drm.h, which is not usable from C++ (member named "or").
// libdrm_freedreno is not used here because it refuses GPUs that report
// MSM_PARAM_GPU_ID (revn) as 0, which is every newer Adreno that only
// identifies by chip ID (e.g. A612, A702, A7xx).
constexpr unsigned long kDrmMsmGetParam = 0x00;
constexpr uint32_t kMsmPipe3d0 = 0x10;
constexpr uint32_t kMsmParamGpuId = 0x01;
constexpr uint32_t kMsmParamChipId = 0x03;

struct MsmParam {
    uint32_t pipe;
    uint32_t param;
    uint64_t value;
    uint32_t len;
    uint32_t pad;
};

bool GetMsmParam(int fd, uint32_t param, uint64_t* value) {
    MsmParam get_param = {
            .pipe = kMsmPipe3d0,
            .param = param,
    };
    if (drmCommandWriteRead(fd, kDrmMsmGetParam, &get_param, sizeof(get_param))) return false;
    *value = get_param.value;
    return true;
}

struct MsmGpu {
    uint32_t series;  // Adreno xxx, divided by 100. 0 if unknown
    bool is_a702;
};

// Chip ID is 0xCCMMNNPP (core, major, minor, patch), and it has the speed bin
// in the upper 32 bits. GPU ID is the model number if the kernel knows it.
// This agrees with fd_dev_gpu_id() in Mesa.
MsmGpu GetMsmGpu(int fd) {
    uint64_t chip_id = 0, gpu_id = 0;
    const bool have_chip_id = GetMsmParam(fd, kMsmParamChipId, &chip_id);
    const bool have_gpu_id = GetMsmParam(fd, kMsmParamGpuId, &gpu_id);
    if (!have_chip_id && !have_gpu_id) {
        LOG(ERROR) << "Failed to get MSM GPU parameters";
        return {};
    }

    chip_id &= UINT32_MAX;
    if (!gpu_id && chip_id) {
        gpu_id = ((chip_id >> 24) & 0xff) * 100 + ((chip_id >> 16) & 0xff) * 10 +
                 ((chip_id >> 8) & 0xff);
    }
    LOG(INFO) << __FUNCTION__ << ": chip_id = 0x" << std::hex << chip_id << std::dec
              << " gpu_id = " << gpu_id;

    if (!gpu_id) return {};
    return {.series = static_cast<uint32_t>(gpu_id / 100), .is_a702 = gpu_id == 702};
}

}  // namespace

void DrmMsmCard(Selections&, const DrmDevice& card, const std::optional<DrmDevice>&) {
    const MsmGpu gpu = GetMsmGpu(card.fd.get());
    if (!gpu.series) return;

    // device/mainline/qcom-common/services/msm_drm_quirks begin
    if (IsForcedSwiftshader()) {
        SetProperty("vendor.minigbm.debug", "nocompression");
    }

    // Adreno 5xx Mesa Freedreno quirks
    if (gpu.series == 5) {
        SetProperty("vendor.mesa.fd.mesa.debug", "sysmem");
    }

    // Let minigbm avoid UBWC for pre Adreno 6xx
    if (gpu.series < 6) {
        SetProperty("vendor.minigbm.avoid_ubwc", "true");
    }
    // device/mainline/qcom-common/services/msm_drm_quirks end
}

void DrmMsmRender(Selections& selections, const DrmDevice& render, const DrmDevice&) {
    const MsmGpu gpu = GetMsmGpu(render.fd.get());
    if (!gpu.series) return;

    // Per Mesa freedreno (essl_feature_level)
    if (gpu.is_a702) {
        // No GS/tessellation
        selections.gles_version = kGlesVersion31;
    } else if (gpu.series >= 4) {
        selections.gles_version = kGlesVersion32;
    } else if (gpu.series == 3) {
        selections.gles_version = kGlesVersion30;
    } else {
        selections.gles_version = kGlesVersion20;
    }

    // Turnip supports Adreno 6xx and newer
    if (gpu.series >= 6) selections.hw_vulkan = HwVulkan::Freedreno;
}

}  // namespace hardware_detect
