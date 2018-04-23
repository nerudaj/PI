/*!
 *
 * \file p4dev_types.h
 *
 * \brief Header file with standard library data types.
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



#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <combo.h>

#ifndef _P4DEV_TYPES_H_
#define _P4DEV_TYPES_H_

/*!
 * \defgroup apimacros API Macros
 * \brief Helping macros in API
 * @{
 */

/*! \brief Helping macro for exporting of library symbol */
#define API __attribute__ ((visibility ("default")))

/*! @}*/

/*! 
 * \defgroup apistruct API Structures
 * \brief Definition of basic API structures.
 *
 * These strucutres are used for communication with P4 device
 * (e.g., initlization of the device, insertion of rule, and so on).
 *
 * @{
 */

/*!
 * \brief Structure with basic data of attached P4 device
 *
 * Each program is required to call the \ref p4dev_init function.
 */
typedef struct p4dev {
    cs_device_t* cs;          /*!< Combo device */
    cs_space_t*  cs_space;    /*!< Combo space mapping */
    const void*  dt;          /*!< Device tree blob (describes the opened P4 device) */
    uint32_t     dt_p4offset; /*!< Offset of a P4 node in the device tree */
} p4dev_t;


/*! \brief Type for the identification of the device */
typedef char* p4dev_name_t;

/*! @}*/


/*!
 * \defgroup p4dev_retcodes Return Codes
 * \brief Definition of basic codes returned by API functions
 * @{
 */

/*!
 * \brief Enumeration of possible return codes
 */
enum P4DEV_RETURN_CODES {
    /*! \brief Everything is OK */
    P4DEV_OK                            =   0x0,
    /*! \brief Unable to attach the device */
    P4DEV_UNABLE_TO_ATTACH              =   0x1,
    /*! \brief Unkwnown error */
    P4DEV_UNKNOWN_ERR                   =   0x2,
    /*! \brief The function is not implemented */
    P4DEV_NOT_IMPLEMENTED               =   0x3,
    /*! \brief The function is not able to map the address space */
    P4DEV_UNABLE_TO_MAP_DEVICE_SPACE    =   0x4,
    /*! \brief Device tree is not valid */
    P4DEV_DEVICE_TREE_NOT_VALID         =   0x5,
    /*! \brief Device is not attached */
    P4DEV_DEVICE_NOT_ATTACHED           =   0x6,
    /*! \brief The passed name of the key is not in the device tree */
    P4DEV_KEY_NAME_ERROR                =   0x7,
    /*! \brief The passed name of the action is not in the device tree */
    P4DEV_ACTION_NAME_ERROR             =   0x8,
    /*! \brief The passed table name is not in the device tree */
    P4DEV_TABLE_NAME_ERROR              =   0x9,
    /*! \brief The length of byte array is not correct */
    P4DEV_BYTE_ARRAY_LENGTH_ERROR       =   0xA,
    /*! \brief Bad rule address */
    P4DEV_RULE_ADDRESS_ERROR            =   0xB,
    /*! \brief No device tree was configured */
    P4DEV_NO_DEVICE_TREE                =   0xC,
    /*! \brief No \ref p4dev_t structure  was pased */
    P4DEV_NO_DEV                        =   0xD,
    /*! \brief Error during the work with device tree */
    P4DEV_DEVICE_TREE_ERROR             =   0xE,
    /*! \brief General error */
    P4DEV_ERROR                         =   0xF,
    /*! \brief Invalid parameter name */
    P4DEV_PARAMETER_NAME_ERROR          =   0x10,
    /*! \brief Unable to read the Device Tree from the device */ 
    P4DEV_DEVICE_TREE_READING_ERROR     =   0x11,
    /*! \brief Library doesn't have implemented appropriate control functions for the search engine */
    P4DEV_NO_CALLBACK                   =   0x12,
    /*! \brief The function was unable to insert all rules */
    P4DEV_UNABLE_TO_INSERT              =   0x13,
    /*! \brief The function wasn't able to allocate system memory */
    P4DEV_ALLOCATE_ERROR                =   0x14,
	P4DEV_NO_REG						=	0x15,
	P4DEV_SMALL_BUFFER					=	0x16,
	P4DEV_REG_INDEX_ERROR				=	0x17
};

/*! 
 * \brief String representation of possible return codes
 */
