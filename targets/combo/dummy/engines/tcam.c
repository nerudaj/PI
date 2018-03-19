/*!
 *
 * \file tcam.c
 *
 * \brief Implementation of control functionality for TCAM engine.
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

#include "tcam.h"
#include "standard.h"
#include "../p4dev_tree.h"
#include <combo.h>
#include <stdlib.h>

/*!
 * \brief Insert the rule into attached P4 device.
 *
 * \param [in] dev P4 device structure
 * \param [in] p4rule Inserted rule.
 * \param [in] address Address of the the inserted record.
 *
 * \return The fuction returns following return codes.
 *      - P4DEV_OK - rule is inserted
 *      - P4DEV_NO_DEV - no device structure was passed in \p dev parameter
 *      - P4DEV_KEY_NAME_ERROR - the name of the key isn't in device tree model
 *      - P4DEV_ACTION_NAME_ERROR - the name of the function isn't in the device tree model
 *      - P4DEV_TABLE_NAME_ERROR - the name of the table isn't in the device tree model
 *      - P4DEV_BYTE_ARRAY_LENGTH_ERROR - one of passed byte vectors in \p rule parameter is smaller 
 *                                        than the required length in the device tree
 *      - P4DEV_RULE_ADDRESS_ERROR - the rule address is out of supported address scope
 *      - P4DEV_DEVICE_NOT_ATTACHED - the device is not attached
 *      - P4DEV_NO_DEVICE_TREE - device tree pointer on \p dev structure is invalid
 *      - P4DEV_DEVICE_TREE_ERROR - error during the parsing of the device tree
 *      - P4DEV_ERROR - other kind of error
 */
uint32_t tcam_insert_rule(const p4dev_t* dev, const p4rule_t* p4rule, uint32_t address) {
    /* The configuration consists of following steps:
     * 1) Identify the address of the table
     * 2) Prepare the key and mask
     * 3) Prepare action and parameters
     * 4) Prepare the inserted record (concatenate everything together and send it to the device) 
     * 5) Upload data to the device
     *
     * Details about the configuration protocol are available here:
     * https://homeproj.cesnet.cz/projects/fwbase/wiki/P4-device-confuration-protocol */

    uint32_t tab_offset; /* Adddress offset for the table */
    uint32_t ret;        /* Helping variable for catching of dt_ return codes */
    uint32_t* key;       /* Pointer with output searialized key */
    uint32_t* mask;      /* Pointer with output searialized mask */
    uint32_t keylen;     /* Bit length of the key and mask */
    uint32_t* action;    /* Pointer with output action */
    uint32_t actionlen;  /* Bit length of the action */
    uint32_t i;          /* Helping index variable */
    uint32_t max_index;  /* Maximal index of multiple transaction array */
    uint32_t tmp_cmd;    /* Temporal data read from MI32 */
    uint32_t max_tcap;   /* Maximal table capacity */
 
    /* Check if the device is already attached, raise error if not */
    if(!dev) {
        return P4DEV_NO_DEV;
    }

    if(!dev->cs || !dev->cs_space) {
        return P4DEV_DEVICE_NOT_ATTACHED;
    }

    if(!dev->dt) {
        return P4DEV_NO_DEVICE_TREE;
    }

    /* Check the rule pointer */
    if(!p4rule) {
        return P4DEV_ERROR;
    }

    /* Check the capacity of the table */
    ret = dt_get_table_capacity(dev, p4rule->table_name, &max_tcap);
    if(ret != P4DEV_OK) return ret;
    if(max_tcap < address) return P4DEV_RULE_ADDRESS_ERROR;
    
    /* Compute the table offset */
    ret = dt_get_table_address_offset(dev->dt, dev->dt_p4offset, p4rule->table_name, &tab_offset);
    if(ret != P4DEV_OK) return ret;

    /* Prepare the key */
    key = NULL;
    mask = NULL;
    keylen = 0;
    ret = tcam_prepare_key(dev->dt, dev->dt_p4offset, *p4rule, &key, &mask, &keylen);
    if(ret != P4DEV_OK) return ret;
    
    /* Prepare the action */
    action = NULL;
    actionlen = 0;
    ret = tcam_prepare_action(dev->dt, dev->dt_p4offset, *p4rule, &action, &actionlen);
    if(ret != P4DEV_OK) {
        tcam_free_key(&key,&mask);
        return ret;
    }

    /* Send data via MI32 .
     * Do the following for data insertion: 
     * 1) Upload address of the record
     * 2) Upload the key (may be in multiple transactions) 
     * 3) Upload the mask (may be in multiple transactions) 
     * 4) Upload the action (may be in multiple transactions)
     * 5) Send the Write command to command register
     * 6) Wait until the data is written (read data from command register and mask the value)
     */ 
    cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, STD_MI32_STD_MI32_REG_OFFSET), address);

    /* Check if you have something to write */
    if(keylen > 0) {
        max_index = ALLOC_SIZE(keylen)-1;
        for(i = 0; i <= max_index; i++) {
            cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, STD_MI32_KEY_REG_OFFSET), key[max_index-i]);
            cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, STD_MI32_MASK_REG_OFFSET), mask[max_index-i]);
        } 
    }

    max_index = ALLOC_SIZE(actionlen)-1;
    for(i = 0; i <= max_index; i++) {
        cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, STD_MI32_RECORD_REG_OFFSET), action[max_index-i]);
    }
    
    /* Keep the current configuration in command register */
    tmp_cmd = cs_space_read_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, CMD_REG_OFFSET));
    cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, CMD_REG_OFFSET), STD_MI32_CMD_WRITE_RECORD | tmp_cmd);

    while(true) {
        /* Read the command register, check if the masked value is 0x0 */
        tmp_cmd = cs_space_read_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, CMD_REG_OFFSET));
        if((tmp_cmd & STD_MI32_CMD_WRITE_RECORD) == 0x0) break;
    }  

    /* Free allocated structions */
    tcam_free_key(&key, &mask);
    tcam_free_action(&action);
    return P4DEV_OK;
}

