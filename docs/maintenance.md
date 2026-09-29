# Maintenance

This page describes the maintenance of this device tree.

The core principle is: keep it generic and actually useful.

This device tree was initially based on the `android-16.0.0_r4` AOSP
tag.

The most common maintenance tasks are:

- Ensuring everything still works as intended on each new Android
  version.
- Keeping up with upstream changes for components that track an
  upstream.
- Extending support for additional hardware, and improving support for
  already-supported hardware.
- Making the device tree more convenient to use, for both developers
  and end users.

## Table of components with an upstream

| Component path | Upstream |
|----------------|----------|
| `configs/init/console_override.rc` | `console` service in `system/core/rootdir/init.rc` |
| `configs/init/init_dev_config_override.rc` | `init_dev_config` service in `system/core/rootdir/init.rc` |
| `configs/input/Generic.kl` | `frameworks/base/data/keyboards/Generic.kl` |
| `configs/misc/pci.ids` | https://pci-ids.ucw.cz/ |
| `configs/misc/usb.ids` | http://www.linux-usb.org/usb-ids.html |
| `configs/properties/vendor_bluetooth_profiles.prop` | `device/linaro/dragonboard/product.prop` |
| `hals/health` | `hardware/interfaces/health/aidl/default` and `device/google/cuttlefish/guest/hals/health/health-aidl.cpp` |
| `overlays/rro_overlays/MainlineGenericWifiOverlay` | `device/google/cuttlefish/shared/phone/overlays/CuttlefishWifiOverlay` |
| `services/generic_init` (in `vendor/mainline` repository) | `system/core/init` |
| `services/modprobe_kernel/modprobe.cpp` | `system/core/toolbox/modprobe.cpp` |
| `services/overlay_remounter_override/overlay_remounter.cpp` | `system/core/overlay_remounter/overlay_remounter.cpp` |
