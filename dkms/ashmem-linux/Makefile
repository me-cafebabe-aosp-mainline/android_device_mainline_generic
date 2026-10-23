# SPDX-License-Identifier: GPL-2.0
obj-m += ashmem_linux.o
ashmem_linux-y := ashmem.o
ccflags-y += -I$(src)

# shmem_file_setup() takes vma_flags_t instead of vm_flags_t in recent kernels
ashmem_vma_flags_t := $(shell grep -s vma_flags_t $(srctree)/include/linux/shmem_fs.h)
ifneq ($(ashmem_vma_flags_t),)
ccflags-y += -DASHMEM_SHMEM_VMA_FLAGS_T
endif

# Recent kernels have mmap_prepare in struct file_operations, which shmem may use instead of mmap
ashmem_mmap_prepare := $(shell grep -s -m1 mmap_prepare $(srctree)/include/linux/fs.h)
ifneq ($(ashmem_mmap_prepare),)
ccflags-y += -DASHMEM_HAVE_MMAP_PREPARE
endif
