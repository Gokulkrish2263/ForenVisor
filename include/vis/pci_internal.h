/*
 * Copyright (c) 2007, 2008 University of Tsukuba
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Tsukuba nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PCI_INTERNAL_H
#define _PCI_INTERNAL_H
#include <vis/io.h>

/********************************************************************************
 * PCI internal definitions and interfaces
 ********************************************************************************/
#define PCI_CONFIG_ADDR_PORT	0x0CF8
#define PCI_CONFIG_DATA_PORT	0x0CFC

#define DEFINE_pci_read_config_data_without_lock(size)		\
static inline u##size pci_read_config_data##size##_without_lock(pci_config_address_t addr, int offset) \
{								\
	u##size data;						\
	asm_out32(PCI_CONFIG_ADDR_PORT, addr.value);		\
	asm_in##size(PCI_CONFIG_DATA_PORT + offset, &data);		\
	return data;						\
}

#define DEFINE_pci_write_config_data_without_lock(size)		\
static inline void pci_write_config_data##size##_without_lock(pci_config_address_t addr, int offset, u##size data) \
{								\
	asm_out32(PCI_CONFIG_ADDR_PORT, addr.value);		\
	asm_out##size(PCI_CONFIG_DATA_PORT + offset, data);		\
}

DEFINE_pci_read_config_data_without_lock(8)
DEFINE_pci_read_config_data_without_lock(16)
DEFINE_pci_read_config_data_without_lock(32)
DEFINE_pci_write_config_data_without_lock(8)
DEFINE_pci_write_config_data_without_lock(16)
DEFINE_pci_write_config_data_without_lock(32)

static inline u32 pci_read_config_data_port_without_lock(){ u32 data; asm_in32(PCI_CONFIG_DATA_PORT, &data); return data;}
static inline void pci_write_config_data_port_without_lock(u32 data){ asm_out32(PCI_CONFIG_DATA_PORT, data); }

//extern int pci_config_data_handler(core_io_t io, union mem *data, void *arg);
//extern int pci_config_addr_handler(core_io_t io, union mem *data, void *arg);
void pci_save_config_addr(void);
void pci_restore_config_addr(void);
extern void pci_append_device(struct pci_device *dev);

#endif
