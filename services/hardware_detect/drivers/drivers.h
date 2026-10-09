/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#pragma once

#include <optional>
#include <string>

#include "../drm_devices.h"
#include "../selections.h"

namespace hardware_detect {

// Apply the selections and workarounds for the selected DRM card/render
// pair. Each driver provides a card handler and/or a render handler, which
// are dispatched by DRM driver name.
void DetectDrmCard(Selections& selections, const DrmDevice& card,
                   const std::optional<DrmDevice>& render);
void DetectDrmRender(Selections& selections, const DrmDevice& render);

using DrmCardHandler = void (*)(Selections& selections, const DrmDevice& card,
                                const std::optional<DrmDevice>& render);
using DrmRenderHandler = void (*)(Selections& selections, const DrmDevice& render);

// generic.cpp
void DrmBasicCard(Selections& selections);
void DrmUnknownCard(Selections& selections, const std::string& card_name);
void DrmUnknownRender(Selections& selections, const std::string& render_name);

// amd.cpp
void DrmAmdgpuRender(Selections& selections, const DrmDevice& render);
void DrmRadeonRender(Selections& selections, const DrmDevice& render);

// asahi.cpp
void DrmAsahiRender(Selections& selections, const DrmDevice& render);

// intel.cpp
void DrmI915Card(Selections& selections, const DrmDevice& card,
                 const std::optional<DrmDevice>& render);
void DrmI915Render(Selections& selections, const DrmDevice& render);
void DrmXeRender(Selections& selections, const DrmDevice& render);

// msm.cpp
void DrmMsmCard(Selections& selections, const DrmDevice& card,
                const std::optional<DrmDevice>& render);
void DrmMsmRender(Selections& selections, const DrmDevice& render);

// nouveau.cpp
void DrmNouveauRender(Selections& selections, const DrmDevice& render);

// virtio_gpu.cpp
void DrmVirtiogpuCard(Selections& selections, const DrmDevice& card,
                      const std::optional<DrmDevice>& render);
void DrmVirtiogpuRender(Selections& selections, const DrmDevice& render);

// vmwgfx.cpp
void DrmVmwgfxRender(Selections& selections, const DrmDevice& render);

}  // namespace hardware_detect
