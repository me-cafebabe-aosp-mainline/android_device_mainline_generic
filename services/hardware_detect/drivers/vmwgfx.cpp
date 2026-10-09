/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <cstdint>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>

#include <vmwgfx_drm.h>
#include <xf86drm.h>

#include "drivers.h"

namespace hardware_detect {

namespace {

// Missing from libdrm's copy of vmwgfx_drm.h
#ifndef DRM_VMW_PARAM_GL43
#define DRM_VMW_PARAM_GL43 16
#endif

bool GetVmwgfxParam(int fd, uint32_t param, uint64_t* value) {
    struct drm_vmw_getparam_arg get_param = {
            .param = param,
    };
    if (drmCommandWriteRead(fd, DRM_VMW_GET_PARAM, &get_param, sizeof(get_param))) return false;
    *value = get_param.value;
    return true;
}

}  // namespace

// Mesa svga picks the GLSL level by the host shader model:
// vgpu9 (no SM4) is GL 2.1, vgpu10 (SM4 / SM4.1) is GL 3.3, SM5 is GL 4.1
// and GL43 is GL 4.3. Unlike vgpu9, vgpu10 and SM5 hosts can do GLES 3.0,
// and GL43 hosts can do GLES 3.1.
void DrmVmwgfxRender(Selections& selections, const DrmDevice& render) {
    uint64_t has_3d = 0, has_dx = 0, has_gl43 = 0;

    if (!GetVmwgfxParam(render.fd, DRM_VMW_PARAM_3D, &has_3d)) {
        LOG(ERROR) << "Failed to get 3D parameter from vmwgfx";
    }
    if (!has_3d) {
        LOG(WARNING) << "vmwgfx does not have 3D support";
        selections.UseSwiftshaderGraphics();
        return;
    }

    // These fail on old kernels, and they are all unsupported there
    GetVmwgfxParam(render.fd, DRM_VMW_PARAM_DX, &has_dx);
    GetVmwgfxParam(render.fd, DRM_VMW_PARAM_GL43, &has_gl43);
    LOG(INFO) << "vmwgfx 3D, DX = " << has_dx << " GL43 = " << has_gl43;

    if (has_gl43) {
        selections.gles_version = kGlesVersion31;
    } else if (has_dx) {
        selections.gles_version = kGlesVersion30;
    } else {
        selections.gles_version = kGlesVersion20;
    }
}

}  // namespace hardware_detect
