/*!
 *
 * \file p4dev.h
 *
 * \brief Local header file for P4DEV library. This library is used for development
 * of tools above generated P4 device. 
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

/*!
 * \mainpage P4 Device Library
 *
 * The library is used for the configuration of P4 device. The structure and available
 * tables, parameters and actions are described in the device tree model.
 * This allows us to implement a general software interface which
 * is capable to handle any synthesized P4 device. The structural description
 * is generated for each network device from translated P4 source code.
 *
 * The description of device tree is available at: https://www.devicetree.org/
 * The source code of device tree is available at: https://git.kernel.org/cgit/utils/dtc/dtc.git
 *
 * The library is ready to be used in any C/C++ project.
 *
 * All API functions are situated in \ref p4dev.h file. Notice that only functions with API in
 * the declaration will be exported for public usage. Detailed grouping of the library is 
 * available in the Modules section.
 */


#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "p4dev_types.h" 
#include "p4rule.h"


#ifndef _P4DEV_H_
#define _P4DEV_H_

/******************************************************************************
* API Functions 
*******************************************************************************/

/*!
 * \defgroup apifunc API Functions
 * \brief Declaration of API headers which are capable to controle a P4 device
 * @{
 */

/*! \brief Identificator of the first card in the system */
//static const p4dev_name_t P4DEV_ID0 = CS_PATH_DEV(0);
/*! \brief Identificator of the second card in the system */
//static const p4dev_name_t P4DEV_ID1 = CS_PATH_DEV(1);

/******************************************************************************
* API Functions
*******************************************************************************/

/*!
 * \defgroup apifunc API Functions
 * \brief Declaration of API headers which are capable to control a P4 device
 * @{
 */

/*!
 * \brief Get the right device path based on the supported communication library.
 *
 * \param [out] dst Pointer to the destination array which will be filled with the path.
 * \param [in] len Lenght of the destionation buffer
 * \param [in] id Selected device to open
 *
 * \return The function returns P4DEV_OK iff everything was OK.
 */
API uint32_t p4dev_get_device_path(char* dst, const uint32_t len, const uint32_t id);

/*!
 * \brief Initialize the P4 device - directly with a given device tree structure.
 *
 * This is typically the first function which is being called
 * at the beginning of usage. The user is also requested to call
 * the \ref p4dev_free at the end of the work. The function also modify 
 * values of \p dev structure. The function also calls the initilization
 * of the P4 device.
 *
 * The following example provides the basic usage:
 * \code
 *      // Read the device tree from a file
 *      void* dt = malloc(ALLOC_SIZE);
 *      int fd = open("devTree");
 *      ....
 *
 *      // Allocate the device tree structure
 *      p4_dev_t p4d;
 *      xret = p4_direct_init(dt, &p4d, P4DEV_ID0);
 *      if(xret != P4DEV_OK) {
 *          ...
 *      }
 *
 *      ...
 *
 *      // Free the P4 device
 *      p4dev_free(&p4d);
 * \endcode
 *
 * \param [in] dt Device Tree blob (more information is available at . 
 *              The passed pointer is rememberd and stored in the \ref p4dev_t structure.
 * \param [inout] dev The P4 Device structure. 
 * \param [in] name Name of the device. You can write the full path of the device 
 *                (like /dev/combosix/0) or you can use predefined constants
 *                \ref P4DEV_ID0 / \ref P4DEV_ID1)
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - everything is OK and device is properly allocated
 *      - P4DEV_UNABLE_TO_ATTACH - the device cannot be attached
 *      - P4DEV_UNABLE_TO_MAP_DEVICE_SPACE - unable to map the address space 
 *      - P4DEV_DEVICE_TREE_NOT_VALID - passed tree is not valid
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_NO_DEV - no \ref p4dev_t structure was passed
 */
API uint32_t p4dev_direct_init(const void* dt, p4dev_t* dev, const p4dev_name_t name);

/*!
 * \brief Initialize the P4 device. The device tree is read directly
 * from the P4 device.
 *
 * \param [inout] dev The P4 device structure
 * \param [in] name Name of the device. You can write the full path of the device 
 *                (like /dev/combosix/0) or you can use predefined constants
 *                \ref P4DEV_ID0 / \ref P4DEV_ID1)
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - everything is OK and device is properly allocated
 *      - P4DEV_UNABLE_TO_ATTACH - the device cannot be attached
 *      - P4DEV_UNABLE_TO_MAP_DEVICE_SPACE - unable to map the address space 
 *      - P4DEV_DEVICE_TREE_NOT_VALID - passed tree is not valid
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_NO_DEV - no \ref p4dev_t structure was passed
 *      - P4DEV_DEVICE_TREE_READING_ERROR - reading of the device tree has failed.
 *      - P4DEV_NO_CALLBACK - appropriate search engine functions are not implemented. 
 */
