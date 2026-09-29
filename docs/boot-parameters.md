# Boot parameters

This document describes the kernel command-line parameters that control
the behavior of this device tree's components. Parameters are grouped
by the component that consumes them.

## generic_init

These parameters are consumed by `generic_init`, the custom init
implementation located at `vendor/mainline/services/generic_init`.
See [Booting process](booting-process.md) for how `generic_init`
fits into the boot sequence.

### androidboot.addon_fstab_suffix

Specifies one or more suffixes (comma-separated) for addon fstab files
to be imported. Each suffix maps to a file named
`fstab.generic_init.addon.<suffix>` loaded from `/system/etc/`.
Addon fstab entries are appended after all dynamically generated
entries.

### androidboot.android_dir

Specifies the preferred directory name to search for on each block
device when looking for Android files (images, subdirectories,
firmware). This name is tried first; the fallbacks are `android` and
`boot/android`. If multiple block devices contain a matching
directory, the last one found is used. See
[Installation](installation.md) for how the Android directory is used.

### androidboot.board_platform

When `generic_init` parses `.rc` files, it must expand property
references in certain lines. Because Android properties are not yet
available at this early stage, `generic_init` works around this by
looking up values from `androidboot.*` parameters instead.

The property name `ro.board.platform` is translated to this parameter
key. The value assigned to this parameter is used when that property
is referenced during `.rc` parsing.

### androidboot.init_fatal_pause

When set to `true`, if `generic_init` crashes, the system hangs
indefinitely instead of rebooting. This is useful for reading
crash-related logs. See [Debugging](debugging.md) for usage.

### androidboot.modalias_handling_delay_ms

Specifies how many milliseconds to delay before the modalias handler
loads a kernel module.

This can be useful for identifying which module is causing a crash,
by slowing down module loading to observe the sequence.

### androidboot.mount_firmware

Specifies how to mount the `/mnt/vendor/firmware` mountpoint.

The following options are supported:

| Value | Description |
|-------|-------------|
| `all_possible_dirs` (or `true`) | Scan all block devices for firmware directories and bind mount the first one found to this mountpoint. On each block device, the directories checked (in order) are: `<android_dir>/firmware`, `usr/lib/firmware`, `lib/firmware`, `linux-firmware`, `firmware`. The last block device with a matching directory wins. |
| `disable` (or `false`) | Skip mounting this mountpoint entirely. |
| `img` | Mount from a firmware image inside the Android directory. The image name may be either `firmware.img` or `linux-firmware.img`. |
| `img_ram` | Like `img`, but the firmware image is first copied to RAM before mounting. |
| `only_android_dir` | Like `all_possible_dirs`, but only checks the `firmware` subdirectory within the Android directory on each block device. |

### androidboot.mount_system

Specifies how to mount the Android system partitions.

The following options are supported:

| Value | Description |
|-------|-------------|
| `std_parts` | Map the `PARTNAME` uevent field (typically the GPT partition name) of block devices to Android system partitions. |
| `blk_devices` | Automatically determine which Android system partition each block device corresponds to, by inspecting the `build.prop` file inside it. |
| `imgs` | Map Android system images in the Android directory to Android system partitions based on their file name. |
| `imgs_ram` | Like `imgs`, but the images are first copied to RAM before mounting. |

### androidboot.mount_userdata

Specifies how to mount the Android userdata partitions.

The following options are supported:

| Value | Description |
|-------|-------------|
| `std_parts` | Map the `PARTNAME` uevent field (typically the GPT partition name) of block devices to Android userdata partitions. |
| `imgs` | Map Android userdata images in the Android directory to Android userdata partitions based on their file name. |
| `bind_mount_dir` | Bind mount directories named after Android userdata partitions in the Android directory to the corresponding mount points. |
| `tmpfs` | Use a `tmpfs` mount for userdata mount points, storing the content in RAM (non-persistent across reboots). |

## hardware_detect

These parameters are consumed by `hardware_detect`, the hardware
configuration and HAL selection service located at
`services/hardware_detect` in this repository. See
[Booting process](booting-process.md#the-hardware_detect-service) for
how this service fits into the boot sequence.

### androidboot.prefer_drm_card_name

Use the DRM card with the specified name if present.

### androidboot.prefer_drm_render_name

Use the DRM render node with the specified name if present.

### androidboot.enum_drm_in_reverse

When set to `true`, DRM devices are enumerated in reverse order. Use
this to pick DRM devices starting from the last one instead of the
first.

### androidboot.graphics

When set to `swiftshader`, SwiftShader (CPU-based rendering) is used
if possible.

### androidboot.use_fb_display

When set to `true`, display output uses framebuffer devices, which
provides maximum compatibility but reduced functionality compared to
DRM.

### Overrides for selections in hardware_detect service

See the `kBootOverridesProp` map in the `hardware_detect` source code
for the full list of override keys.

## Init rc

These parameters are consumed by init `.rc` scripts in the mounted
Android OS.

### androidboot.lcd_density

Passes the specified value to the `ro.sf.lcd_density` property. If
this parameter is not set, `ro.sf.lcd_density` is set by the
`get_display_ppi` service instead.

### androidboot.save_early_logs

When set to `1`, early logs (currently logcat only) captured before
boot completes are saved to `/metadata`.

### androidboot.seriallogging

Set this to the name of a serial device to mirror logcat output to
that device.

### androidboot.seriallogging.logcat_buffer

Sets the logcat buffer(s) printed to the serial device. The value is
passed directly to the `-b` option of `logcat`.

### androidboot.wifi_impl

When set to `virt_wifi`, the `setup_wifi` service starts and creates a
wlan interface bridged to an existing Ethernet interface.

## libinit

### androidboot.insecure_adb

When set to `true`, ADB authentication is disabled, ADB runs as root
by default, and the ADB service is available earlier during boot.
See [Debugging](debugging.md) for usage.

## Others

### androidboot.mode

When set to `console`, spawns a shell on the console device. Once the
shell exits, booting continues in normal mode.

Requires the "Add console boot mode" patch (Change-Id
`Id51f200ca2c5123cf16212c363d770f503744581`) to be applied to
`system/core`. See [Patches](patches.md) for how to apply it.
