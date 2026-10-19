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
   with its own render is preferred. A card of an SoC (platform bus)
   without a render of its own gets the usable render of the SoC, since
   the GPU is another DRM device there (Apple, Allwinner, ...). The other
   devices are left alone;
   Mesa is told the selected render node, and the other HALs the selected
   paths, by properties.
3. Defaults for DRM display are set: minigbm gralloc, DRM HWC when atomic
   modesetting is available, the primary planes can scan out the
   framebuffers of Android (ABGR8888, or XBGR8888 with
   `ro.vendor.hwc.drm.avoid_using_alpha_bits_for_framebuffer`), and the
   card and the render are the same device or on the same SoC, and Mesa,
   which is llvmpipe with lavapipe when there is no render. `ro.boot.graphics=llvmpipe` forces that, and
   `ro.boot.graphics=swiftshader` switches to Swiftshader instead.
4. A render that cannot do hardware rendering (`virtio_gpu` and `vmwgfx`
   without 3D, Intel Gen 2, `nouveau` before NV30) is checked by its
   driver, and selects Swiftshader. Its card still displays.
   Then the driver handlers refine the selection, mostly according to the
   GPU generation and what Mesa supports on it (OpenGL ES version, Vulkan
   driver, gralloc, and quirks).
5. The boot overrides are processed and the properties are set.

## Hybrid graphics

The card (display) and the render can be different devices, for example a
`bochs-drm` display with a `virtio_gpu` or an `amdgpu` render. The pair
is the same device by default, or the display and the GPU of an SoC. To
select another pair, give the card and the render names on the kernel
command line:

```
androidboot.prefer_drm_card_name=bochs-drm
androidboot.prefer_drm_render_name=virtio_gpu
```

The render handlers see both devices, and `hybrid_graphics` is set when
they differ. Mesa uses only the render node of
`vendor.mesa.drm.render.node`, and the HALs use the paths of the card and
the render. The other DRM nodes are left alone.

A card that displays the render of another device uses drmfb-composer,
which copies the buffers, since drm_hwcomposer imports them on the card
and that cannot be relied on across devices. A basic card (`bochs-drm`,
...) uses it too. The devices of an SoC share memory, so they keep
drm_hwcomposer. `androidboot.hwc_apex=1` selects drm_hwcomposer anyway,
for trying a pair. The gralloc and minigbm then allocate the buffers for
scanout as linear (`vendor.gralloc.scanout_linear` and
`vendor.minigbm.scanout_linear`, set for such a pair), since the display
cannot use the modifiers of another GPU. Only the `gbm_mesa` gralloc and
the `gbm_mesa` backend of minigbm support it.

`androidboot.prefer_discrete_render=true` selects such a pair when the
card is a PCI device on the root bus (an integrated GPU, an Intel iGPU
for example) and a usable render is a PCI device behind a bridge (a card
in a slot). Otherwise nothing changes. An APU display and a render of
the same driver are not told apart, give the names for them. The names
above take precedence.

## Properties

Read (set on the kernel command line / bootconfig):

| Property | Meaning |
|----------|---------|
| `ro.boot.use_fb_display` | Force the framebuffer display |
| `ro.boot.graphics` | `swiftshader` forces software rendering with Swiftshader, `llvmpipe` with Mesa llvmpipe and lavapipe |
| `ro.boot.enum_drm_in_reverse` | Enumerate DRM nodes in reverse order |
| `ro.boot.prefer_drm_card_name` | Prefer the card of the given DRM driver, with its own render or none |
| `ro.boot.prefer_drm_render_name` | Prefer the render of the given DRM driver |
| `ro.boot.prefer_discrete_render` | Render with a discrete GPU in a slot when the display is an integrated GPU |
| `ro.boot.<name>` | Override a selection with the numeric value of its enum in `selections.h`. `<name>` is one of `gles_version`, `hw_egl`, `hw_vulkan`, `gralloc_apex`, `hwc_apex`, `usb_gadget_apex`, `vulkan_apex`, `minigbm_generic_backend`, `sf_native_window_buffers_format` |
| `sys.usb.controller` | Presence enables the USB gadget APEX |

Set:

`ro.opengles.version`, `ro.hardware.egl`, `ro.hardware.vulkan`, the
`ro.boot.vendor.apex.*` selections for the gralloc, HWC, USB gadget and
Vulkan APEXes, `ro.vendor.graphics.card.name`,
`ro.vendor.graphics.render.name`, `vendor.hwc.drm.device`,
`vendor.gralloc.device`, `vendor.minigbm.device`,
`vendor.mesa.drm.render.node`, `vendor.mesa.android.no.kms.swrast`,
`vendor.mesa.libgl.always.software`,
`vendor.gralloc.scanout_linear`,
`vendor.minigbm.scanout_linear`, `vendor.minigbm.generic_backend`,
`vendor.sys.usb.adb.disabled`, `ro.surface_flinger.supports_background_blur`,
and the quirk properties set by the drivers (`ro.vendor.hwc.drm.*`,
`vendor.minigbm.*`, `vendor.mesa.*`).

## Driver notes

- **i915 / xe**: the device ID is mapped to an Intel generation. Gen 2 has
  no Mesa driver (Swiftshader), Gen 3~5 get GLES 2.0 without Vulkan,
  Gen 6 GLES 3.0 without Vulkan, Gen 7 GLES 3.1 and Gen 8 GLES 3.2 with
  `intel_hasvk`, and Gen 9+ GLES 3.2 with `intel`. Unlisted IDs are Gen 9+.
- **amdgpu**: GLES 3.2 and `radv`, with the `amdgpu` backend of minigbm,
  which allocates through Mesa's GBM (linear only without it, or with
  `debug.minigbm.amdgpu.linear_only=true`). It allocates scanout buffers as
  linear, so it needs no `scanout_linear`. `androidboot.gralloc_apex=3`
  selects the `gbm_mesa` gralloc instead.
- **radeon**: GLES by Mesa driver (r300g 2.0, R600~R700 3.0,
  Evergreen/NI 3.1, radeonsi 3.2), without Vulkan.
- **nouveau**: GLES by chipset (NV30 2.0, NV50 3.0, Fermi and newer 3.1),
  NVK for Kepler and newer, Swiftshader before NV30. Swiftshader is also
  selected when the kernel has no acceleration, which is usually the
  missing firmware of the GPU. It is detected by a channel allocation that
  fails with `ENODEV` and never allocates a channel.
- **msm**: GLES by Adreno series (a2xx 2.0, a3xx 3.0, a4xx and newer
  3.2, a702 3.1), Turnip for Adreno 6xx and newer. The GPU model is read
  with the MSM ioctls directly, because libdrm_freedreno refuses GPUs
  whose GPU ID is 0.
- **virtio_gpu**: 3D capable hosts use Mesa and the `virtio` Vulkan
  driver, others use Swiftshader.
- **vmwgfx**: hosts without 3D use Swiftshader, otherwise GLES 2.0
  (vgpu9), 3.0 (vgpu10 / SM5) or 3.1 (GL43).
