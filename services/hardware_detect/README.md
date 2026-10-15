# hardware_detect

Early boot service that inspects the hardware and selects the graphics
stack (EGL, Vulkan, gralloc and HWC APEXes, OpenGL ES version) and the USB
gadget APEX, by setting system properties before the HALs start.

## Layout

| File | Purpose |
|------|---------|
| `hardware_detect.cpp` | `main()`: USB gadget choice and the graphics detection flow |
| `selections.{h,cpp}` | `Selections` (everything the detection selects), the boot overrides, and `ApplySelections()` which sets the properties |
| `drm_devices.{h,cpp}` | Enumerates `/dev/dri`, selects the DRM card and render |
| `drivers/drivers.{h,cpp}` | Dispatches the selected card and render to the DRM driver handlers by driver name |
| `drivers/*.cpp` | One file per DRM driver family. A handler only changes `Selections` and sets driver specific quirk properties |
| `drivers/*_pci_ids.h` | PCI ID tables, generated from Mesa's `include/pci_ids` |

To support a new DRM driver, add a handler in `drivers/` and register its
name in the tables of `drivers/drivers.cpp`. Drivers that are not
registered are treated as unknown.

## Detection flow

1. `ro.boot.use_fb_display` forces the framebuffer display. The same is
   used when no DRM card is found.
2. The DRM card and render are selected: sysfb cards (`simpledrm`, ...)
   and cards without a connected connector are the last choice, and a card
   with its own render is preferred. The other nodes are renamed with an
   `_orig` suffix and replaced by symlinks to `/dev/null`.
3. Defaults for DRM display are set: minigbm gralloc, DRM HWC when atomic
   modesetting is available, and Mesa. `ro.boot.graphics=swiftshader`
   switches to Swiftshader instead.
4. The driver handlers refine the selection, mostly according to the GPU
   generation and what Mesa supports on it (OpenGL ES version, Vulkan
   driver, gralloc, and quirks).
5. The boot overrides are processed and the properties are set.

## Properties

Read (set on the kernel command line / bootconfig):

| Property | Meaning |
|----------|---------|
| `ro.boot.use_fb_display` | Force the framebuffer display |
| `ro.boot.graphics` | `swiftshader` forces software rendering |
| `ro.boot.enum_drm_in_reverse` | Enumerate DRM nodes in reverse order |
| `ro.boot.prefer_drm_card_name` | Prefer the card of the given DRM driver, with its own render or none |
| `ro.boot.prefer_drm_render_name` | Prefer the render of the given DRM driver |
| `ro.boot.<name>` | Override a selection with the numeric value of its enum in `selections.h`. `<name>` is one of `gles_version`, `hw_egl`, `hw_vulkan`, `gralloc_apex`, `hwc_apex`, `usb_gadget_apex`, `vulkan_apex`, `minigbm_generic_backend`, `sf_native_window_buffers_format` |
| `sys.usb.controller` | Presence enables the USB gadget APEX |

Set:

`ro.opengles.version`, `ro.hardware.egl`, `ro.hardware.vulkan`, the
`ro.boot.vendor.apex.*` selections for the gralloc, HWC, USB gadget and
Vulkan APEXes, `ro.vendor.graphics.card.name`,
`ro.vendor.graphics.render.name`, `vendor.hwc.drm.device`,
`vendor.gralloc.device`, `vendor.minigbm.device`,
`vendor.minigbm.generic_backend`, `vendor.sys.usb.adb.disabled`,
`ro.surface_flinger.supports_background_blur`, and the quirk properties
set by the drivers (`ro.vendor.hwc.drm.*`, `vendor.minigbm.*`,
`vendor.mesa.*`).

## Driver notes

- **i915 / xe**: the device ID is mapped to an Intel generation. Gen 2 has
  no Mesa driver (Swiftshader), Gen 3~5 get GLES 2.0 without Vulkan,
  Gen 6 GLES 3.0 without Vulkan, Gen 7 GLES 3.1 and Gen 8 GLES 3.2 with
  `intel_hasvk`, and Gen 9+ GLES 3.2 with `intel`. Unlisted IDs are Gen 9+.
- **amdgpu**: GLES 3.2 and `radv`.
- **radeon**: GLES by Mesa driver (r300g 2.0, R600~R700 3.0,
  Evergreen/NI 3.1, radeonsi 3.2), without Vulkan.
- **nouveau**: GLES by chipset (NV30 2.0, NV50 3.0, Fermi and newer 3.1),
  NVK for Kepler and newer, Swiftshader before NV30.
- **msm**: GLES by Adreno series (a2xx 2.0, a3xx 3.0, a4xx and newer
  3.2, a702 3.1), Turnip for Adreno 6xx and newer. The GPU model is read
  with the MSM ioctls directly, because libdrm_freedreno refuses GPUs
  whose GPU ID is 0.
- **virtio_gpu**: 3D capable hosts use Mesa and the `virtio` Vulkan
  driver, others use Swiftshader.
- **vmwgfx**: hosts without 3D use Swiftshader, otherwise GLES 2.0
  (vgpu9), 3.0 (vgpu10 / SM5) or 3.1 (GL43).