static const char* P4DEV_STR_RETURN_CODES[] = {
    [P4DEV_OK]                          = "P4DEV_OK",
    [P4DEV_UNABLE_TO_ATTACH]            = "P4DEV_UNABLE_TO_ATTACH",
    [P4DEV_UNKNOWN_ERR]                 = "P4DEV_UNKNOWN_ERR",
    [P4DEV_NOT_IMPLEMENTED]             = "P4DEV_NOT_IMPLEMENTED",
    [P4DEV_UNABLE_TO_MAP_DEVICE_SPACE]  = "P4DEV_UNABLE_TO_MAP_DEVICE_SPACE",
    [P4DEV_DEVICE_TREE_NOT_VALID]       = "P4DEV_DEVICE_TREE_NOT_VALID",
    [P4DEV_DEVICE_NOT_ATTACHED]         = "P4DEV_DEVICE_NOT_ATTACHED",
    [P4DEV_KEY_NAME_ERROR]              = "P4DEV_KEY_NAME_ERROR",
    [P4DEV_ACTION_NAME_ERROR]           = "P4DEV_ACTION_NAME_ERROR",
    [P4DEV_TABLE_NAME_ERROR]            = "P4DEV_TABLE_NAME_ERROR",
    [P4DEV_BYTE_ARRAY_LENGTH_ERROR]     = "P4DEV_BYTE_ARRAY_LENGTH_ERROR",
    [P4DEV_RULE_ADDRESS_ERROR]          = "P4DEV_RULE_ADDRESS_ERROR",
    [P4DEV_NO_DEVICE_TREE]              = "P4DEV_NO_DEVICE_TREE",
    [P4DEV_NO_DEV]                      = "P4DEV_NO_DEV",
    [P4DEV_DEVICE_TREE_ERROR]           = "P4DEV_DEVICE_TREE_ERROR",
    [P4DEV_ERROR]                       = "P4DEV_ERROR",
    [P4DEV_PARAMETER_NAME_ERROR]        = "P4DEV_PARAMETER_NAME_ERROR",
    [P4DEV_DEVICE_TREE_READING_ERROR]   = "P4DEV_DEVICE_TREE_READING_ERROR",
    [P4DEV_NO_CALLBACK]                 = "P4DEV_NO_CALLBACK",
    [P4DEV_UNABLE_TO_INSERT]            = "P4DEV_UNABLE_TO_INSERT",
    [P4DEV_ALLOCATE_ERROR]              = "P4DEV_ALLOCATE_ERROR",
    [P4DEV_NO_REG]             			= "P4DEV_NO_REG",
    [P4DEV_SMALL_BUFFER]             	= "P4DEV_SMALL_BUFFER",
    [P4DEV_REG_INDEX_ERROR]             = "P4DEV_REG_INDEX_ERROR"
};

/*! 
 * \brief This macro is used for the computation of the total number of return codes
 *
 * The number of return codes in the array is computed as the size of allocated static array divided
 * by the size of one char* pointer
 */
#define P4DEV_RETURN_CODES (sizeof(P4DEV_STR_RETURN_CODES)/sizeof(char*))

/*! 
 * \brief Translate the given error to the string representation.
 *
 * This function translates the error code to the string representation. The error
 * is printed on the standard output.
 *
 * \param[in] err Error code to translate. The list of available error codes 
 *                is in the \ref P4DEV_RETURN_CODES.
 */
inline static void p4dev_err_stderr(const uint32_t err) {
    if(err > P4DEV_RETURN_CODES-1) {
        perror("Cannot map the passed error string");
        return;
    }

    fprintf(stderr,"%s\n", P4DEV_STR_RETURN_CODES[err]);
}

/*!
 * \brief Copy the value of output string to the destination array
 *
 * \param [in] err Translated return code.
 * \param [out] dst Output string array.
 * \param [in] dst_len Length of the \p dst array.
 *
 * \return The function returns following codes:
 *      - P4DEV_OK - the string was copied into the \p dst array.
 *      - P4DEV_ERROR - the passed destination array is NULL, the error code in \p err parameter
 *                      is out of the range or the destination array is shorter than is required.
 */
inline static uint32_t p4dev_err_copy(const uint32_t err, char* dst, uint32_t dst_len) {
    /* Declare variables */
    uint32_t len;

    /* Sanity check of input and copy operation */
    if(!dst || err > P4DEV_RETURN_CODES-1) return P4DEV_ERROR;
    
    len = strnlen(P4DEV_STR_RETURN_CODES[err], 255);
    if((len+1) > dst_len) return P4DEV_ERROR;

    /* So far so good, copy the value into the byte array */
    strncpy(dst, P4DEV_STR_RETURN_CODES[err], len+1);
    return P4DEV_OK;
}

/*! @}*/

#endif /* _P4DEV_TYPES_H_ */
