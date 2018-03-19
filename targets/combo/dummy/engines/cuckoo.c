/*!
 *
 * \file tcam.c
 *
 * \brief Implementation of control functionality for Cuckoo engine.
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

#include "cuckoo.h"
#include "standard.h"
#include "../p4dev_tree.h"
#include "../lib/cuckoo_lib.h"
#include "bstlpm.h"
#include "tcam.h"

#include <combo.h>
#include <stdlib.h>

/*!
 * \brief Number of instantiated tables in the search engine.
 * \todo Remove this after the reading of the table (from DT) will be implemented
 */
#define TABLE_COUNT     3

uint32_t cuckoo_get_properties(const p4dev_t* dev, const char* table_name, uint32_t* keylen, uint32_t* tables, uint32_t* lines) {
    /* Variables */
    uint32_t xret;

    /* Basic sanity check */
    if(!dev) {
        return P4DEV_NO_DEV;
    }

    if(!keylen || !tables || !lines) {
        return P4DEV_ERROR;
    }

    /* Setup the number of allocated tables */
    *tables = TABLE_COUNT;

    /* Extract the number of lines */
    xret = dt_get_table_capacity(dev, table_name, lines);
    if(xret != P4DEV_OK) return xret;

    /* Extract the length of the key from the device tree */
    xret = dt_get_key_len(dev,table_name,keylen);
    if(xret != P4DEV_OK) return xret;

    return P4DEV_OK;
}

/*!
 * \brief Upload rules into the device
 *
 * \param [in] dev P4 device structure
 * \param [in] p4rule P4 rule array to upload
 * \param [in] p4rule_count Number of rules to upload
 * \param [in] tab Cuckoo table structure with rule ordering
 * \param [in] name Name of the table we process
 *
 * \return The function returns \ref P4DEV_OK iff everything was OK.
 */
static uint32_t cuckoo_upload_rules(const p4dev_t* dev, const p4rule_t** p4rule, const uint32_t p4rule_count, __cuckoo_table_t* tab, const char* name) {
    /* Variables */
    uint32_t*           key;            // Serialized key
    uint32_t            keylen;         // Length of the key
    uint32_t            tmp;
    uint32_t*           action;         // Vector with action and parameters
    uint32_t            actionlen;      // Length of serialized data
    uint32_t            tab_offset;     // Offset of the table
    uint32_t            xret;           // Temporal variable with returned code
    uint32_t            address;        // Address in the table
    uint32_t            max_index;      // Maximal index
    uint32_t            tmp_cmd;        // 
    int                 fret;           

    /* Prepare the offset of the table */
    xret = dt_get_table_address_offset(dev->dt, dev->dt_p4offset, name, &tab_offset);
    if(xret != P4DEV_OK) return xret;

    /* No sanity check is required because we did it before. */
    for(uint32_t i = 0; i < p4rule_count; i++) {
        /* Prepare the key, parameters and other necessary data. We can reuse the code from the TCAM because construction of
         * parameters is same.*/
        key    = NULL;
        keylen = 0;
        xret = bstlpm_prepare_key(dev->dt, dev->dt_p4offset, *p4rule[i], &key, &keylen, &tmp);
        if(xret != P4DEV_OK) {
            return P4DEV_ERROR;
        }

        action    = NULL;
        actionlen = 0;
        xret = tcam_prepare_action(dev->dt, dev->dt_p4offset, *p4rule[i], &action, &actionlen);
        if(xret != P4DEV_OK) {
            bstlpm_free_key(&key);
            return P4DEV_ERROR;
        } 

        /* Extract the address from the cuckoo table */
        fret = __cuckoo_table_find(tab, key, &address); 
        if(!fret) {
            /* Uuups ... no rule was found */
            bstlpm_free_key(&key);
            tcam_free_action(&action);
            return P4DEV_ERROR;
        }

        /* Send data via MI32 .
         * Do the following for data insertion: 
         * 1) Upload address of the record
         * 2) Upload the action (may be in multiple transactions)
         * 3) Send the Write command to command register
         * 4) Wait until the data is written (read data from command register and mask the value)
         */ 
        cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, STD_MI32_STD_MI32_REG_OFFSET), address);

        max_index = ALLOC_SIZE(actionlen)-1;
        for(uint32_t i = 0; i <= max_index; i++) {
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
    }

    return P4DEV_OK;
}

