# Debian package

The `debpackage` build target makes a `.deb` that installs Android next
to a Debian-based Linux distribution, on the same filesystem, and adds
it to the GRUB menu. The host's firmware files are used, so nothing
else has to be installed.

## Building

```
m debpackage
```

The output is `$ANDROID_PRODUCT_OUT/<artifact prefix>_<arch>.deb`.
`debpackage-nodeps` rebuilds the package without rebuilding the images.

| Variable | Default | Meaning |
|----------|---------|---------|
| `MAINLINE_GENERIC_DEBPACKAGE_MAINTAINER` | `Unknown <unknown@localhost>` | `Maintainer` field of the package |

The package is built with `dpkg-deb` (from `dpkg`) of the build host.
It is not compressed, because its content already is.

## Requirements on the host

- GRUB as the boot manager. With another one, the boot entry has to be
  written by hand, as described in [Installation](installation.md).
- `/usr/lib` on ext4 or F2FS, directly on a partition or disk.
  LVM, LUKS and other device mapper devices are not supported, nor are
  other filesystems: `generic_init` can not find the Android directory
  there.
- Secure Boot disabled, or the kernel signed with a key the firmware
  trusts. The package ships its own kernel, because the Android
  configuration is done in the kernel config.

## What it installs

| Path | Content |
|------|---------|
| `/usr/lib/android-system/<device>/` | The Android directory: kernel, ramdisk and system images, plus the user data. Mode `0700`, because Android user data is owned by Android UIDs that are ordinary user UIDs on the host. |
| `/etc/grub.d/35_<package>` | Adds the boot entries to `grub.cfg` |
| `/etc/default/<package>` | Configuration, see below |

`<package>` is `android-system-` followed by the device name in lower
case, for example `android-system-generic-x86-64`.

The GRUB menu gets two entries: one with the configured user data, one
with user data in RAM, for when the stored user data does not boot.

## Configuration

`/etc/default/<package>` sets:

| Variable | Default | Meaning |
|----------|---------|---------|
| `ANDROID_USERDATA_MODE` | `bind_mount_dir` | `androidboot.mount_userdata`: `bind_mount_dir`, `imgs` or `tmpfs`, see [Boot parameters](boot-parameters.md) |
| `ANDROID_USERDATA_SIZE` | `8G` | Size of `userdata.img` for `imgs` |
| `ANDROID_SERIAL_CONSOLE` | `false` | Adds the serial console to the kernel command line |
| `ANDROID_CMDLINE_EXTRA` | empty | More kernel parameters, for example `androidboot.*` |

Run `dpkg-reconfigure <package>` after changing it. It creates the
user data directories or images that are missing, and updates
`grub.cfg`.

## Upgrades, removal and downgrades

- Upgrading keeps the user data.
- `apt remove` keeps the user data and the configuration. `apt purge`
  deletes both.
- Downgrading is refused while user data exists, because Android can not
  use user data written by a newer build. Purge the package first, or
  set `ANDROID_ALLOW_DOWNGRADE=1` to install the older build anyway.

## Notes

- If another disk also has an Android directory, the last one found is
  used, see [Installation](installation.md).
- Only `Generic_x86_64` and `Generic_arm64` are packaged. On arm64,
  devices that boot with a device tree blob need it added to the boot
  entry by hand.
