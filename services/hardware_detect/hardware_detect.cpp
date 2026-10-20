/*
  SPDX-FileCopyrightText: The LineageOS Project
  SPDX-License-Identifier: Apache-2.0
*/

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <optional>
#include <string>
#include <utility>

#define LOG_TAG "hardware_detect"
#include <android-base/logging.h>
#include <android-base/properties.h>

#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "drivers/drivers.h"
#include "drm_devices.h"
#include "selections.h"

using namespace android::base;
using namespace hardware_detect;

namespace {

constexpr char kGraphicsCardNameProp[] = "ro.vendor.graphics.card.name";
constexpr char kGraphicsRenderNameProp[] = "ro.vendor.graphics.render.name";

constexpr char kHwcDrmDeviceProp[] = "vendor.hwc.drm.device";
constexpr char kGrallocDeviceProp[] = "vendor.gralloc.device";
constexpr char kMinigbmDeviceProp[] = "vendor.minigbm.device";
constexpr char kAvoidAlphaBitsProp[] = "ro.vendor.hwc.drm.avoid_using_alpha_bits_for_framebuffer";
constexpr char kMesaDrmRenderNodeProp[] = "vendor.mesa.drm.render.node";
constexpr char kMesaNoKmsSwrastProp[] = "vendor.mesa.android.no.kms.swrast";
constexpr char kMesaAlwaysSoftwareProp[] = "vendor.mesa.libgl.always.software";
constexpr char kGrallocScanoutLinearProp[] = "vendor.gralloc.scanout_linear";
constexpr char kMinigbmScanoutLinearProp[] = "vendor.minigbm.scanout_linear";
constexpr char kMinigbmAmdgpuLinearOnlyProp[] = "vendor.minigbm.amdgpu.linear_only";

constexpr char kUsbControllerProp[] = "sys.usb.controller";

// What the primary planes of a card can scan out of the framebuffers of Android. HWC uses
// ABGR8888 for RGBA_8888, or XBGR8888 when it is told to avoid the alpha bits.
enum class PrimaryPlaneFormats {
    Unknown,
    None,
    Xbgr,
    Abgr,
};

bool IsPrimaryPlane(int fd, uint32_t plane_id) {
    drmModeObjectProperties* props =
            drmModeObjectGetProperties(fd, plane_id, DRM_MODE_OBJECT_PLANE);
    if (!props) return false;

    bool primary = false;
    for (uint32_t i = 0; i < props->count_props; ++i) {
        drmModePropertyRes* prop = drmModeGetProperty(fd, props->props[i]);
        if (!prop) continue;
        if (strcmp(prop->name, "type") == 0) {
            primary = props->prop_values[i] == DRM_PLANE_TYPE_PRIMARY;
        }
        drmModeFreeProperty(prop);
    }

    drmModeFreeObjectProperties(props);
    return primary;
}

// The result is the formats that all the primary planes support. It needs the universal
// planes, which the atomic client capability enables.
PrimaryPlaneFormats GetPrimaryPlaneFormats(const DrmDevice& card) {
    drmModePlaneRes* planeRes = drmModeGetPlaneResources(card.fd.get());
    if (!planeRes) {
        LOG(ERROR) << "Failed to get plane resources";
        return PrimaryPlaneFormats::Unknown;
    }

    bool found = false, all_abgr = true, all_xbgr = true;

    for (uint32_t i = 0; i < planeRes->count_planes; ++i) {
        drmModePlane* plane = drmModeGetPlane(card.fd.get(), planeRes->planes[i]);
        if (!plane) continue;

        if (IsPrimaryPlane(card.fd.get(), plane->plane_id)) {
            bool abgr = false, xbgr = false;
            for (uint32_t j = 0; j < plane->count_formats; ++j) {
                abgr |= plane->formats[j] == DRM_FORMAT_ABGR8888;
                xbgr |= plane->formats[j] == DRM_FORMAT_XBGR8888;
            }
            found = true;
            all_abgr &= abgr;
            all_xbgr &= xbgr;
        }

        drmModeFreePlane(plane);
    }

    drmModeFreePlaneResources(planeRes);

    if (!found) return PrimaryPlaneFormats::Unknown;
    if (all_abgr) return PrimaryPlaneFormats::Abgr;
    return all_xbgr ? PrimaryPlaneFormats::Xbgr : PrimaryPlaneFormats::None;
}

bool IsDrmMultiplePlanesSupported(const DrmDevice& card) {
    drmModePlaneRes* planeRes = drmModeGetPlaneResources(card.fd.get());
    if (!planeRes) {
        LOG(ERROR) << "Failed to get plane resources";
        return false;
    }

    drmModeRes* res = card.drm_mode_res;

    bool supportsMultiplePlanes = false;

    // For each CRTC, count how many planes can attach
    for (int crtc_idx = 0; crtc_idx < res->count_crtcs; ++crtc_idx) {
        int planeCountForCrtc = 0;

        for (uint32_t i = 0; i < planeRes->count_planes; ++i) {
            drmModePlane* plane = drmModeGetPlane(card.fd.get(), planeRes->planes[i]);
            if (!plane) continue;

            if (plane->possible_crtcs & (1 << crtc_idx)) {
                planeCountForCrtc++;
            }

            drmModeFreePlane(plane);
        }

        if (planeCountForCrtc > 1) {
            supportsMultiplePlanes = true;
            break;
        }
    }

    drmModeFreePlaneResources(planeRes);

    return supportsMultiplePlanes;
}

void SetDefaultsForDrmDisplay(Selections& selections, const DrmDevice& card,
                              const std::optional<DrmDevice>& render) {
    /*
     * Gralloc
     *
     * What else is there other than minigbm-upstream for now?
     * - Unmodified external/minigbm (Quite useless for a generic port)
     * - Standalone Mesa GBM gralloc (Our minigbm currently have Mesa GBM backend)
     * - HBM gralloc (standalone form is still a draft, minigbm backend form is used only for amdgpu
     * for now)
     * - DRM gralloc (RIP)
     */
    selections.gralloc_apex = GrallocApex::Minigbm;

    /*
     * HwComposer
     *
     * drmfb-composer for legacy hardware that does not support
     * certain capabilities, according to its README
     *
     * drm_hwcomposer-upstream for full-fledged support
     * (however we have some hacks in there too)
     *
     * drmfb-composer for a card that displays buffers of another
     * device's render, since drm_hwcomposer would import them on the card.
     * It works on an SoC, where the devices share memory.
     */
    if (selections.hybrid_graphics && !IsSocDrmDevicePair(card, render.value())) {
        selections.hwc_apex = HwcApex::DrmFb;

        // For drm_hwcomposer, if it is selected anyway
        SetProperty(kGrallocScanoutLinearProp, "true");
        SetProperty(kMinigbmScanoutLinearProp, "true");
    } else if (!drmSetClientCap(card.fd.get(), DRM_CLIENT_CAP_ATOMIC, 1)) {
        selections.hwc_apex = HwcApex::Drm;

        // A card that cannot scan out the framebuffers cannot be used by drm_hwcomposer
        switch (GetPrimaryPlaneFormats(card)) {
            case PrimaryPlaneFormats::Xbgr:
                SetProperty(kAvoidAlphaBitsProp, "1");
                break;
            case PrimaryPlaneFormats::None:
                LOG(INFO) << "The primary planes cannot scan out ABGR8888 and XBGR8888";
                selections.hwc_apex = HwcApex::DrmFb;
                break;
            default:
                break;
        }

        if (selections.hwc_apex == HwcApex::Drm && !IsDrmMultiplePlanesSupported(card)) {
            SetProperty("ro.vendor.hwc.drm.disable_planes", "1");
        }
    } else {
        selections.hwc_apex = HwcApex::DrmFb;
    }

    /*
     * Graphics
     *
     * Mesa is the only one for hardware rendering
     * Swiftshader is only for software rendering
     *
     * Mesa may have software rendering backend built-in, and
     * performance wise it's bit better than Swiftshader,
     * but compatibility wise is still not as good as Swiftshader
     */
    if (IsForcedSwiftshader()) {
        selections.UseSwiftshaderGraphics();
    } else if (IsForcedMesaSoftware()) {
        selections.UseMesaSoftwareGraphics();
        SetProperty(kMesaAlwaysSoftwareProp, "true");
    } else {
        selections.hw_egl = HwEgl::Mesa;
        // Mesa llvmpipe and lavapipe
        if (!render.has_value()) selections.UseMesaSoftwareGraphics();
    }
}

void DetectGraphics(Selections& selections) {
    if (IsForcedFramebufferDisplay()) {
        LOG(INFO) << "Forced using framebuffer display";
        selections.SetupFramebufferDisplay();
        return;
    }

    auto drm_pair = GetDrmCardRenderDevicePair();
    auto drm_card = std::move(drm_pair.first);
    auto drm_render = std::move(drm_pair.second);

    if (!drm_card.has_value()) {
        LOG(ERROR) << "Falling back to framebuffer display";
        selections.SetupFramebufferDisplay();
        return;
    }

    SetProperty(kGraphicsCardNameProp, drm_card.value().name);
    SetProperty(kGraphicsRenderNameProp, drm_render.has_value() ? drm_render.value().name : "");

    if (drm_render.has_value() &&
        !drmDevicesEqual(drm_card.value().drm_device, drm_render.value().drm_device)) {
        LOG(INFO) << "DRM card " << drm_card.value().path << " and DRM render "
                  << drm_render.value().path << " are different devices";
        selections.hybrid_graphics = true;
    }

    SetDefaultsForDrmDisplay(selections, drm_card.value(), drm_render);

    // DRM HWC tries the first or the specified card node
    SetProperty(kHwcDrmDeviceProp, drm_card.value().path);

    // Minigbm tries the first render node, and then the first card node
    SetProperty(kGrallocDeviceProp,
                drm_render.has_value() ? drm_render.value().path : drm_card.value().path);
    SetProperty(kMinigbmDeviceProp,
                drm_render.has_value() ? drm_render.value().path : drm_card.value().path);

    DetectDrmCard(selections, drm_card.value(), drm_render);

    // Render
    if (drm_render.has_value()) {
        DetectDrmRender(selections, drm_render.value(), drm_card.value());

        // Mesa uses only this device, so that it does not pick another GPU
        if (selections.hw_egl == HwEgl::Mesa && !selections.uses_sw_rendered_graphics) {
            SetProperty(kMesaDrmRenderNodeProp, drm_render.value().path);
        }
    }

    // The software rendering of Mesa must not open the DRM card. The first client to
    // open it becomes the DRM master, and then the HWC cannot control the display.
    if (selections.hw_egl == HwEgl::Mesa && selections.uses_sw_rendered_graphics) {
        SetProperty(kMesaNoKmsSwrastProp, "true");
    }

    // Software rendering reads and writes the buffers by CPU, tiled ones would only be detiled
    if (selections.uses_sw_rendered_graphics) {
        SetProperty(kMinigbmAmdgpuLinearOnlyProp, "true");
    }

    CleanupDrmDevice(drm_card.value());
    if (drm_render.has_value()) CleanupDrmDevice(drm_render.value());
}

}  // namespace

int main(int, char* argv[]) {
    InitLogging(argv, &KernelLogger);
    umask(000);

    Selections selections;

    if (GetProperty(kUsbControllerProp, "").empty()) {
        selections.usb_gadget_apex = UsbGadgetApex::None;
    } else {
        LOG(INFO) << "Detected USB controller, enable USB Gadget support";
        selections.usb_gadget_apex = UsbGadgetApex::Mainline;
    }

    DetectGraphics(selections);

    return ApplySelections(selections) ? EXIT_SUCCESS : EXIT_FAILURE;
}