uint32_t cuckoo_insert_rules(const p4dev_t* dev, const p4rule_t** p4rule, const uint32_t p4rule_count) {
    /* Prepare variables */
    __cuckoo_table_t    tab;            // Cuckoo hash structure 
    uint32_t            keylen;         // Length of the key
    uint32_t            tables;         // Number of tables
    uint32_t            lines;          // Number of lines
    uint32_t            xret;           // Returned value from p4lib functions
    int                 fret;           // Returned value from cuckoo table library
    const char*         table_name;     // Name of the table
    uint32_t            bits;           // Bit length required for the identification of line
    uint32_t*           serkey;         // Serialized key
    uint32_t            tmp;

    /* Some basic sanity check */
    if(!dev) {
        return P4DEV_NO_DEV;
    }

    if(!p4rule || p4rule_count == 0) {
        return P4DEV_ERROR;
    }

    /* Extract the name of the table. This parameter can be taken from the rule because
     * all rules are designated to one table only */
    table_name = p4rule[0]->table_name;

    /* Create a rule set and compute the address, etc. */
    xret = cuckoo_get_properties(dev,table_name,&keylen,&tables,&lines);
    if(xret != P4DEV_OK) return xret;

    bits = (uint32_t) ceil(log2(lines));
    __cuckoo_table_init(&tab);
    fret = __cuckoo_table_alloc(&tab, tables, bits, keylen);
    if(!fret) {
        __cuckoo_table_free(&tab);
        return P4DEV_ERROR;
    }

    /* Prepare the organization of records in Cuckoo table */
    for(uint32_t i = 0; i < p4rule_count; i++) {
        /* Prepare the key - we can use the function from BST LPM version */
        serkey = NULL;
        keylen = 0;
        xret = bstlpm_prepare_key(dev->dt, dev->dt_p4offset, *p4rule[i], &serkey, &keylen, &tmp);
        if(xret != P4DEV_OK) {
            __cuckoo_table_free(&tab);
            return P4DEV_ERROR;
        }

        /* Insert all keys into the set --> compute the rule ordering in the table */
        fret = __cuckoo_table_add(&tab, serkey);
        if(!fret) {
            __cuckoo_table_free(&tab);
            return P4DEV_ERROR;
        }

        /* Free the key and prepare the variable for the next iteration */
        bstlpm_free_key(&serkey);
    }

    /* Iterate again over all elements, prepare data and insert them into the table.
     * Oh .. don't forget to cleanup all data! */
    xret = cuckoo_upload_rules(dev, p4rule, p4rule_count, &tab, table_name);
    __cuckoo_table_free(&tab);
    return xret;
}

uint32_t cuckoo_initialize_table(const p4dev_t* dev, const char* name) {
    /* Variables */
    uint32_t keylen;
    uint32_t tables;
    uint32_t lines;
    uint32_t max_index;
    uint32_t tab_offset;
    uint32_t address;
    uint32_t bits;
    uint32_t tmp_cmd;
    uint32_t xret;
    
    /* Sanity check of input */ 
    if(!dev) {
        return P4DEV_NO_DEV;
    }

    if(!name) {
        return P4DEV_ERROR;
    }

    /* Create a rule set and compute the address, etc. */
    xret = cuckoo_get_properties(dev,name,&keylen,&tables,&lines);
    if(xret != P4DEV_OK) return xret;
    
    /* Prepare the offset of the table */
    xret = dt_get_table_address_offset(dev->dt, dev->dt_p4offset, name, &tab_offset);
    if(xret != P4DEV_OK) return xret;
     
    /* Iterate over all possible addresses and upload default action (vector of zeros) */
    max_index = ALLOC_SIZE(keylen)-1;
    for(uint32_t i = 0; i <= max_index; i++) {
        cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, STD_MI32_RECORD_REG_OFFSET), 0x0);
    }
    
    /* Iterate over all possible addresses and tables */
    bits = (uint32_t) ceil(log2(lines));
    for(uint32_t tab = 0; tab < tables; tab++) {
        for(uint32_t line = 0; line < lines; line++) {
            address = (tab << bits) | line;

            /* Write the address and start the write command */
            cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, STD_MI32_STD_MI32_REG_OFFSET), address);
            tmp_cmd = cs_space_read_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, CMD_REG_OFFSET));
            cs_space_write_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, CMD_REG_OFFSET), STD_MI32_CMD_WRITE_RECORD | tmp_cmd);

            while(true) {
                /* Read the command register, check if the masked value is 0x0 */
                tmp_cmd = cs_space_read_4(dev->cs, dev->cs_space, DEV_ADDRESS(tab_offset, CMD_REG_OFFSET));
                if((tmp_cmd & STD_MI32_CMD_WRITE_RECORD) == 0x0) break;
            }  
        }
    }

    return P4DEV_OK;
}

uint32_t cuckoo_enable(const p4dev_t* dev,const char* table_name) {
    /* The engine shares the MI32 endpoint with TCAM --> we can use the TCAM version */
    return tcam_enable(dev,table_name);
}

uint32_t cuckoo_disable(const p4dev_t* dev, const char* table_name) {
    return tcam_disable(dev,table_name);
}

