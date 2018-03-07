/*!
 *
 * \file p4dev_private.h
 *
 * \brief Private structures and declarations of the library 
 * (mapping of search engines to 
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

#include "p4rule.h"
#include <stdint.h>

#ifndef _P4PRIVATE_H_
#define _P4PRIVATE_H_

/*!
 * \brief Structure with elementary callbacks into search engine.
 *
 * Individual elements of the structure are pointers on functions with given 
 * functionality.
 */
typedef struct {

    /*!<
     * \brief Pointer type for a function which inserts the rule set into the attached P4 device. 
     *
     * Pointed function accepts following arguments: P4 device structure, array of inserted rules
     * and number of inserted rules. 
     * The fuction should return codes declared in the \ref P4DEV_RETURN_CODES structure.
     */
    uint32_t (*insert_rules)(const p4dev_t*, const p4rule_t**, const uint32_t);
    
    /*!<
     * \brief Pointer type for a function which initializes the table into the default state.
     *
     * Pointer function accepts following arugments: P4 device structure, name of the table
     * The fuction should return codes declared in the \ref P4DEV_RETURN_CODES structure.
     */
    uint32_t (*initialize_table)(const p4dev_t*, const char*);
    
    /*!<
     * \brief Pointer type for a function which enables the table.
     *
     * Pointed function accepts following arguments: P4 device structure, name of the table.
     * The fuction should return codes declared in the \ref P4DEV_RETURN_CODES structure.
     */
    uint32_t (*enable)(const p4dev_t*,const char*);
    
    /*!<
     * \brief Pointer type for a function which disables the table.
     *
     * Pointed function accepts following arguments: P4 device structure, name of the table.
     * The fuction should return codes declared in the \ref P4DEV_RETURN_CODES structure.
     */
    uint32_t (*disable)(const p4dev_t*, const char*);

    /*!<
     * \brief Pointer type for a fuction which returns the capacity of the table
     *
     * Pointer function accepts following arguments: P4 device, name of the table and pointer which
     * will be filled with available capacity.
     * The function should return codes declared in the \ref P4DEV_RETURN_CODES structure.
     */
    uint32_t (*get_capacity)(const p4dev_t*, const char*, uint32_t*);
    
} p4engine_callbacks_t;

/*!
 * \brief Structure for mapping between engines and its string representation 
 */
typedef struct {
    const char*                 name;       /*!< String representation of the search engine */
    const p4engine_type_t       type;       /*!< Mapped type (defined in the p4rule.h) */
    const p4engine_callbacks_t  callback;   /*!< Structure with callbacks */
} engine_map_t;


/*! \brief Name of the instance */
const static char* compatibility_str = "netcope,p4";

#endif /* _P4PRIVATE_H_ */
