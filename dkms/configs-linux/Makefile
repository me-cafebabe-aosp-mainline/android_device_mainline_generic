# SPDX-License-Identifier: GPL-2.0
obj-m += configs.o
# The assembler finds config_data.gz of configs.c in the build directory
ccflags-y += -Wa,-I$(abspath $(obj))

$(obj)/configs.o: $(obj)/config_data.gz

# The config of the kernel headers, as the one of the kernel
$(obj)/config_data.gz: $(objtree)/.config FORCE
	gzip -n -9 < $< > $@
