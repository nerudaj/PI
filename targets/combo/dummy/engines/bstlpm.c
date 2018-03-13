/*!
 *
 * \file bstlpm.c
 *
 * \brief Implementation of control functionality for LPM engine.
 *
 * \author Pavel Benacek <benacek@cesnet.cz>
 */
 /*
 * Copyright (C) 2018 CESNET
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

#include "bstlpm.h"
#include "standard.h"
#include "../p4dev_tree.h"
#include "tcam.h"
#include <combo.h>
#include <stdlib.h>

/* Include library code for the LPM BST engine */
#include "../lib/vector.h"

int32_t bstlpm_find(const p4dev_t* dev, bst_lpm_component_t* bst, const char* name) {
    /* Helping variables */
    uint32_t tab_offset;
    uint32_t xret;

    /* Basic sanity check */
    if(!dev || !bst || !name) {
        return P4DEV_ERROR;
    }

    /* Setup default values in the bst component */ 
    bst->space = NULL;

    /* Find the offset of the table */
    xret = dt_get_table_address_offset(dev->dt, dev->dt_p4offset, name, &tab_offset);
    if(xret != P4DEV_OK) return -xret;

    /* Fill the bst component with data */
    uint32_t tmp = cs_space_read_4(dev->cs, dev->cs_space, DEV_ADDRESS((uint32_t) tab_offset,LPM_REG_CMD_OFFSET));
    bst->key_width = tmp >> 16;
    bst->data_width = (tmp >> 8) & 0xff;
    bst->tree_stages = tmp & 0xff;
    if(bst->key_width < 1 || bst->data_width < 1 || bst->tree_stages < 1)
        return -P4DEV_ERROR;

    return tab_offset;
}

/*!
 * \brief Configure the device with given set
 *
 * \param [in] dev P4 device structure
 * \param [in] offset Offset of the search engine
 * \param [in] bst BST LPM structure with configuration
 * \param [in] data Uploaded data
 *
 * \warning The filtering should be disabled in the time of configuration
 * 
 * \return The number of free records in hardware. Negative value represents the state when code 
 * wasn't able to upload the data.
 */  
int32_t bstlpm_configure(const p4dev_t* dev, const uint32_t offset, bst_lpm_component_t* bst, const uint8_t *data) {
    unsigned item_words = ((bst->key_width-1)>>5) + ((bst->data_width-1)>>5) + 3;
    uint32_t *ptr32;
    unsigned rules = 1;
    unsigned records, addr;

    ptr32 = (uint32_t*)(data) + (item_words<<1) - 1; // check size of the set
    while(*ptr32) {
        ptr32 += item_words;
        rules += 1;
    }
    if(rules > bst_lpm_capacity(bst))
        return bst_lpm_capacity(bst) - rules;
    records = rules;

    // configure firmware filter
    ptr32 = (uint32_t*)data;
        // configure valid data
        for(unsigned i=0; i<records; i++, ptr32+=item_words)
            cs_space_write_multi_4(dev->cs, dev->cs_space, DEV_ADDRESS((uint32_t) offset,4), item_words, ptr32);
        // invalidate some records
        addr = records - 1;
        for(unsigned i=0; i< bst->tree_stages; i++) {
            if(addr & (1<<i)) {
                addr -= 1<<i;
            } else {
                cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS((uint32_t) offset, item_words<<2), addr | (1<<i));
            }
        }

    return bst_lpm_capacity(bst) - rules;
}

/*!
 * \brief Write command
 *
 * Write command into the component
 *
 * \param [in] dev P4 device structure
 * \param [in] table_name Name of the table
 * \param [in] val Value to write (ENABLE or DISABLE)
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - all tables are enabled
 *      - P4DEV_DEVICE_TREE_ERROR - error during the processing of device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_TABLE_NAME_ERROR - table name wasn't find in the \p rule parameter
 */
uint32_t bstlpm_write_cmd(const p4dev_t* dev, const char* table_name, uint32_t val) {
    /* Helping variables */
    bst_lpm_component_t bst;
    int32_t fret;
    
    /* Get the configuration of the device */ 
    fret = bstlpm_find(dev, &bst, table_name);
    if(fret < 0) return -fret;

    /* Write the command */
    cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS((uint32_t) fret,LPM_REG_CMD_OFFSET), val); 

    return P4DEV_OK;
}

