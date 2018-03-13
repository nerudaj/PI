/*!
 * \file p4rule.h
 * \brief Header file for the P4 record module, providing helper functions for
 * P4 rule ADT
 *
 * \author Jan Remes <remes@netcope.com>
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

#include <stdint.h>
#include <stdbool.h>
#include "p4dev_types.h"

#ifndef _P4RULE_H_
#define _P4RULE_H_

/*!
 * \defgroup p4rule P4 Rule structues & Functions
 * \brief Declaration of general helping functions for basic work with rules (creation, setting of rules, etc.)
 * @{
 */

/*!
 * \brief Enumeration of known engines 
 */
typedef enum {
    P4ENGINE_TCAM,    /*!< TCAM engine */
    P4ENGINE_LPM,     /*!< LPM engine */
    P4ENGINE_CUCKOO,  /*!< Cuckoo engine */
    P4ENGINE_UNKNOWN  /*!< Unknown engine */
} p4engine_type_t;

/*!
 * \brief This structure holds additional information for the
 * value stored in the \ref p4key_elem_t strucutre. It typically represent
 * the LPM or it points on the mask
 */
typedef union value_option {
    uint8_t*  mask;       /*!< Corresponding bit mask  
                           *     - 0 -> don't care,
                           *     - 1 -> match the corresponding value) */
    uint32_t  prefix_len; /*!< Length of the prefix */
} value_option_t;

/*!
 * \brief One element of P4 key
 *
 * The element contains one part of search key.
 * The structure consists of following subcomponents:
 *  - key element name (taken from P4 program)
 *  - value - value of the record
 *  - next - next \ref p4key_elem_t part
 */
typedef struct p4key_elem {
    const char*         name;           /*!< Name of the key part */
    uint8_t*            value;          /*!< Binary representation with value of the key */
    value_option_t      opt;            /*!< Optional parameters which are required with a value (mask, prefix length, etc.) */
    uint32_t            val_size;       /*!< Size of the value array in bytes */
    struct p4key_elem*  next;           /*!< Next element of the key. The last element */
} p4key_elem_t;

/*!
 * \brief P4 action parameter
 *
 * The element contains one parameter of selected action.
 * The structure consists of following subcomponents:
 *  - name - name of the parameter
 *  - value - value of the parameter
 *  - next - next \ref p4param_t part
 */
typedef struct p4param {
    const char*         param_name; /*!< Name of the parameter */
    uint8_t*            value;      /*!< Binary representation of value */
    uint32_t            val_size;   /*!< Size of the value array in bytes */
    struct p4param*     next;       /*!< Next available parameter */
} p4param_t;

/*!
 * \brief Structure with inserted rule.
 *
 * The structure describes following parts:
 *  - name - name of the target table
 *  - address - address of inserted element to the search table
 *  - key parts - parts of inserted key
 *  - action - selected action
 *  - params - value of parameters
 *
 *  The key and action can be NULL - no search key or action will be constructed.
 *  The key is typically NULL when the P4 program doesn't contain any \p reads section.
 *
 *  Not needed elements in linked list are not serialized while the error is thrown if
 *  required element is not find.
 */
typedef struct p4rule {
    const char*         table_name;       /*!< Name of the target table (C String) */
    p4engine_type_t     engine;           /*!< Type of  used engine. This parameter  typically infers the type of constructed \p key */
    p4key_elem_t*       key;              /*!< Key to insert - linked list of \ref p4key_elem_t structures */
    const char*         action;           /*!< Name of the action (C String) */
    p4param_t*          param;            /*!< Parameters of selected action */
    bool                def;              /*!< The rule is marked as default */
    void*               private_param;    /*!< Pointer on private pamarameter. The private parameter holds the 
                                               additional parameters or information for the algorithm. Allocated memory
                                               is freed in the  \ref p4rule_free function. Notice that this pointer is considered
                                               to be set by the code of given search engine. */
} p4rule_t;


/*!
 * \brief Create a general P4 rule structure.
 *
 * \param [in] table Name of the table where the rule will be inserted 
 * \param [in] engine Used search engine.
 *
 * The created rule is also marked as not default.
 * 
 * \return The function returns pointer on new created structure. NULL is returned
 * in the case of any error.
 */
API p4rule_t * p4rule_create(const char * table, const p4engine_type_t engine);

/*!
 * \brief Mark the rule as default.
 *
 * \param [in] rule Rule to mark
 */
API void p4rule_mark_default(p4rule_t* rule);

/*!
 * \brief Add a key element to the rule structure.
 *
 * \param [in] rule Rule structure which will be extended with the key element structure.
 * \param [in] key Key structure to add.
 *
 * \return Function returns non zero value in the case of a fail.
 */ 
