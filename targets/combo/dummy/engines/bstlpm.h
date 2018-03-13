/*!
 *
 * \file bstlpm.h
 *
 * \brief Header file with definition of control functionality for LPM engine.
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

#include "../p4dev_types.h"
#include "../p4rule.h"
#include "../lib/bst_lpm.h"

#ifndef _BSTLPM_H_
#define _BSTLPM_H_

/*!
 * \defgroup bstlpm LPM engine 
 * \brief LPM engine functions for working with rules, description of address space, etc.
 * \todo Implement the get capacity handler
 *
 * @{ 
 */

/*! \brief Offset of the configuration register */
#define LPM_REG_CMD_OFFSET  0x0

/*! \brief Command for disabling of the search engine */
#define LPM_CMD_DISABLE     0x0

/*! \brief Command for enabling of the search engine */
#define LPM_CMD_ENABLE      0x1

/*!
 * \brief Insert the rule into attached P4 device. 
 *
 * The passed rule set is inserted based on the position in the position in ruleset.
 *
 * \warning Nothing is done iff the p4rule_count equals to 0.
 *
 * \param [in] dev P4 device structure
 * \param [in] p4rule Inserted rule set. The rule is skipped if the pointer value equals to NULL.
 * \param [in] p4rule_count Number of inserted rules
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
 *      - P4DEV_ERROR - there was detected another kind of bad behavior
 */
uint32_t bstlpm_insert_rules(const p4dev_t* dev, const p4rule_t** p4rule, const uint32_t p4rule_count);

/*!
 * \brief Initialize the table into the default state.
 *
 * \param [in] dev P4 device structure
 * \param [in] name Name of the table
 *
 * \return The function returns following return codes.
 *      - P4DEV_OK - rule is inserted
 *      - P4DEV_NO_DEV - no device structure was passed in \p dev parameter
 *      - P4DEV_NO_DEVICE_TREE - device tree pointer on \p dev structure is invalid
 *      - P4DEV_DEVICE_TREE_ERROR - error during the parsing of the device tree
 *      - P4DEV_TABLE_NAME_ERROR - the name of the table isn't in the device tree model
 */
uint32_t bstlpm_initialize_table(const p4dev_t* dev, const char* name);

/*!
 * \brief Enable the P4 pipeline
 *
 * The function enables the packet processing in Match+Action table.
 *
 * \param [in] dev P4 device structure
 * \param [in] table_name Name of the table
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - all tables are enabled
 *      - P4DEV_DEVICE_TREE_ERROR - error during the processing of device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_TABLE_NAME_ERROR - table name wasn't find in the \p rule parameter
 */
uint32_t bstlpm_enable(const p4dev_t* dev,const char* table_name);

/*!
 * \brief Disable the P4 pipeline
 *
 * The function disables the packet processing in Match+Action table.
 *
 * \param [in] dev P4 device structure
 * \param [in] table_name Name of the table.
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - all tables are enabled
 *      - P4DEV_DEVICE_TREE_ERROR - error during the processing of the device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_DEVICE_NOT_ATTACHED - device is not attached
 *      - P4DEV_TABLE_NAME_ERROR - table name wasn't find in the \p rule parameter
 */
uint32_t bstlpm_disable(const p4dev_t* dev, const char* table_name);

/*!
 * \brief Prepare the key 
 *
 * The function prepares the bit representation which will be uploaded to the device. All allocated
 * structures have to be cleaned with \ref bstlpm_free_key function.
 *
 * \param [in] dt Device tree to use. The parameter shouldn't be NULL.
 * \param [in] dt_p4offset Starting offset in the device tree.
 * \param [in] rule Inserted rule.
 * \param [out] outkey Pointer which will be filled with allocated array. It is recommended 
 *              to set the initial value to \p NULL.
 * \param [out] keylen Length of the key in bits. The length of the array can be computed
 *              using the \ref ALLOC_SIZE macro.
 * \param [out] prefixlen Prefix length of the created rule. This information is computed from the
 *              optional part of the created rule.
 *
 * \warning The allocated key should be freed using the \ref bstlpm_free_key function. Also, the \p outkey
 * parametrs has to be set to NULL in the first round.
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - the creation of key and mask transaction has been successfull
 *      - P4DEV_KEY_NAME_ERROR - the key name wasn't found in the \p rule but it was found in the device tree
 *      - P4DEV_DEVICE_TREE_ERROR - there was any error during the work with the device tree
 *      - P4DEV_ERROR - any of input parameters wasn't correct
 *      - P4DEV_BYTE_ARRAY_LENGTH_ERROR - one of passed byte vectors in \p rule parameter is smaller 
 *                                        than the required length in the device tree
 *      - P4DEV_TABLE_NAME_ERROR - table name wasn't find in the \p rule parameter
 */
uint32_t bstlpm_prepare_key(const void* dt, const uint32_t dt_p4offset, const p4rule_t rule, 
                    uint32_t** outkey, uint32_t* keylen, uint32_t* prefixlen);

/*!
 * \brief Free the allocated structures and set the pointers to NULL
 *
 * \param [in] outkey Pointer on allocated structure
 */
void bstlpm_free_key(uint32_t** outkey);

/*!
 * \brief Find the LPM  component using the device tree 
 * 
 * \param [in] dev P4 device structure
 * \param [out] bst LPM BST configuration
 * \param [in] name Name of the table we want to work with.
 *
 * \todo Make this function statefull (we don't want to call it during each call of bst_lpm*
 * functions)
 *
 * \return The function returns offset of the component in address space. Negative
 * value is returned in the case of any error (the value is a negative \ref P4DEV_RETURN_CODES).
 */ 
int32_t bstlpm_find(const p4dev_t* dev, bst_lpm_component_t* bst, const char* name);

/*! @} */

#endif /* _BSTLPM_H_ */
