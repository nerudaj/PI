/*!
 *
 * \file p4dev_tree.h
 *
 * \brief Local header file with private helping device tree functions.
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

#include <stdint.h>
#include "p4dev_types.h"

#ifndef _P4DEV_TREE_H_
#define _P4DEV_TREE_H_

/*!
 * \defgroup dev_tree Device Tree helping library
 * \brief These functions and macros are used as the helping infrastructure for the work
 *        with generated DeviceTree description in P4 language.
 *
 * The specification of the device tree is available from:
 *  - https://homeproj.cesnet.cz/projects/fwbase/wiki/P4_device_tree
 *  - https://www.devicetree.org/
 *
 *  @{
 */

/*!
 * \brief Get the base address from the device tree desscription
 *
 * \param [in] dt Device tree blob. Can't be NULL.
 * \param [in] dt_p4offset Offset of the starting P4 node.
 * \param [out] base_addr Pointer on the output base address variable. Can't be NULL.
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - everything is OK and output address is in \p base_addr parameter.
 *      - P4DEV_DEVICE_TREE_ERROR - error during the work with the device tree (unable to read address from the 
 *                                  blob and so on).
 *      - P4DEV_ERROR - any of input parameter wasn't correct
 */
uint32_t dt_get_base_address(const void* dt, const uint32_t dt_p4offset, uint32_t* base_addr);

/*!
 *
 * \brief Prepare the table address.
 *
 * The function computes and returnes the offset of the address which can be used for acessing of the table
 * in mapped address space.
 *
 * \param [in] dt Device tree blob.
 * \param [in] dt_p4offset Starting offset in the device tree.
 * \param [in] table_name Name of the table.
 * \param [out] ret_addr Pointer on the variable where the address will be returned
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - everything is OK and output address is in \p ret_addr parameter.
 *      - P4DEV_DEVICE_TREE_ERROR - error during the work with device tree (unable to read address from the 
 *                                  blob and so on).
 *      - P4DEV_ERROR - any of input parameters wasn't correct.
 *      - P4DEV_TABLE_NAME_ERROR - table in \p rule wasn't find
 */
uint32_t dt_get_table_address_offset(const void* dt, const uint32_t dt_p4offset, const char* table_name, uint32_t* ret_addr);


/*!
 * \brief Identify the offset of the node based on the name
 *
 * \param [in] dt Device tree blob.
 * \param [in] dt_p4offset Starting offset in the device tree.
 * \param [in] table_name Name of the searched table.
 * \param [out] node_offset Offset of searched node. The value is < 0 when
 *                          no corresponding node is found.
 *
 * \return The function returns following codes
 *      - P4DEV_OK - the output offset is stored in the \p node_offset parameter
 *      - P4DEV_DEVICE_TREE_ERROR - error during reading of the device tree
 *      - P4DEV_ERROR - any of input parameters wasn't correct
 *      - P4DEV_TABLE_NAME_ERROR - table name wasn't find in the \p rule parameter
 */
uint32_t dt_get_table_node(const void* dt, const uint32_t dt_p4offset, const char* table_name, int32_t* node_offset);

/*!
 * \brief Get the maximal amount of supported table records
 *
 * \param [in] dev The P4 device structure
 * \param [in] name Name of the investigated table
 * \param [out] capacity Pointer on output variable which will be filled with read data 
 *
 * \return The function returns following codes
 *      - P4DEV_OK - everything is OK and the capacity is stored in \p capacity argument
 *      - P4DEV_DEVICE_TREE_ERROR - throwed if there was any error during parsing of device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_TABLE_NAME_ERROR - the name of the table isn't in the device tree model
 *      - P4DEV_DEVICE_NOT_ATTACHED - device is not attached
 *      - P4DEV_ERROR - the desired table wasn't found or the capacity pointer is NULL
 */
uint32_t dt_get_table_capacity(const p4dev_t* dev, const char* name, uint32_t* capacity);

/*!
 * \brief Extract the length of the key from the Device Tree description
 *
 * \param [in] dev P4 device structure
 * \param [in] name Name of the investigated table
 * \param [in] keylen Pointer on output variable which will be filled with the length of the key
 *
 * \return The function returns P4DEV_OK iff everything was OK. The returned code 
 * can be investigated using the \ref p4dev_err_stdout or \ref p4dev_err_copy.
 */
uint32_t dt_get_key_len(const p4dev_t* dev, const char* name, uint32_t* keylen);

/*! @}*/

#endif /*_P4DEV_TREE_H_*/
