/*!
 *
 * \file p4dev_tree.c
 *
 * \brief Implementation private helping device tree functions.
 *
 * \author Pavel Benacek <benacek@cesnet.cz>
 */
 /*
 * Copyright (C) 2016 CESNET
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

#include "p4dev_tree.h"
#include <libfdt.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

uint32_t dt_get_table_address_offset(const void* dt, const uint32_t dt_p4offset, 
                                     const char* table_name, uint32_t* ret_addr) {
    /* Prepare variables */     
    int32_t table_offset;
    uint32_t xret;
    int32_t lenp;
    fdt32_t* tmp_addr;
    
    /* Sanity check of the input */
    if(!dt || !ret_addr) return P4DEV_ERROR;
    
    /* Get the table offset from the device tree node, find a given table and
     * return the value of <offset> attirbute */
    xret = dt_get_table_node(dt, dt_p4offset, table_name, &table_offset);
    if(xret != P4DEV_OK) return xret;

    tmp_addr = (fdt32_t*) fdt_getprop(dt, table_offset, "offset", &lenp);
    if(lenp < 0) {
        /* Error during the operation with the device tree */
        return P4DEV_DEVICE_TREE_ERROR;
    }
    
    /* Copy the value of the device tree to the endianity of CPU */
    *ret_addr = fdt32_to_cpu(*tmp_addr);
    return P4DEV_OK;
}

uint32_t dt_get_base_address(const void* dt, const uint32_t dt_p4offset, uint32_t* base_addr) {
    /* Prepare variables */     
    int32_t lenp;
    fdt64_t* tmp_addr;

    /* Sanity check of input */
    if(!dt || !base_addr) return P4DEV_ERROR;
    
    /* Get the base address from the device tree */
    tmp_addr = (fdt64_t*) fdt_getprop(dt, dt_p4offset, "reg", &lenp);
    if(lenp < 0) {
        /* Error during the operatin with the device tree */
        return P4DEV_DEVICE_TREE_ERROR;
    }
    
    /* Copy the value of the device tree to the endianity of CPU -- upper 32 bits*/
    *base_addr = (uint32_t) (fdt64_to_cpu(*tmp_addr) >> 32);
    return P4DEV_OK;
}


uint32_t dt_get_table_node(const void* dt, const uint32_t dt_p4offset, const char* table_name, int32_t* node_offset) {
    /* Declare variables */
    int32_t dt_lenp;
    int32_t dt_node_offset = (int32_t) dt_p4offset; /* Offset in the current node */
    int32_t tmp_offset = 0; /* Working temporal offset */
    const void* tmp_prop; /* Temporal property name */
    int32_t cmp_ret; /* Result of the comparison */
    /* Sanity check of the input */
    if(!dt || !table_name || !node_offset) return P4DEV_ERROR;

    /* Read the "table-name" property and check for equality with <table_name> parameter */
    fdt_for_each_subnode(tmp_offset, dt, dt_node_offset) {
        /* Read the property and check for any error */
        tmp_prop = fdt_getprop(dt,tmp_offset, "table-name", &dt_lenp);
        if(dt_lenp < 0) {
            return P4DEV_DEVICE_TREE_ERROR; 
        }
    
        cmp_ret = strncmp((const char*) tmp_prop, table_name, 128);
        if(!cmp_ret) {
            /* We find a given node! Remember it and end */
            *node_offset = tmp_offset;
            return P4DEV_OK;
        }
    }
    /* The node doesn't exsit */
    return P4DEV_TABLE_NAME_ERROR;
}


uint32_t dt_get_table_capacity(const p4dev_t* dev, const char* name, uint32_t* capacity) {
    /* Define variables */
    uint32_t xret;
    int32_t node_offset;
    int32_t lenp;
    fdt32_t* tmp_capacity;

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

    /* Check the table, get the offset and extract the capacity */
    if(!name || !capacity) return P4DEV_ERROR;

    /* Get the table offset */  
    xret = dt_get_table_node(dev->dt, dev->dt_p4offset, name, &node_offset); 
    if(xret != P4DEV_OK) return xret;

    /* Find the match subnode */
    node_offset = fdt_subnode_offset(dev->dt, node_offset, "match");
    if(node_offset < 0) {
        /* Ooops, error during the work with the device tree */
        return P4DEV_DEVICE_TREE_ERROR;
    }

    /* Extract the capacity */
    tmp_capacity = (fdt32_t*) fdt_getprop(dev->dt, node_offset, "row-count", &lenp);
    if(lenp < 0) {
        /* Ooops error during the work with the device tree */
        return P4DEV_DEVICE_TREE_ERROR;
    }   
    
    /* Convert the device tree integer to CPU's byte order and return success */
    *capacity = fdt32_to_cpu(*tmp_capacity);
    return P4DEV_OK;
}

uint32_t dt_get_key_len(const p4dev_t* dev, const char* name, uint32_t* keylen) {
    /* Variables */
    uint32_t xret;
    int32_t dt_table_offset;
    fdt32_t* tmp_bitwidth;
    int32_t match_offset; 
    int32_t dt_match_field; 
    int32_t lenp;

    /* Basic sanity check */
    if(!dev) {
        return P4DEV_NO_DEV;
    }

    if(!dev->dt) {
        return P4DEV_DEVICE_TREE_ERROR;
    }

    if(!name || !keylen) {
        return P4DEV_ERROR;
    }

    /* Get the table node, iterate over all subnodes, extract the match-size attribute and
     * sum the length of the key. */
    xret = dt_get_table_node(dev->dt, dev->dt_p4offset, name, &dt_table_offset);
    if(xret != P4DEV_OK) return xret;

    /* Iterate through key elements and build the configuration vector of 32 bit
     * transactions. */
    match_offset = fdt_subnode_offset(dev->dt, dt_table_offset, "match");
    if(match_offset < 0) return P4DEV_DEVICE_TREE_ERROR;

    *keylen = 0;
    fdt_for_each_subnode(dt_match_field, dev->dt, match_offset) {
        tmp_bitwidth = (fdt32_t*) fdt_getprop(dev->dt, dt_match_field, "match-size", &lenp);
        if(lenp < 0) return P4DEV_DEVICE_TREE_ERROR;
        
        /* Convert the value from fdt integer to byte-order of the target machine */
        *keylen += fdt32_to_cpu(*tmp_bitwidth);   
    }

    return P4DEV_OK;
}

