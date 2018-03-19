/*!
 *
 * \file tcam.h
 *
 * \brief Header file with definition of control functionality for TCAM 
 *        engine.
 *
 * \author Pavel Benacek <benacek@cesnet.cz>
 */
 /*
 * Copyright (C) 2016,2017 CESNET
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

#ifndef _TCAM_H_
#define _TCAM_H_

/*!
 * \defgroup tcam TCAM engine 
 * \brief TCAM engine functions for working with rules, description of address space, etc.
 * @{ 
 */
/*! \brief Address of the default rule in the TCAM*/
#define TCAM_DEF_RULE_ADDR          0x0

/*! \brief Starting address for normal rules in the TCAM */
#define TCAM_COMMON_RULE_ADDR       0x1

/*!
 * \brief Insert the rule into attached P4 device. 
 *
 * The passed rule set is inserted based on the position in the position in ruleset.
 * Firstly, the rule array is scanned for the default rule where the lastly detected
 * rule is selected for the insertion. After that, remaining rules are inserted based 
 * on the position in the rule array starting from the index 1.
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
 */
uint32_t tcam_insert_rules(const p4dev_t* dev, const p4rule_t** p4rule, const uint32_t p4rule_count);

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
uint32_t tcam_initialize_table(const p4dev_t* dev, const char* name);

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
uint32_t tcam_enable(const p4dev_t* dev,const char* table_name);

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
uint32_t tcam_disable(const p4dev_t* dev, const char* table_name);

/*!
 * \brief Prepare the key 
 *
 * The function prepares the bit representation which will be uploaded to the device. All allocated
 * structures have to be cleaned with \ref tcam_free_key function. The function also appends one bit
 * on LSB position which means that rule is valid. 
 *
 * \param [in] dt Device tree to use. The parameter shouldn't be NULL.
 * \param [in] dt_p4offset Starting offset in the device tree.
 * \param [in] rule Inserted rule.
 * \param [out] outkey Pointer which will be filled with allocated array. It is recommended 
 *              to set the initial value to \p NULL.
 * \param [out] outmask Pointer which will be filled with allocated array. It is recommended 
 *              to set the initial value to \p NULL.
 * \param [out] keylen Length of the key and mask (in bits). The length of the array can be computed
 *              using the \ref ALLOC_SIZE macro.
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
uint32_t tcam_prepare_key(const void* dt, const uint32_t dt_p4offset, const p4rule_t rule, 
                    uint32_t** outkey, uint32_t** outmask, uint32_t* keylen);

/*!
 * \brief Prepare the action
 *
 * The function prepares the bit representation which will be uploaded to the device. The allocated
 * strucutre has to be cleaned with \ref tcam_free_action function.
 *
 * \param [in] dt Device tree to use. The parameter shouldn't be NULL.
 * \param [in] dt_p4offset Starting offset in the device tree.
 * \param [in] rule Inserted rule.
 * \param [out] outaction Pointer which will be filled with allocated array. It is recommended 
 *              to set the initial value to \p  NULL.
 * \param [out] actionlen Lenght of the action (in bits). The length of the array can be computed
 *              using the \ref ALLOC_SIZE macro.
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - the creation of key and mask transaction have been successfull
 *      - P4DEV_ACTION_NAME_ERROR - the action name wasn't found in \p rule but it was found in the device tree
 *      - P4DEV_DEVICE_TREE_ERROR - there was any error during work with the device tree
 *      - P4DEV_BYTE_ARRAY_LENGTH_ERROR - one of passed byte vectors in \p rule parameter is smaller 
 *                                        than the required length in the device tree
 *      - P4DEV_ERROR - any of input parameters wasn't correct
 *      - P4DEV_TABLE_NAME_ERROR - table name wasn't find in the \p rule parameter
 *      - P4DEV_PARAMETER_NAME_ERROR - name the serialized parameter wasn't find in \p rule parameter
 */
uint32_t tcam_prepare_action(const void* dt, const uint32_t dt_p4offset, const p4rule_t rule, 
                    uint32_t** outaction, uint32_t* actionlen);

/*!
 * \brief Free the allocated structures and set the pointers to NULL
 *
 * \param [in] outkey Pointer on allocated structure
 * \param [in] outmask Pointer on allocated strcuture
 */
void tcam_free_key(uint32_t** outkey, uint32_t** outmask);

/*!
 * \brief Free allocated structures and set the pointer to NULL
 *
 * \param [in] outaction Pointer on allocated structure
 */
void tcam_free_action(uint32_t** outaction);

/*! @} */

#endif /* _TCAM_H_ */