uint32_t bstlpm_insert_rules(const p4dev_t* dev, const p4rule_t** p4rule, const uint32_t p4rule_count) {
    /* Helping variables */
    bst_lpm_component_t     bst;        // Component with configuration
    bst_lpm_prefix_set_t    set;        // Prefix set to upload
    int32_t                 fret;      
    int32_t                 cret;
    uint32_t                xret;
    unsigned char*          data;
    uint32_t                prefix_len;

    /* Key & data folks */
    uint32_t*               key;
    uint32_t                keylen;
    uint32_t*               action;
    uint32_t                actionlen;

    /* Check if we have something to upload */
    if(p4rule_count == 0) {
        return P4DEV_OK;
    }

    /* In this phase, we can be sure that all rules are designated to one table. 
     * Therefore, extract the table name from the rule */

    /* Get the configuration of the component */
    fret = bstlpm_find(dev, &bst, p4rule[0]->table_name);
    if(fret < 0) return -fret;

    /* Prepare the LPM set which will be configured into the device. We are going to use
     * the library functions which are supplied together with the BST LPM component. 
     */
    set = bst_lpm_prefix_set_new(&bst);
    if(!set) return P4DEV_ERROR;

    for(uint32_t i = 0; i < p4rule_count; i++) {
        /* Get the rule and prepare add it to the set. Notice that NULL pointers 
         * will be skipped. */
        if(!p4rule[i]) continue;

        /* Prepare the key to upload. Afterthat we need to prepare the data which
         * will be returened during the match. In this case, we can reuse the
         * TCAM action generator because the data structure is same */
        key = NULL;
        keylen = 0;
        prefix_len = 0;
        xret = bstlpm_prepare_key(dev->dt, dev->dt_p4offset, *p4rule[i], &key, &keylen, &prefix_len);
        if(xret != P4DEV_OK) {
            bstlpm_free_key(&key);
            bst_lpm_prefix_set_delete(set);
            return xret;
        }

        action = NULL;
        actionlen = 0;
        xret = tcam_prepare_action(dev->dt, dev->dt_p4offset, *p4rule[i], &action, &actionlen);
        if(xret != P4DEV_OK) {
            bstlpm_free_key(&key);
            tcam_free_action(&action);
            bst_lpm_prefix_set_delete(set);
            return xret;
        }

        /* So far so good ... everything is prepared, add the rule to the ruleset. Also handle the default rule
         * which may be also passed in the rule set. 
         */
        if(p4rule[i]->def) {
            /* Default rule is being processed */
            bst_lpm_prefix_set_set_default_data(set, (unsigned char*) action);
        } else {
            /* Normal rule is being processed */
            bst_lpm_prefix_set_add(set,(unsigned char*) key, prefix_len,(unsigned char*) action); 
        }

        /* Free keys and actions for the next iteration*/
        bstlpm_free_key(&key);
        tcam_free_action(&action);
    }

    /* Dump the prefix set and upload it into the device. */
    data = bst_lpm_prefix_set_dump(set);
    if(!data) {
        bst_lpm_prefix_set_delete(set);
        return P4DEV_ERROR;
    }

    cret = bstlpm_configure(dev, (uint32_t) fret, &bst, data);
    /* Return all allocated resources and check the return code */
    free(data);
    bst_lpm_prefix_set_delete(set);
    if(cret < 0) {
        return P4DEV_ERROR;
    }

    return P4DEV_OK;
}

uint32_t bstlpm_initialize_table(const p4dev_t* dev, const char* name) {
    /* Variables */
    uint8_t* tmp;
    bst_lpm_component_t     bst;    // Component with configuration
    uint32_t                size;   // Size to allocate    
    bst_lpm_prefix_set_t    set;    // Prefix set to upload
    unsigned char*          data;   // Pointer on dumped data
    int32_t                 xret;
    int32_t                 fret;  

    /* Get the configuration of the component */
    fret = bstlpm_find(dev, &bst, name);
    if(fret < 0) return -fret;

    /* Prepare the default data (all positions are filled with zeros) */
    size = ALLOC_SIZE(bst.data_width);
    tmp = (uint8_t*) malloc(sizeof(uint32_t)*size);
    if(!tmp) {
        return P4DEV_ERROR;
    }

    /* Create a set and setup the defualt rule --> all positions are filled 
     * with zeros */
    for(uint32_t i = 0; i < size*sizeof(uint32_t); i++) {
        tmp[i] = 0x0;
    }

    set = bst_lpm_prefix_set_new(&bst);
    if(!set) return P4DEV_ERROR;

    bst_lpm_prefix_set_set_default_data(set, (unsigned char*) tmp);
    data = bst_lpm_prefix_set_dump(set);

    /* Free allocated resources and check the return code */
    free(tmp);
    if(!data) {
        bst_lpm_prefix_set_delete(set);
        return P4DEV_ERROR;
    } 

    /* Upload the rule set, free allocated resources and return from the function */
    xret = bstlpm_configure(dev, (uint32_t) fret, &bst, data);
    bst_lpm_prefix_set_delete(set);
    free(data);

    if(xret < 0) return -xret;    
    
    return P4DEV_OK;
}

