/*!
 *
 * \file p4dev_reg.c
 *
 * \brief Local header file for P4DEV library. This library is used for development
 * of tools above generated P4 device.
 *
 * \author Pavel Kohout <xkohou15@stud.fit.vutbr.cz>
 */
 /*
 * Copyright (C) 2017 CESNET
 *
 * LICENSE TERMS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#include "p4dev_reg.h"
#include "p4dev.h"
#include "p4dev_tree.h"
#include "p4dev_types.h"
#include "engines/standard.h"
#include <libfdt.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

API uint32_t p4dev_register_read(const p4dev_t* dev, p4_register_t* reg, uint8_t* read_data, const uint32_t length ,uint32_t reg_ind) {
    uint32_t trans_count;
    uint32_t tmp_data;
    
    /* Check input parameters */
    if(!dev) { 
        return P4DEV_NO_DEV;
    }
 
    if(!reg) { 
        return P4DEV_NO_REG;
    }

    /* Check value of register index and extract the address offset of the register array */
    if(reg_ind >= reg->count) {
        return P4DEV_REG_INDEX_ERROR;
    }
	
	if (length < (uint32_t)(ceil(reg->width / 8.0))) {
		return P4DEV_SMALL_BUFFER;
	}

    for (uint32_t i = 0; i < 4; i++) {
		read_data[i] = (uint8_t)(reg_ind);
	}

    return P4DEV_OK;   
}

API uint32_t p4dev_register_write(const p4dev_t* dev, p4_register_t* reg, uint8_t* data_to_write, const uint32_t length, uint32_t reg_ind) {
    (void)data_to_write;
	(void)length;

    /* Check input parameters */
    if(!dev) {
        return P4DEV_NO_DEV;
    }

    if(!reg) {
        return P4DEV_NO_REG;
    }

    /* Check value of register index */
    if(reg_ind >= reg->count) {
        return P4DEV_REG_INDEX_ERROR;
    }
    
    return P4DEV_OK;
}

API void p4dev_registers_free(p4_register_t* reg_arr, const uint32_t reg_arr_len) {
    /* Sanity check of input */
    if(reg_arr == NULL) return;
    if(reg_arr_len == 0) {
        free(reg_arr);
        return;
    }

    /* Free allocated table names */
    for(uint32_t i = 0; i < reg_arr_len; i++) {
        free(reg_arr[i].bind_table);
    }

    /* Free the register array */
    free(reg_arr);
}

API uint32_t p4dev_initialize_registers(const p4dev_t* dev, p4_register_t* reg) {
    (void)dev;

    /* Check input parameters */
    if(!reg) {
        return P4DEV_NO_REG;
    }

    return P4DEV_OK;
}

const uint32_t REG_COUNT = 3;
const char REG_NAMES[][64] = {
	"reg1", "reg2", "reg3"
};
const uint32_t REG_WIDTH = 32;
const uint32_t REG_IN_ARR_COUNTS[] = {
	1, 2, 4
};

API uint32_t p4dev_registers_get(const p4dev_t* dev, p4_register_t** reg_arr, uint32_t* length) {
	(void)dev;
	
	*reg_arr = NULL;
    *reg_arr = (p4_register_t*)(malloc(sizeof(p4_register_t) * REG_COUNT));
	if (reg_arr == NULL) {
		return P4DEV_ALLOCATE_ERROR;
	}
	
	*length = REG_COUNT;
	
	for (uint32_t i = 0; i < REG_COUNT; i++) {
		(*reg_arr)[i].name = strndup(REG_NAMES[i], 64);
		(*reg_arr)[i].width = REG_WIDTH;
		(*reg_arr)[i].offset = 0;
		(*reg_arr)[i].count = REG_IN_ARR_COUNTS[i];
		(*reg_arr)[i].bind_type = BINDING_GLOBAL;
		(*reg_arr)[i].bind_table = NULL;
	}
	
    return P4DEV_OK;
}

API uint32_t p4dev_initialize_table_registers(const p4dev_t* dev, const char* table_name) {
    (void)dev;
	(void)table_name;
    return P4DEV_OK; 
}