uint32_t tcam_insert_rules(const p4dev_t* dev, const p4rule_t** p4rule, const uint32_t p4rule_count) {
    int32_t   def_index; /* Index of the default rule. The value -1 means that no default rule was found */
    uint32_t  tmp_addr; /* Variable with temporal address of the record */
    uint32_t  xret;     /* Returned code from the rule insertion function */
  
    /* Basic sanity check */
    if (!dev || !p4rule ) {
        return P4DEV_ERROR;
    }
    
    /* Search for the default rule in the rule array */
    def_index = -1;
    for(int32_t tmp = 0; tmp < p4rule_count; tmp ++) {
        def_index = tmp;
    }

    /* Insert rules into the device and initialize the address pointer */
    tmp_addr = TCAM_COMMON_RULE_ADDR;
    for(int32_t tmp = 0; tmp < p4rule_count; tmp ++) {
        if(!p4rule[tmp]) {
            /* No rule was detected */
            continue;
        }

        /* Check if we are inserting the default rule. Increment the address if common rule is inserted */
        if(def_index == tmp) {
            xret = tcam_insert_rule(dev,p4rule[tmp], TCAM_DEF_RULE_ADDR);
        } else {
            xret = tcam_insert_rule(dev,p4rule[tmp], (uint32_t) tmp_addr);
            tmp_addr++;
        } 

        /* Check the return code */
        if(xret != P4DEV_OK) {
            return xret;
        }
    }

    return P4DEV_OK;
}

uint32_t tcam_prepare_key(const void* dt, const uint32_t dt_p4offset, const p4rule_t rule, 
                    uint32_t** outkey, uint32_t** outmask, uint32_t* keylen) {
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
    uint32_t mbit_index = 0; /* Bit index in allocated mask array */
    const uint8_t  valid_bit  = {0x1}; /* Constant for the serialization of the valid bit */
    /* Sanity check of the input */
    if(!dt || !outkey || !outmask || !keylen) return P4DEV_ERROR;

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
    
    /* Append the valid bit on LSB position, the mask on given position is set to 1. 
     * Do that for both parts (mask and key) */
    xret = serialize_to_transaction(&valid_bit,1,&kbit_index,outkey,1);
    if(xret != P4DEV_OK) {
        return xret;
    }

    xret = serialize_to_transaction(&valid_bit,1,&mbit_index,outmask,1);
    if(xret != P4DEV_OK) {
        free_transaction(outkey);
        return xret;
    }

    fdt_for_each_subnode(dt_match_field, dt, match_offset) {
        /* Extract the name, and bit width. Find the correspoding p4key_elem_t in the rule structure
         * and append the element to the output vector. */
        key_name = (char*) fdt_getprop(dt, dt_match_field, "match-name", &lenp);
        if(lenp < 0) {
            /* Error during reading of the device tree */
            free_transaction(outkey);
            free_transaction(outmask);
            return P4DEV_KEY_NAME_ERROR;
        }       
        
        tmp_bitwidth = (fdt32_t*) fdt_getprop(dt, dt_match_field, "match-size", &lenp);
        if(lenp < 0) {
            /* Error during reading of the device tree */
            free_transaction(outkey);
            free_transaction(outmask);
            return P4DEV_DEVICE_TREE_ERROR;
        }
        /* Convert the value from fdt integer to byte-order of the target machine */
        bitwidth = fdt32_to_cpu(*tmp_bitwidth);   
        /* Search for the key element in the rule and dump it into the output vector */
        tmp_keyelem = find_key_element(rule, key_name);
        if(!tmp_keyelem) {
            /* No such element found; return the error */
            free_transaction(outkey);
            free_transaction(outmask);
            return P4DEV_KEY_NAME_ERROR;
        }
    
        /* Serialize the key to the array of 32-bit transactions */
        xret = serialize_to_transaction(tmp_keyelem->value, tmp_keyelem->val_size, &kbit_index, outkey, bitwidth);
        if(xret != P4DEV_OK) {
            /* Error during serialization */
            free_transaction(outkey);
            free_transaction(outmask);
            return xret;
        }

        xret = serialize_to_transaction(tmp_keyelem->opt.mask, tmp_keyelem->val_size, &mbit_index, outmask, bitwidth);
        if(xret != P4DEV_OK) {
            /* Error during serialization */
            free_transaction(outkey);
            free_transaction(outmask);
            return xret;
        }
    }
    /* Update the output bit length */
    *keylen = kbit_index;

    return P4DEV_OK;
}