uint32_t bstlpm_enable(const p4dev_t* dev,const char* table_name) {
    return bstlpm_write_cmd(dev, table_name, LPM_CMD_ENABLE);
}

uint32_t bstlpm_disable(const p4dev_t* dev, const char* table_name) {
    return bstlpm_write_cmd(dev, table_name, LPM_CMD_DISABLE);
}

uint32_t bstlpm_prepare_key(const void* dt, const uint32_t dt_p4offset, const p4rule_t rule, 
                    uint32_t** outkey, uint32_t* keylen, uint32_t* prefixlen) {
    /* Declare variables */
    int32_t dt_table_node_offset;
    uint32_t xret; /*Helping variable for catching of return codes*/
    int32_t match_offset; /* Offset of match subnode */
    int32_t dt_match_field; /* Offset of match field subnode */
    int32_t lenp; /*Helping variable for reading of the property*/    

    fdt32_t* tmp_bitwidth; /* Device tree uint32_t-like type */
    uint32_t bitwidth; /* Bit width of the serialized key */
    char* key_name; /* Name of the key */

    p4key_elem_t* tmp_keyelem; /* Helping variable with temporal key element */
    uint32_t kbit_index = 0; /* Bit index in allocated key array */
    /* Sanity check of the input */
    if(!dt || !outkey || !keylen || !prefixlen) return P4DEV_ERROR;

    /* Take the P4 rule, iterate through all match elements. Find the corresponding node
     * in device tree, extract the required bit width and use this information
     * for creation of mask and search key */

    /* Find the correspoding table node */
    xret = dt_get_table_node(dt, dt_p4offset,rule.table_name, &dt_table_node_offset);
    if(xret != P4DEV_OK) return xret;

    /* Iterate through key elements and build the configuration vector of 32 bit
     * transactions. */
    match_offset = fdt_subnode_offset(dt, dt_table_node_offset, "match");
    if(match_offset < 0) {
        /*No such node has been foud, ending.*/
        return P4DEV_DEVICE_TREE_ERROR;
    }
  
    /* Iterate through all subnodes of match node. After that, found a corresponding record in 
     * the rule structure (key subpart). Initialize the number of transactions to 0 (no transaction is
     * created by default.) */
    *keylen = 0;   
    *prefixlen = 0; 
    fdt_for_each_subnode(dt_match_field, dt, match_offset) {
        /* Extract the name, and bit width. Find the correspoding p4key_elem_t in the rule structure
         * and append the element to the output vector. */
        key_name = (char*) fdt_getprop(dt, dt_match_field, "match-name", &lenp);
        if(lenp < 0) {
            /* Error during reading of the device tree */
            free_transaction(outkey);
            return P4DEV_KEY_NAME_ERROR;
        }       
        
        tmp_bitwidth = (fdt32_t*) fdt_getprop(dt, dt_match_field, "match-size", &lenp);
        if(lenp < 0) {
            /* Error during reading of the device tree */
            free_transaction(outkey);
            return P4DEV_DEVICE_TREE_ERROR;
        }
        /* Convert the value from fdt integer to byte-order of the target machine */
        bitwidth = fdt32_to_cpu(*tmp_bitwidth);   
        /* Search for the key element in the rule and dump it into the output vector */
        tmp_keyelem = find_key_element(rule, key_name);
        if(!tmp_keyelem) {
            /* No such element found; return the error */
            free_transaction(outkey);
            return P4DEV_KEY_NAME_ERROR;
        }
        /* Serialize the key to the array of 32-bit transactions */
        xret = serialize_to_transaction(tmp_keyelem->value, tmp_keyelem->val_size, &kbit_index, outkey, bitwidth);
        if(xret != P4DEV_OK) {
            /* Error during serialization */
            free_transaction(outkey);
            return xret;
        }

        /* Update the lenght of the prefix */
        *prefixlen += tmp_keyelem->opt.prefix_len;
    }
    /* Update the output bit length */
    *keylen = kbit_index;

    return P4DEV_OK;
}

void bstlpm_free_key(uint32_t** outkey) {
    /* Sanity check of the input */
    if(!outkey) return;
    /* Free pointers and set them to NULL */
    free(*outkey);
    *outkey = NULL;
}

