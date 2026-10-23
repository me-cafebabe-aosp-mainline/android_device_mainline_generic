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
| `TARGET_NO_KERNEL` | unset | `true`: the package has no kernel and boots Android with the kernel of the distribution, see [Without a kernel](#without-a-kernel) |

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
  configuration is done in the kernel config. Built without a kernel, it
  uses the kernel of the distribution, which is signed for Secure Boot
  there, see [Without a kernel](#without-a-kernel).

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

## Without a kernel

With `TARGET_NO_KERNEL=true` the build has no kernel, and the package
boots Android with the Debian kernel of the system it is installed on.
Which kernel that is, is only known at installation: nothing about the
kernel is taken from the build host.

The Android directory has the pieces of the ramdisk in `ramdisk/`, and
the package has a program, `/usr/share/<package>/update`, that makes the
files GRUB loads:

| File in the Android directory | Content |
|-------------------------------|---------|
| `kernel` | A copy of `/boot/vmlinuz-<version>` |
| `ramdisk-all-combined.img` | The initramfs Debian made for that kernel, as it is, followed by the Android ramdisk |
| `modules-overlay/<version>/` | `modules.load` and `modules.options` for the modules of Debian |

The kernel is the newest one, or `ANDROID_HOST_KERNEL_VERSION` in
`/etc/default/<package>`. The files are made when the package is
installed, whenever `update-initramfs` makes the initramfs of that
kernel (so after each kernel upgrade), and when a kernel is removed.
To make them again by hand, run `/usr/share/<package>/update`.

The Android ramdisk is decompressed and compressed again as the Debian
initramfs is (zstd, xz, gzip, ...), whatever its compression is. Its
`/lib` is merged into `/usr/lib` of the Debian initramfs, which is a
symbolic link there. It needs the filesystem and `nls` modules from
`modules.load.ramdisk` in the initramfs: a hook of the package adds them
to it.

The modules are loaded in two steps, as for an Android kernel, and
before `/system/bin/init` starts:

1. `generic_init` loads the modules of `modules.load.ramdisk` from
   `/lib/modules/<version>` of the Debian initramfs, which has
   them compressed (`.ko.xz`). Debian's `modules.dep` is used.
2. After the Android partitions are mounted, an addon fstab of the
   ramdisk (`fstab.generic_init.addon.debian_modules`, selected by
   `androidboot.addon_fstab_suffix`, which the GRUB entry sets) mounts an
   overlay on the empty `/vendor_dlkm/lib/modules`: the modules of the
   Debian system (`/usr/lib/modules`) with `modules-overlay` on top.
   `/vendor/lib/modules` is a symbolic link to there, so `generic_init`
   loads `modules.load` of `modules-overlay`: the modules of
   `kernel/mainline/configs/modprobe/modules.load`,
   `configs/modprobe/modules.load` and `configs/modprobe/modules.load.debian`
   that the kernel has. The uevents of
   hardware load all the others, and `modprobe_kernel` the ones asked
   by name.

The firmware is the one of the Debian system (`/usr/lib/firmware`), whose
compressed files `generic_init` can use.

Requirements of this mode:

- `/usr/lib` (the modules) is on the filesystem of the Android directory,
  as the overlay reads them from there. Otherwise only the modules in the
  initramfs are loaded.
- The kernel has the options Android needs (SELinux, PSI, ...). The
  Debian one may not have them all. Its `CONFIG_LSM` has AppArmor before
  SELinux, which would stay disabled: the GRUB entry adds
  `security=selinux`. Binder is a module there:
  `configs/modprobe/modules.load.debian` lists it, as nothing loads it
  by itself like the modules that are loaded on demand, and
  `modules.options` gives it the devices `binder,hwbinder,vndbinder`
  (`CONFIG_ANDROID_BINDER_DEVICES` is only `binder`, and Debian has no
  binderfs).
  The Debian kernel also lacks the cgroup v1 `cpuset` controller
  (`CONFIG_CPUSETS_V1`) and the v1 memory controller (`CONFIG_MEMCG_V1`,
  Android uses v2 for it). A build without a kernel therefore installs
  `/vendor/etc/cgroups.json` (`configs/cgroups/cgroups.json`), which makes
  `cpuset` optional: without it init stops setting up cgroups and no
  service starts. The cpuset task profiles are skipped then.
- The installation has `python3`, `initramfs-tools`, `kmod` and the
  compression tools; the package depends on them.

## Upgrades, removal and downgrades

- Upgrading keeps the user data. Without a kernel, the kernel and
  ramdisk files are made again.
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
