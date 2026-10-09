/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include "drivers.h"

#include <set>
#include <string>
#include <unordered_map>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>

namespace hardware_detect {

namespace {

const std::set<std::string> kBasicDrmCards = {
        "bochs-drm", "gud", "hyperv_drm", "qxl", "udl", "vboxvideo",
};

const std::set<std::string> kMustUseFbDisplayCards = {};

// Drivers that need nothing on card or render side are not listed here,
// but they are still recognized so that they won't be treated as unknown.
const std::unordered_map<std::string, DrmCardHandler> kDrmCardHandlers = {
        {"amdgpu", nullptr},
        // Not checking with "asahi" here, because it seems like it appears when simpledrm is
        // enabled, and does not work with 3D graphics
        {"apple", nullptr},
        {"i915", DrmI915Card},
        {"msm", DrmMsmCard},
        {"nouveau", nullptr},
        {"radeon", nullptr},
        {"virtio_gpu", DrmVirtiogpuCard},
        {"vmwgfx", nullptr},
        {"xe", nullptr},
};

const std::unordered_map<std::string, DrmRenderHandler> kDrmRenderHandlers = {
        {"amdgpu", DrmAmdgpuRender}, {"apple", DrmAsahiRender},
        {"asahi", DrmAsahiRender},   {"i915", DrmI915Render},
        {"msm", DrmMsmRender},       {"nouveau", DrmNouveauRender},
        {"radeon", DrmRadeonRender}, {"virtio_gpu", DrmVirtiogpuRender},
        {"vmwgfx", DrmVmwgfxRender}, {"xe", DrmXeRender},
};

}  // namespace

void DetectDrmCard(Selections& selections, const DrmDevice& card,
                   const std::optional<DrmDevice>& render) {
    if (kMustUseFbDisplayCards.contains(card.name)) {
        LOG(INFO) << "This DRM card must use framebuffer display for now";
        selections.SetupFramebufferDisplay();
    } else if (kBasicDrmCards.contains(card.name) || card.is_sysfb) {
        LOG(INFO) << "This DRM card is basic";
        DrmBasicCard(selections);
    } else if (auto it = kDrmCardHandlers.find(card.name); it != kDrmCardHandlers.end()) {
        if (it->second) it->second(selections, card, render);
    } else {
        DrmUnknownCard(selections, card.name);
    }
}

void DetectDrmRender(Selections& selections, const DrmDevice& render) {
    if (selections.uses_sw_rendered_graphics) {
        // Nothing to do
        return;
    }

    if (auto it = kDrmRenderHandlers.find(render.name); it != kDrmRenderHandlers.end()) {
        if (it->second) it->second(selections, render);
    } else {
        DrmUnknownRender(selections, render.name);
    }
}

}  // namespace hardware_detect