uint32_t tcam_prepare_action(const void* dt, const uint32_t dt_p4offset, const p4rule_t rule, 
                    uint32_t** outaction, uint32_t* actionlen) {
    /* Declare variables */
    uint32_t xret; /*Helping variable for catching of return codes*/
    int32_t action_offset; /* Offset of action subnode */
    int32_t dt_table_node_offset; /* Temporal variable for storage of device tree offset */
    uint32_t opcode_width; /* The bit width of the opcode */
    int32_t lenp; /*Helping variable for reading of the property*/    

    fdt32_t* tmp_opcode_width; /* Working device tree bit width */
    uint8_t opcode; /*Operation code*/
    int8_t tmp_opcode; /* Temporal opcode value*/

    int32_t tmp_param_offset; /* Temporal parameter offset */
    char* param_name; /* Name of the parameter */
    fdt32_t*  tmp_param_width; /* Temporal working parameter width */
    uint32_t param_width; /* Bit width of the parameter */
    p4param_t* tmp_param; /* Pointer to the parameter structure */

    uint32_t bit_index = 0; /*Serialized bit-width index*/
    /* Sanity check of input */
    if(!dt || !outaction || !actionlen) return P4DEV_ERROR;
    
    /* Take the P4 rule, iterate through all parameter elements. First of all, 
     * extract the selected action and serialize the parameters behind it.
     * Find correspoding table node and check if we have a node to work with */
    xret = dt_get_table_node(dt, dt_p4offset, rule.table_name, &dt_table_node_offset);
    if(xret != P4DEV_OK) return xret;

    /* Prepare the action node */
    action_offset = fdt_subnode_offset(dt, dt_table_node_offset, "action");
    if(action_offset < 0) {
        /*No such node has been foud, ending.*/
        return P4DEV_DEVICE_TREE_ERROR;
    }

    /* Setup the default value of serialized parameter */
    *actionlen = 0;

    /* Extarct the opcoide width */
    tmp_opcode_width = (fdt32_t*) fdt_getprop(dt,action_offset, "opcode-width", &lenp);
    if(lenp < 0) {
        /* No such attribute was found */
        return P4DEV_DEVICE_TREE_ERROR;
    }
    /* Convert the value from the device tree to the CPU endianity */
    opcode_width = fdt32_to_cpu(*tmp_opcode_width); 
    
    /* Search for the opcode*/ 
    tmp_opcode = search_opcode(dt, &action_offset, rule);
    if(tmp_opcode < 0) {
       /* No mapping to action, ending. */
        return P4DEV_ACTION_NAME_ERROR;
    }
    
    /*Serialize the opcode into the action array. We can consider the opcode as one element array.
    * The width of inserted data is the width of opcode from the device tree. */
    opcode = (uint8_t) tmp_opcode;
    xret = serialize_to_transaction(&opcode, 1, &bit_index, outaction, opcode_width);
    if(xret != P4DEV_OK) {
        /* Ouch! Error during serialization, ending.*/
        tcam_free_action(outaction);
        return P4DEV_DEVICE_TREE_ERROR;
    }

    /* Iterate through the list of parameters */
    fdt_for_each_subnode(tmp_param_offset, dt, action_offset) {
        /* Take the node, extract name and width of parameter */ 
        param_name = (char*) fdt_getprop(dt, tmp_param_offset, "param-name", &lenp);
        if(lenp < 0) {
            /* Error during processing of the device tree. Free all allocated 
             * structures and return error. */
            tcam_free_action(outaction);             
            return P4DEV_DEVICE_TREE_ERROR;
        }      

        tmp_param_width = (fdt32_t*) fdt_getprop(dt, tmp_param_offset, "param-width", &lenp);
        if(lenp < 0) {
            /* The same like above, ending. */
            tcam_free_action(outaction);             
            return P4DEV_DEVICE_TREE_ERROR;
        }
        /* Convert the value to CPU endianity */
        param_width = fdt32_to_cpu(*tmp_param_width);
 
        /* Found the parameter in passed rule*/
        tmp_param = find_param_element(rule, param_name);
        if(!tmp_param) {
            /* No such parameter was found, ending. */
            tcam_free_action(outaction);             
            return P4DEV_PARAMETER_NAME_ERROR;
        }
        
        /* Serialize data to transaction */
        xret = serialize_to_transaction(tmp_param->value, tmp_param->val_size, &bit_index, outaction, param_width);
        if(xret != P4DEV_OK) {
            /* Error during serializatin, ending. */
            tcam_free_action(outaction);
            return xret;
        }
    }
    /* Update the serialized length */
    *actionlen = bit_index;
    
    return P4DEV_OK;
}

