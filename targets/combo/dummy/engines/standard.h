/*!
 *
 * \file standard.h
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

#include <math.h>
#include <stdlib.h>
#include <libfdt.h>
#include "../p4rule.h"
#include "../p4dev_types.h"

#ifndef _STANDARD_H_
#define _STANDARD_H_

/*!
 * \defgroup standardlib Standard library for work of Match+Action Tables
 * \brief These functions and macros are planned to be used in variety of search
 *        engines. It contains functions for the serialization, searching in linked list, etc.
 *
 *  @{
 */

/*! \brief Real allocated size can be computed based on the bit length
 *
 * \param [in] x Bit lenght of allocated record
 */
#define ALLOC_SIZE(x) ((uint32_t) ceil(x/32.0)) 

/*!
 * \brief Macro for computation of address to the hardware
 */
#define DEV_ADDRESS(base,offset) ((base) + (offset))

/*! \brief Offfset of command register */
#define CMD_REG_OFFSET          0x0
/*! \brief Disable the table */
#define CMD_DISABLE_TABLE       0x0
/*! \brief Enable the table  */
#define CMD_ENABLE_TABLE        0x1
/*! \brief Busy flag -- the device is still processing packets in the match action table */
#define CMD_BUSY_FLAG           0x4

/*! \brief Invalidate the search key and row registers. This command is used 
 *  during the initilization of the row.  */
#define STD_MI32_CMD_INVALIDATE_ROW  0x8
/*! \brief Write the record  */
#define STD_MI32_CMD_WRITE_RECORD    0x2

/*! \brief Offset of the record address register */
#define STD_MI32_STD_MI32_REG_OFFSET 0x4
/*! \brief Offset of the key register (multiple transactions) */
#define STD_MI32_KEY_REG_OFFSET      0x8
/*! \brief Offset of the mask register (multiple transactions) */
#define STD_MI32_MASK_REG_OFFSET     0xC
/*! \brief Offset of the inserted record -- operation, parameters (multiple transactions) */
#define STD_MI32_RECORD_REG_OFFSET   0x10

/*!
 * \brief Serialize the vector into uint32_t transaction array
 *
 * The function performs the serialization into the array vector. The function
 * is designed to allocate/reallocate the passed array based on the length of serialized data.
 *
 * \param [in] data Data to serialize
 * \param [in] data_len Length of the array in bytes.
 * \param [inout] bit_index Used bits of uint32_t transaction array. The initial value should be set to 0.
 * \param [inout] trans_data Transaction data. The array is allocated when the value of parameter equals to NULL.
 * \param [in] bitwidth Width of the serialized element.
 *
 * \return The function returns following erros:
 *      - P4DEV_OK - Inpute data was successfully appneded. All inout parameters were updated.
 *      - P4DEV_ERROR - Error during the serialization (due to unexpected situation or due to bad input parameters)
 *      - P4DEV_BYTE_ARRAY_LENGTH_ERROR - the passed byte vector is smaller than is required in \p bitwidth parameter
 */
uint32_t serialize_to_transaction(const uint8_t* data, const uint32_t data_len, uint32_t* bit_index, uint32_t** trans_data, 
                                  const uint32_t bitwidth);

/*!
 * \brief Clean everything regarding to serialized data
 *
 * \param [inout] trans_data Pointer to the array with allocated data
 */
void free_transaction(uint32_t** trans_data);

/*!
 * \brief Search for the operation code in passed device tree and rule parameters.
 *
 * \param [in] dt Device tree blob
 * \param [inout] offset Starting offset in the Device Tree. The offset is used
 *             as a stargint point during iteration through all action nodes. 
 *             The variable also contains the updated offset of found action node.
 * \param [in] rule Processed rule
 *
 * \return Found operation code (if value is >= 0). The code is not found when the returned value
 * is negative (it typically equals to the -\ref P4DEV_ERROR value) 
 */
int8_t search_opcode(const void* dt, int32_t* offset, const p4rule_t rule);

/*!
 * \brief Write a given command to each table in P4 design.
 *
 * \param [in] dev Pointer on P4 device structure
 * \param [in] val Value to write
 * \param [in] wait Wait on the busy flag if required.
 * \param [in] table_name Name of the table to process
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - all tables are enabled
 *      - P4DEV_DEVICE_TREE_ERROR - error during the processing of device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_DEVICE_NOT_ATTACHED - device is not attached
 *      - P4DEV_TABLE_NAME_ERROR - table name wasn't find in the \p rule parameter
 */
uint32_t write_cmd_to_table(const p4dev_t* dev, uint32_t val, bool wait, const char* table_name);

/*!
 * \brief Return the \ref p4key_elem_t structure with given name
 *
 * \param [in] rule Searched rule.
 * \param [in] name Name of the key.
 *
 * \return Not NULL value if the element was found.
 */
p4key_elem_t* find_key_element(const p4rule_t rule, const char* name);

/*!
 * \brief Return the \ref p4param_t strcture with given name
 *
 * \param [in] rule Processed rule.
 * \param [in] name Name of the key.
 *
 * \return Not NULL value if th element was found.
 */
p4param_t* find_param_element(const p4rule_t rule, const char* name);
/*! @}*/

#endif /*_COMMON_H_ */