API uint32_t p4rule_add_key_element(p4rule_t * rule, p4key_elem_t * key);

/*!
 * \brief Add the action structure into the the rule.
 *
 * \param [in] rule Rule structure to extend.
 * \param [in] action Action structure to add.
 *
 * \return Function returns non zero value in the case of a fail.
 */
API uint32_t p4rule_add_action(p4rule_t * rule, const char * action);

/*!
 * \brief Add a function parameter into the rule structure.
 *
 * \param [in] rule Rule structure to extend.
 * \param [in] param Parameter structure to add.
 *
 * \return Function returns the non zero value in the case of a fail.
 */
API uint32_t p4rule_add_param(p4rule_t * rule, p4param_t * param);

/*!
 * \brief Free function for a created rule structure.
 *
 * The function destroys the alloacted rule. It also calls the 
 * free function for a key based on the used key allocater function.
 * For example it calls the tcam_p4key_free function iff the tcam_p4key_create function
 * was called.
 *
 * \param [in] rule Rule to free.
 */
API void p4rule_free(p4rule_t * rule);

/*!
 * \brief Create a parameter structure.
 *
 * \param [in] name Name of the created parameter.
 * \param [in] size Size of the created structure in bytes.
 * \param [in] value Value of the parameter (array of bytes).
 *
 * \return Function returns pointer on new structure. It may return NULL in the case of any error.
 */
API p4param_t * p4param_create(const char * name, uint32_t size, uint8_t * value);

/*!
 * \brief Free the allocated parameter structure
 *
 * \param [in] param Paramater to free.
 */
API void p4param_free(p4param_t * param);

/*!
 * \brief Create a P4 TCAM rule
 *  
 *  The function allocates new key element for the TCAM engine.
 *
 * \param [in] name Name of the created key.
 * \param [in] size Size of created key in bytes.
 * \param [in] value Value to set (array of bytes).
 * \param [in] mask Mask to set (array of bytes).
 *
 * \return Returns pointer on allocated structure. The function returns NULL in the case of any fail.
 */
API p4key_elem_t * tcam_p4key_create(const char * name, uint32_t size, uint8_t * value, uint8_t * mask);

/*!
 * \brief Free a passed TCAM key element.
 *
 * \param [in] key Key to free.
 */
API void tcam_p4key_free(p4key_elem_t * key);

/*!
 * \brief Comparator of two TCAM keys.
 *
 * \param [in] key1 First key to compare.
 * \param [in] key2 Second key to compare
 *
 * \return The function returns true iff both keys are identical.
 */
API bool tcam_p4key_cmp(const p4key_elem_t* key1,const p4key_elem_t* key2);

/*!
 * \brief Create a P4 Exact rule
 *  
 *  The function allocates new key element for the Cuckoo engine.
 *
 * \param [in] name Name of the created key.
 * \param [in] size Size of created key in bytes.
 * \param [in] value Value to set (array of bytes).
 *
 * \return Returns pointer on allocated structure. The function returns NULL in the case of any fail.
 */
API p4key_elem_t * cuckoo_p4key_create(const char * name, uint32_t size, uint8_t * value);

/*!
 * \brief Free a passed TCAM key element.
 *
 * \param [in] key Key to free.
 */
API void cuckoo_p4key_free(p4key_elem_t * key);

/*!
 * \brief Comparator of two TCAM keys.
 *
 * \param [in] key1 First key to compare.
 * \param [in] key2 Second key to compare
 *
 * \return The function returns true iff both keys are identical.
 */
API bool cuckoo_p4key_cmp(const p4key_elem_t* key1,const p4key_elem_t* key2);

/*!
 * \brief Create a P4 BST LPM key element
 *  
 *  The function allocates new key element for the BST LPM engine.
 *
 * \param [in] name Name of the created key.
 * \param [in] size Size of created key in bytes.
 * \param [in] value Value to set (array of bytes).
 * \param [in] prefix_len Bit length of the prefix
 *
 * \return Returns pointer on allocated structure. The function returns NULL in the case of any fail.
 */
API p4key_elem_t * bstlpm_p4key_create(const char * name, uint32_t size, uint8_t * value, uint32_t prefix_len);

/*!
 * \brief Free a passed BST LPM key element.
 *
 * \param [in] key Key to free.
 */
API void bstlpm_p4key_free(p4key_elem_t * key);

/*!
 * \brief Comparator of two BST LPM keys.
 *
 * \param [in] key1 First key to compare.
 * \param [in] key2 Second key to compare
 *
 * \return The function returns true iff both keys are identical.
 */
API bool bstlpm_p4key_cmp(const p4key_elem_t* key1,const p4key_elem_t* key2);

/*! @} */

#endif /* _P4RULE_H_ */
