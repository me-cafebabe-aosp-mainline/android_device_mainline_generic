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
void DetectDrmRender(Selections& selections, const DrmDevice& render, const DrmDevice& card);

// Whether the render can do hardware rendering. A render that cannot is
// used with software rendering, while its card still displays.
bool IsDrmRenderUsable(const DrmDevice& render);

using DrmCardHandler = void (*)(Selections& selections, const DrmDevice& card,
                                const std::optional<DrmDevice>& render);
using DrmRenderHandler = void (*)(Selections& selections, const DrmDevice& render,
                                  const DrmDevice& card);
using DrmRenderUsableCheck = bool (*)(const DrmDevice& render);

// generic.cpp
void DrmBasicCard(Selections& selections);
void DrmUnknownCard(Selections& selections, const std::string& card_name);
void DrmUnknownRender(Selections& selections, const std::string& render_name);

// amd.cpp
void DrmAmdgpuRender(Selections& selections, const DrmDevice& render, const DrmDevice& card);
void DrmRadeonRender(Selections& selections, const DrmDevice& render, const DrmDevice& card);

// asahi.cpp
void DrmAsahiRender(Selections& selections, const DrmDevice& render, const DrmDevice& card);

// intel.cpp
void DrmI915Card(Selections& selections, const DrmDevice& card,
                 const std::optional<DrmDevice>& render);
bool DrmI915RenderUsable(const DrmDevice& render);
void DrmI915Render(Selections& selections, const DrmDevice& render, const DrmDevice& card);
void DrmXeRender(Selections& selections, const DrmDevice& render, const DrmDevice& card);

// msm.cpp
void DrmMsmCard(Selections& selections, const DrmDevice& card,
                const std::optional<DrmDevice>& render);
void DrmMsmRender(Selections& selections, const DrmDevice& render, const DrmDevice& card);

// nouveau.cpp
bool DrmNouveauRenderUsable(const DrmDevice& render);
void DrmNouveauRender(Selections& selections, const DrmDevice& render, const DrmDevice& card);

// virtio_gpu.cpp
void DrmVirtiogpuCard(Selections& selections, const DrmDevice& card,
                      const std::optional<DrmDevice>& render);
bool DrmVirtiogpuRenderUsable(const DrmDevice& render);
void DrmVirtiogpuRender(Selections& selections, const DrmDevice& render, const DrmDevice& card);

// vmwgfx.cpp
bool DrmVmwgfxRenderUsable(const DrmDevice& render);
void DrmVmwgfxRender(Selections& selections, const DrmDevice& render, const DrmDevice& card);

}  // namespace hardware_detect
