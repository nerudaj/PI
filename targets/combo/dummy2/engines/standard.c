/*!
 *
 * \file standard.c
 *
 * \brief Header file with prototypes and constants of widely used code.
 *
 * \author Pavel Benacek <benacek@cesnet.cz>
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

#include "standard.h"
#include "../p4dev_types.h"
#include "../p4dev_tree.h"

uint32_t serialize_to_transaction(const uint8_t* data, const uint32_t data_len, uint32_t* bit_index, uint32_t** trans_data, 
                                  const uint32_t bitwidth) {
    /*Declare variables*/
    uint32_t* tmp_arr; /* Temporal working array */
    uint32_t tmp_elem; /* One mi32 element used for serialization */
    uint8_t transactions; /*Allocated transactions*/
    uint32_t tmp_index; /* Helping index value*/
    uint32_t i, j; /* Helping indexes during serialization */
    uint32_t bitshift; /* Variable which holds the required bit shift */
    uint32_t rem; /* Remaining data from the shift */
    uint32_t start_index; /* Starting index */
    uint32_t* new_trans_data; /*Pointer which holds the reference to new transfer array*/
    uint32_t new_transactions; /*Required number of transactiosn*/
    uint8_t tmp_data; /*Temporal data for appending */
    uint8_t mask; /* Bitmask to use for the last byte */
    /* Basic sanity check of the input */
    if(!data || !bit_index || !trans_data) return P4DEV_ERROR;
    if(data_len*8 < bitwidth) return P4DEV_BYTE_ARRAY_LENGTH_ERROR;
    /* Allocate the array of elements and serialize the uint8_t to one or more 32 bit transactions*/
    transactions = (uint32_t) ceil(data_len*8/32.0);
    tmp_arr = malloc(sizeof(uint32_t)*(transactions+1));
    if(!tmp_arr) return P4DEV_ERROR;
    
    /* Serialize data to MI32 transactions - we can use the transactions variable because the array length is
    * the number of transactions plus one. */
    tmp_arr[transactions] = 0x0;
    for(i = 0; i < transactions; i++) {
        /*Setup the default value and reset the temporal element*/
        tmp_elem = 0x0;
        for(j = 0; j < 4; j++) {
            tmp_index = i*4+j;
            if(tmp_index == data_len) break;
            /* Prepare data for appending (mask unused data in the last byte) */
            tmp_data = data[tmp_index];
            if(tmp_index == data_len-1) {
                mask = bitwidth % 8 == 0 ? 0xff : ~(0xff << (bitwidth % 8));
                tmp_data &= mask;
            }

            tmp_elem |= tmp_data << j*8;
        }
        /*Store the transaction*/
        tmp_arr[i] = tmp_elem;
    }
    /* Data are serialized, prepare data for appending */
    bitshift = *bit_index % 32;  
    rem = 0x0;
    for(i = 0; i < transactions; i++ ) {
        if(bitshift == 0) break;
        /* Shift data in current word and append the remaining bit vector
         * from previous run. */
        tmp_elem= (tmp_arr[i] << bitshift) | rem; 
        rem = (tmp_arr[i] >> (32 - bitshift));
        tmp_arr[i] = tmp_elem;
    }
    tmp_arr[transactions] = rem;
    /*Recompute the length and identify the starting and ending index */
    new_transactions = (uint32_t) ceil((*bit_index + bitwidth)/32.0);
    if(*trans_data == NULL) {
        /* We are taking the first round, setup initial values */
        start_index = 0;
    } else {
        /* We are appending to existing array, compute real indexes */
        start_index = *bit_index / 32;
    }
    /* Reallocate :-) */ 
    new_trans_data = realloc(*trans_data, sizeof(uint32_t) * new_transactions);
    if(!new_trans_data) {
        free(tmp_arr);
        return P4DEV_ERROR;
    }
    /* Append 32-bit transactions into the array - also compute the required number of trancsactions
     * (including the shift value) */
    transactions = (uint32_t) ceil((bitwidth + bitshift)/32.0);
    for(i = 0; i < transactions; i++) {
        /* Both words are merged if we are inserting the first word of new transaction and we already start 
         * the new 32 bit word */
        if(i == 0 && *bit_index > start_index*32) {
            /* Finish the word */
            new_trans_data[i+start_index] |= tmp_arr[i];
        } else { 
            /* Copy whole 32 bit transactions */
            new_trans_data[i+start_index] = tmp_arr[i];
        }
    }
    /* Refresh output data and return success */
    *trans_data = new_trans_data;
    *bit_index += bitwidth;
    free(tmp_arr);
    return P4DEV_OK;
} 

