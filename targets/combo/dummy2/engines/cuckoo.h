/*!
 *
 * \file cuckoo.h
 *
 * \brief Header file with definition of control functionality for Cuckoo engine.
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

#ifndef _CUCKOO_H_
#define _CUCKOO_H_

/*!
 * \defgroup cuckoo Cuckoo engine 
 * \brief Cuckoo engine functions for working with rules, description of address space, etc.
 * @{ 
 */

/*!
 * \brief Insert the rule into attached P4 device. 
 *
 * The passed rule set is inserted based on the position in the position in ruleset.
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
 *      - P4DEV_UNABLE_TO_INSERT - function wasn't able to insert all rules.
 */
uint32_t cuckoo_insert_rules(const p4dev_t* dev, const p4rule_t** p4rule, const uint32_t p4rule_count);

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
uint32_t cuckoo_initialize_table(const p4dev_t* dev, const char* name);

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
uint32_t cuckoo_enable(const p4dev_t* dev,const char* table_name);

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
uint32_t cuckoo_disable(const p4dev_t* dev, const char* table_name);

/*!
 * \brief Extract basic properties from the device tree and other sources
 * 
 * \param [in] dev P4 device structure
 * \param [in] keylen Pointer on the variable which will be filled with the keylength
 * \param [in] table_name Name of the processed table.
 * \param [in] tables Pointer on the variable which will be filled with the number of instantiated keys
 * \param [in] lines Pointer on the variable which will be filled with the number of instantiated lines
 *
 * \return The function returns P4DEV_OK if everything was OK. 
 */ 
uint32_t cuckoo_get_properties(const p4dev_t* dev, const char* table_name, uint32_t* keylen, uint32_t* tables, uint32_t* lines);

/*! @} */

#endif /* _CUCKOO_H_ */