API uint32_t p4dev_init(p4dev_t* dev, const p4dev_name_t name);

/*!
 * \brief De-initialize the P4 device.
 *
 * The function deinitializes allocated resources and the attached 
 * device is being detached. 
 * It also cleans the value (i.e., it sets the NULL value) 
 * of pointers in \p dev structure.
 *
 * \param [in] dev The P4 device structure
 */
API void p4dev_free(p4dev_t* dev);

/*!
 * \brief Insert the rule set into attached P4 device.
 *
 * \param [in] dev P4 device structure.
 * \param [in] p4rules Array with rules to insert. The rule should be created with 
 *                     with function function which are specificaly created for this purpose.
 *                     For example, if you want to create an TCAM key, you should call the 
 *                     \ref tcam_p4key_create to create a key and \ref tcam_p4key_free to free it.
 *                     Positions with NULL pointer will not be used for the insertion to the table
 *                     (i.e., they will be skipped).
 *                      
 * \param [in] rule_count Number of inserted rules.
 *
 * \warning All rules in the \p p4rules parameter will be uploaded to one table. This property
 * is extensively checked by the library.
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
 *      - P4DEV_ERROR - general parameter error (NULL p4rules array, etc)
 */
API uint32_t p4dev_insert_rules(const p4dev_t* dev, const p4rule_t** p4rules, uint32_t rule_count);

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
API uint32_t p4dev_initialize_table(const p4dev_t* dev, const char* name);

/*!
 * \brief Initialize the whole device.
 *
 * This function initialize the state of the P4 device. The typical operation
 * is the initilization of all tables and so on. 
 *
 * \return The function returns following return codes.
 *      - P4DEV_OK - rule is inserted
 *      - P4DEV_NO_DEV - no device structure was passed in \p dev parameter
 *      - P4DEV_NO_DEVICE_TREE - device tree pointer on \p dev structure is invalid
 *      - P4DEV_DEVICE_TREE_ERROR - error during the parsing of the device tree
 */
API uint32_t p4dev_reset_device(const p4dev_t* dev);

/*!
 * \brief Enable the P4 pipeline
 *
 * The function enables the packet processing in P4 pipeline.
 *
 * \param [in] dev P4 device structure
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - all tables are enabled
 *      - P4DEV_DEVICE_TREE_ERROR - error during the processing of device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_NO_CALLBACK - callback function wasn't find
 */
API uint32_t p4dev_enable(const p4dev_t* dev);

/*!
 * \brief Disable the P4 pipeline
 *
 * The function disables the packet processing in P4 pipeline.
 *
 * \param [in] dev P4 device structure
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - all tables are enabled
 *      - P4DEV_DEVICE_TREE_ERROR - error during the processing of the device tree
 *      - P4DEV_NO_DEV - no device was passed
 *      - P4DEV_NO_DEVICE_TREE - no device tree was passed
 *      - P4DEV_DEVICE_NOT_ATTACHED - device is not attached
 *      - P4DEV_NO_CALLBACK - callback function wasn't find
 */
API uint32_t p4dev_disable(const p4dev_t* dev);

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
API uint32_t p4dev_get_table_capacity(const p4dev_t* dev, const char* name, uint32_t* capacity);

/*!
 * \brief Get names of all P4 device tables
 *
 * \param [in] dev The P4 device structure
 * \param [out] names Returned P4 device table names
 * \param [out] count Returned P4 device table count
 *
 * \warning The user is responsible to call the \ref p4dev_free_table_names function to 
 * free all allocated resources.
 *
 * \return The function returns following codes
 *      - P4DEV_OK - everything is OK, the names are returned through \p names argument
 *      - P4DEV_DEVICE_TREE_ERROR - throwed if there was any error during parsing of device tree
 *      - P4DEV_ERROR - a parameter was NULL or malloc() error occured
 *      - P4DEV_ALLOCATE_ERROR - function wasn't able to allocate system memory
 */
API uint32_t p4dev_get_table_names(const p4dev_t* dev, char*** names, uint32_t* count);

/*!
 * \brief Free the allocated structure of table names
 * 
 * The function returns all allocated resources and sets the NULL
 * values of input variables.
 *
 * \param [out] names Returned P4 device table names
 * \param [out] count Returned P4 device table count
 *
 */
API void p4dev_free_table_names(char*** names, uint32_t* count);

/*!
 * \brief Get the type of instantiated engine by the table name.
 *
 * \param [in] dev The P4 device structure
 * \param [in] name Name of the table to process
 *
 * \return The function returns type of used search engine.
 */
API p4engine_type_t  p4dev_get_table_type(const p4dev_t* dev, const char* name);

/*! @}*/

#endif /*_P4DEV_H_*/

#ifdef __cplusplus
}
#endif