void free_transaction(uint32_t** trans_data) {
    /* Basic saniti check of input */
    if(!trans_data) return;
    /* Free allocated data and set the pointer to NULL*/
    free(*trans_data);
    *trans_data = NULL;
}

int8_t search_opcode(const void* dt, int32_t* offset, const p4rule_t rule) {
    /* Iterate through all available nodes and try to match the name. If match
     * was found, return the opcode and modify the offset variable. */
    int32_t lenp;
    int32_t tmp_offset;
    char* tmp_name;
    int8_t opcode;
    fdt32_t* tmp_opcode;

    int cmp_ret;

    /* Sanity check of input */
    if(!dt || !offset) return -P4DEV_ERROR;
   
    /* Iterate through all subnodes and search for a given opcode */ 
    fdt_for_each_subnode(tmp_offset, dt, *offset) {
        tmp_name = (char*) fdt_getprop(dt, tmp_offset, "action-name", &lenp);
        if(lenp < 0) {
            /* No such attribute was found, ending. */
            return -P4DEV_ERROR;
        }

        /* Check if we found the node */
        cmp_ret = strncmp(tmp_name, rule.action, 128);
        if(cmp_ret != 0) continue;

        /* Hell yeah! We have the node. Get the operation code and return it */
        tmp_opcode = (fdt32_t*) fdt_getprop(dt, tmp_offset, "opcode", &lenp);
        if(lenp < 0) {
            /* It is bad :-( error has been detected, ending. */
            return -P4DEV_ERROR;
        }
    
        /* Read the opcode, update the offset and return */
        opcode = (uint8_t) fdt32_to_cpu(*tmp_opcode);
        *offset = tmp_offset;
        return opcode;
    }

    return -P4DEV_ERROR;
}

uint32_t write_cmd_to_table(const p4dev_t* dev, uint32_t val, bool wait,const char* table_name) {
    /* Iterate through all tables of the device tree and write the val variable */
    const int32_t parent_offset = (int32_t) dev->dt_p4offset; 
    uint32_t   xret;
    int32_t   tmp_offset;
    int32_t    lenp;
    fdt32_t*   tmp_tab_offset;
    uint32_t   tab_offset;
    uint32_t   tmp_data;
   
    /* Check input parameters */ 
    if(!dev) {
        return P4DEV_NO_DEV;
    }

    if(!dev->cs || !dev->cs_space) {
        return P4DEV_DEVICE_NOT_ATTACHED;
    }

    if(!dev->dt) {
        return P4DEV_NO_DEVICE_TREE;
    }

    /* Get the table node offset */
    xret = dt_get_table_node(dev->dt, parent_offset, table_name, &tmp_offset);
    if(xret != P4DEV_OK) {
        return xret;
    }
        
    /* Get the offset of the table */
    tmp_tab_offset = (fdt32_t*) fdt_getprop(dev->dt, tmp_offset, "offset", &lenp);
    if(lenp < 0) {
        /* Error during the processing of device tree, ending.*/
        return P4DEV_DEVICE_TREE_ERROR;
    }
    
    /* Convert the device tree integer to the endianity of CPU */
    tab_offset = fdt32_to_cpu(*tmp_tab_offset);
    
    /* Write the command */
    cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, CMD_REG_OFFSET), val); 
    
    /* Wait untill the busy offset is 0 */
    while(wait) {
        tmp_data = cs_space_read_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, CMD_REG_OFFSET));
        /* Check if the device is still busy */
        if(!(tmp_data & CMD_BUSY_FLAG)) break;
    }

    return P4DEV_OK;
}

p4key_elem_t* find_key_element(const p4rule_t rule, const char* name) {
    /* Declaration of variables */
    p4key_elem_t* tmp = rule.key;
    int cmp_ret;
    /* Basic sanity check */
    if(!name) return NULL;
    /*Try to identify the name of the table*/
    while(tmp) {
        cmp_ret = strncmp(tmp->name, name, 128);
        if(!cmp_ret) return tmp;
        /* No match, try the next element */
        tmp = tmp->next;
    }

    return NULL;
}

p4param_t* find_param_element(const p4rule_t rule, const char* name) {
    /* Declaration of parameters */
    int cmp_ret;
    p4param_t* tmp = rule.param;
    /* Sanity check of the input */
    if(!name) return NULL;
    /* Try to identify the parameter element in linked list */     
    while(tmp) {
        cmp_ret = strncmp(tmp->param_name, name, 128);
        if(!cmp_ret) return tmp;
        /* Try the next element */
        tmp = tmp->next;
    }

    return NULL;
}

