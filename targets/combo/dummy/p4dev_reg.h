/*!
 *
 * \file p4dev_reg.h
 *
 * \brief Local header file of p4dev_reg.h with function for creting configuration
 *        stream of used registers.
 *
 * \author Pavel Kohout <xkohou15@stud.fit.vutbr.cz>
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

#include "p4dev_types.h" 

#ifndef _P4DEV_REG_H_
#define _P4DEV_REG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup registers Registers and folks
 * \brief Functions for direct work with registers and other stateful elements.
 * @{ 
 */

/*!
 * \brief Type of the register binding
 */
typedef enum {
    BINDING_DIRECT,     /*!< Direct binding of registers (each row in the table has its own register cell) */
    BINDING_STATIC,     /*!< Static binding (register cells are designated to the table) */
    BINDING_GLOBAL      /*!< Register cells are accessible from any table */
} register_bind_t;

/*!
 * \brief Structure with basic data of register array
 */
typedef struct p4_register {
    char*               name;       /*!< Name of the array */
    uint32_t            width;      /*!< Bit width of the register cell */    
    uint32_t            offset;     /*!< Offset of the register array in the device address space */
    uint32_t            count;      /*!< Number of registers in the array */
    register_bind_t     bind_type;  /*!< Type of the register binding */
    char*               bind_table; /*!< Assigned table. The value is NULL in the case of the global binding */
} p4_register_t;

/*!
 * \brief Read value from register on index \p reg_ind and store in \p read_data
 *
 * \param [in] dev The P4 device structure
 * \param [in] reg Structure with register data
 * \param [out] read_data Pointer on array where read data will be stored. 
 *          Data are stored in the little endian order.
 * \param [in] length Size of \p read_data array. 
 * \param [in] reg_ind Index of target memory cell in register array.
 *          Register cells are numbered from 0.
 *
 * \warning The device should be disabled before the call of this function
 *
 * \return The function returns following codes
 *      - P4DEV_OK - everything is OK and data are stored in array
 *      - P4DEV_DEVICE_TREE_ERROR - throwed if there was any error during parsing of device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_SMALL_BUFFER - the length of the passed buffer is smaller than is required
 *      - P4DEV_REG_INDEX_ERROR - given index is out of range
 *      - P4DEV_NO_REG - no register structure was passed
 */
API uint32_t p4dev_register_read(const p4dev_t* dev, p4_register_t* reg, uint8_t* read_data, const uint32_t length, uint32_t reg_ind);

/*!
 * \brief Write store value in \p data_to_write array to register \p reg on index \p reg_ind
 *
 * \param [in] dev The P4 device structure
 * \param [in] reg Structure with register data
 * \param [in] data_to_write Pointer on input array of bytes to be stored in register
 * \param [in] length Size of \p data_to_write array in bytes
 * \param [in] reg_ind Index of target memory cell in register array.
 *          Register cells are numbered from 0.
 *
 * \warning The device should be disabled before the call of this function
 *
 * \return The function returns following codes
 *      - P4DEV_OK - everything is OK and data are stored in array
 *      - P4DEV_DEVICE_TREE_ERROR - throwed if there was any error during parsing of device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_ERROR - the internal zero vector is too small
 *      - P4DEV_REG_INDEX_ERROR - given index is out of range
 *      - P4DEV_NO_REG - no register structure was passed
 */
API uint32_t p4dev_register_write(const p4dev_t* dev, p4_register_t* reg, uint8_t* data_to_write, const uint32_t length, uint32_t reg_ind);

/*!
 * \brief Get array of \ref p4_register_t structures which hold information about available registers
 *
 * \param [in] dev The P4 device structure
 * \param [out] reg_arr Array of p4_register_t data
 * \param [out] length Pointer on output variable which will be filled with length of array
 *
 * \warning \ref p4dev_registers_free has to be used after the call of this function 
 *
 * \return The function returns following codes
 *      - P4DEV_OK - everything is OK and data are stored in array
 *      - P4DEV_DEVICE_NOT_ATTACHED - error during the parsing of the device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_TABLE_NAME_ERROR - the name of the table isn't in the device tree model
 *      - P4DEV_ERROR - reg_arr pointer is NULL
 */
API uint32_t p4dev_registers_get(const p4dev_t* dev, p4_register_t** reg_arr, uint32_t* length);

/*!
 * \brief Free the array which holds information about registers
 *
 * \param [in] reg_arr Array to deallocation
 * \param [in] reg_arr_len Length of the allocated array 
 */
API void p4dev_registers_free(p4_register_t* reg_arr, const uint32_t reg_arr_len);

/*!
 * \brief Initialize values of all registers in the register array
 *
 * \param [in] dev P4 device structure
 * \param [in] reg Structure with data of one register array
 *
 * \return The function returns following codes
 *      - P4DEV_OK - register array was initilized
 *      - P4DEV_NO_DEV - no device structure was passed in \p dev parameter
 *      - P4DEV_NO_DEVICE_TREE - device tree pointer on \p dev structure is invalid
 *      - P4DEV_DEVICE_NOT_ATTACHED - error during the parsing of the device tree
 *      - P4DEV_ERROR - error with p4_register_t structure
 *      - P4DEV_NO_REG - no register structure was passed
 */
API uint32_t p4dev_initialize_registers(const p4dev_t* dev, p4_register_t* reg);


/*!
 * \brief Initialize all registers in the table
 *
 * \param [in] dev P4 device structure 
 * \param [in] table_name Name of the table to initialize
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - everything is OK and data are stored in array
 *      - P4DEV_DEVICE_NOT_ATTACHED - error during the parsing of the device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_TABLE_NAME_ERROR - the name of the table isn't in the device tree model
 *      - P4DEV_ERROR - general error (like bad parameter, etc.)
 */
API uint32_t p4dev_initialize_table_registers(const p4dev_t* dev, const char* table_name);

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* _P4DEV_REG_H_ */
