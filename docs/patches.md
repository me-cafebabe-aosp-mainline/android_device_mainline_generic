# Patches

This page lists the patches that must be applied to the LineageOS
source tree before building. The lists below assume repositories from
the LineageOS organization; they may not apply to other upstreams.

## Topics

Run:

```
repopick -t c2-rgb
repopick -t mainline-kernel
```

## external/zstd

For the `lineage-24.0` branch:

| Change ID | Change number | Commit message |
|-----------|---------------|----------------|
| `I12cbe2c8b47a0ceccf248b088202fefc74525b3a` | 496520 | `zstd: Android.bp: Add `//vendor:__subpackages__` to visibility` |

Run `repopick [change number ...]` to pick these patches.

## kernel/mainline/android-mainline

- `drm/vmwgfx: add ABGR8888 to vmw_primary_plane_formats[]`: https://review.lineageos.org/c/LineageOS/android_kernel_virt_virtio/+/423579
- `HACK: selinux: Force permissive when androidboot.selinux=permissive`: https://review.lineageos.org/c/LineageOS/android_kernel_virt_virtio/+/464426
- `Revert "drm/virtio: Don't create a context with default param if context_init is supported"`: https://review.lineageos.org/c/LineageOS/android_kernel_virt_virtio/+/496668

## system/core

For the `lineage-24.0` branch:

| Change ID | Change number | Commit message |
|-----------|---------------|----------------|
| `Ie62f6ce7783f4c1b19464a44b8cd58e09fcb2e7b` | 501438 | `init: Make first stage init call the real SetFatalRebootTarget()` |
| `Ie86329f1a03169d08c2dbb9269705e56c5f9ff1f` | 501439 | `init: reboot_utils: Add option to pause on init fatal error` |
| `Id51f200ca2c5123cf16212c363d770f503744581` | 501440 | `Add console boot mode` |

Run `repopick [change number ...]` to pick these patches.

## system/memory/libmeminfo

Apply from [here](../.patches/system/memory/libmeminfo).