void tcam_free_key(uint32_t** outkey, uint32_t** outmask) {
    /* Sanity check of the input */
    if(!outkey || !outmask) return;
    /* Free pointers and set them to NULL */
    free(*outkey);
    free(*outmask);
    *outkey = NULL;
    *outmask = NULL;    
}

void tcam_free_action(uint32_t** outaction) {
    /* Sanity check of the input */
    if(!outaction) return;
    /* Free pointers and set them to NULL */
    free(*outaction);
    *outaction = NULL;
}


uint32_t tcam_initialize_table(const p4dev_t* dev, const char* name) {
    uint32_t table_capacity;    
    uint32_t curr_row;
    uint32_t xret;
    uint32_t table_offset;
    uint32_t tmp_cmd;
 
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
  
    /* Get the table capacity, write the invalidate signal
     * configure the address and send the write command */
    xret = dt_get_table_capacity(dev,name,&table_capacity);
    if(xret != P4DEV_OK) {
        /* Oops, error during the read of the table capacity */
        return xret;
    }    

    /* Get the table address we want to work with */
    xret = dt_get_table_address_offset(dev->dt,dev->dt_p4offset,name,&table_offset);
    if(xret != P4DEV_OK) return xret;
    
    curr_row = 0; 
    do {
        /* Upload the address */
        cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(table_offset, STD_MI32_STD_MI32_REG_OFFSET), curr_row);

        /* Upload the invalidation command */
        tmp_cmd = cs_space_read_4(dev->cs, dev->cs_space,DEV_ADDRESS(table_offset,CMD_REG_OFFSET));  
        cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(table_offset, CMD_REG_OFFSET), tmp_cmd | STD_MI32_CMD_INVALIDATE_ROW); 

        /* Upload the write command and wait untill the record is written*/
        tmp_cmd = cs_space_read_4(dev->cs, dev->cs_space,DEV_ADDRESS(table_offset,CMD_REG_OFFSET));  
        cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(table_offset, CMD_REG_OFFSET), tmp_cmd | STD_MI32_CMD_WRITE_RECORD); 
        while(true) {
            tmp_cmd = cs_space_read_4(dev->cs, dev->cs_space, DEV_ADDRESS(table_offset, CMD_REG_OFFSET));
            if((tmp_cmd & STD_MI32_CMD_WRITE_RECORD) == 0x0) break;
        } 
        
        /* Go to the next row */
        curr_row++;
    } while(curr_row <= table_capacity);

    return P4DEV_OK; 
}

uint32_t tcam_enable(const p4dev_t* dev,const char* table_name) {
    /* Enable of TCAM match action table is using the standard way */
    return write_cmd_to_table(dev, CMD_ENABLE_TABLE,false, table_name);
}

uint32_t tcam_disable(const p4dev_t* dev, const char* table_name) {
    /* Enable of TCAM match action table is using the standard way */
    return write_cmd_to_table(dev, CMD_DISABLE_TABLE, true, table_name);
}
