/*!
 * \file p4rule.c
 * \brief P4 rule ADT operations module
 * \author Jan Remes <remes@netcope.com>
 * \author Pavel Benacek <benacek@cesnet.cz>
 */

/*
* Copyright (C) 2017 Netcope Technologies
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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "p4rule.h"

API p4rule_t * p4rule_create(const char * table, const p4engine_type_t engine) {
    if (!table)
        return NULL;

    p4rule_t * retval = malloc(sizeof(p4rule_t));
    if (!retval)
        return NULL;

    retval->engine = engine;
    retval->action = NULL;
    retval->key    = NULL;
    retval->param  = NULL;
    retval->def    = false;
    retval->private_param = NULL;
    retval->table_name = strndup(table, 128);
    if (!retval->table_name) {
        free(retval);
        return NULL;
    }

    return retval;
}

API void p4rule_mark_default(p4rule_t* rule) {
    if(!rule) return;

    rule->def = true;
}

API uint32_t p4rule_add_key_element(p4rule_t * rule, p4key_elem_t * key) {
    if (!rule || !key) {
        return 1;
    }

    /* Add the key element at the beginning of the list */ 
    key->next = rule->key;
    rule->key = key; 
    return 0;
}

API uint32_t p4rule_add_action(p4rule_t * rule, const char * action) {
    if (!rule)
        return 1;
    if (!action)
        return 1;

    rule->action = strdup(action);

    return 0;
}

API uint32_t p4rule_add_param(p4rule_t * rule, p4param_t * param) {
    if (!rule || !param)
        return 1;

    if (rule->param == NULL) {
        rule->param = param;
        return P4DEV_OK;
    }
    
    p4param_t *iter = rule->param;
    while (iter->next != NULL) iter = iter->next;
    iter->next = param;

    return P4DEV_OK;
}

/*!
 * \brief Free the key based on the type of action
 *
 * \param [in] elem Element to destroy
 * \param [in] ktype Type of used key allocator
 */ 
void free_key_by_type(p4key_elem_t* elem, const p4engine_type_t ktype ) {
    switch(ktype) {
        case P4ENGINE_TCAM:
            tcam_p4key_free(elem);
            break;

        case P4ENGINE_CUCKOO:
            cuckoo_p4key_free(elem);
            break;
        
        case P4ENGINE_LPM:
            bstlpm_p4key_free(elem);
            break;
        /* We dont't want to call any free function for the rest of key allocators */
        default: ;
    }
}

API void p4rule_free(p4rule_t * rule) {
    if (!rule)
        return;

    /* Free all parts of the structure, the key has to be destroyed based
     * on the used key allocator */
    if (rule->action)
        free((char *)rule->action);
    while (rule->key) {
        p4key_elem_t * tmp = rule->key->next;
        free_key_by_type(rule->key,rule->engine);
        rule->key = tmp;
    }
    while (rule->param) {
        p4param_t * tmp = rule->param->next;
        p4param_free(rule->param);
        rule->param = tmp;
    }
    if (rule->table_name)
        free((char *)rule->table_name);
    if (rule->private_param) 
        free(rule->private_param);

    free(rule);
}

API p4key_elem_t * tcam_p4key_create(const char * name, uint32_t size,
                            uint8_t * value, uint8_t * mask) {
    if (!name || !value || !mask)
        return NULL;
    if (size == 0)
        return NULL;

    p4key_elem_t * retval = malloc(sizeof(p4key_elem_t));
    if (!retval)
        return NULL;

    retval->name = strdup(name);
    retval->val_size = size;

    retval->value    = malloc(size);
    retval->opt.mask = malloc(size); 
    if (!retval->value) {
        free(retval->value);
        free((char *)retval->name);
        free(retval);
        return NULL;
    }

    memcpy(retval->value, value, size);
    memcpy(retval->opt.mask, mask, size);
    retval->next = NULL;
    
    return retval;
}

API void tcam_p4key_free(p4key_elem_t * key) {
    if (!key)
        return;

    if (key->name)
        free((char *)key->name);
    if (key->value)
        free(key->value);
    if (key->opt.mask)
        free(key->opt.mask);

    free(key);
}

API p4key_elem_t * cuckoo_p4key_create(const char * name, uint32_t size, uint8_t * value) {
    if (!name || !value)
        return NULL;
    if (size == 0)
        return NULL;

    p4key_elem_t * retval = malloc(sizeof(p4key_elem_t));
    if (!retval)
        return NULL;

    retval->name = strdup(name);
    retval->val_size = size;

    retval->value = malloc(size);
    if (!retval->value) {
        free(retval->value);
        free((char *)retval->name);
        free(retval);
        return NULL;
    }

    memcpy(retval->value, value, size);
    retval->next = NULL;
    
    return retval;
}

API void cuckoo_p4key_free(p4key_elem_t * key) {
    if (!key)
        return;

    if (key->name)
        free((char *)key->name);
    if (key->value)
        free(key->value);

    free(key);
}

API p4key_elem_t * bstlpm_p4key_create(const char * name, uint32_t size, uint8_t * value, uint32_t prefix_len) {
    /* The allocation is is simliar to the cucko allocation. Therefore, perform cuckoo allocation and
     * fill the prefix length into the structure */
    p4key_elem_t* tmp = cuckoo_p4key_create(name,size,value);
    if(!tmp) return NULL;

    tmp->opt.prefix_len = prefix_len;
    return tmp;
}

API void bstlpm_p4key_free(p4key_elem_t * key) {
    /* Freeing is same as freeing of the cuckoo record */
    cuckoo_p4key_free(key);
}

/*!
 * \brief Compare common parts of the \ref p4key_elem_t structure.
 *
 * \warning This function doesn't iterate over the linked list. It 
 * just perfroms comparison of two input key elements (\p key1 and \p key2)
 *
 * \param [in] key1 First key structure to compare.
 * \param [in] key2 Second key structure to compare
 *
 * \warning Function doesn't compare names of processed tables because 
 * both passed structures are considered to have the same name.
 *
 * \return The function returns true iff both common values are same.
 */
bool p4key_cmp_common(const p4key_elem_t* key1,const p4key_elem_t* key2) {
    
    /* Compare the common key stuff */
    if(key1->val_size != key2->val_size) {
        return false;
    }

    for(uint32_t i = 0; i < key1->val_size;i++) {
        if(key1->value[i] != key2->value[i]) {
            return false;
        }
    }

    return true;
}

/*!
 * \brief Check availability of keys
 *
 * The function checks availablity if keys from \p key1 are available in \p key2.
 *
 * \param [in] key1 First key set
 * \param [in] key2 Second key set
 *
 * \return Function returns true iff all keys from \p key1 are available in \p key2 set.
 */
bool p4key_check_key_availabilty(const p4key_elem_t* key1, const p4key_elem_t* key2) {
    /* Helping variables */
    const p4key_elem_t* tmp1;
    const p4key_elem_t* tmp2;

    tmp1 = key1;
    while(tmp1) {
        tmp2 = key2;
        do {
            /* Check if the key is found, escape if yes */
            if(!strncmp(tmp1->name,tmp2->name,255)) break;
            /* Go to the next key */
            tmp2 = tmp2->next;
        } while(tmp2);
       
        /* Uppps .. no such key */ 
        if(!tmp2) return false;

        /* Go to the next key */
        tmp1 = tmp1->next;
    }

   return true; 
}


/*!
 * \brief Find key element from the passed p4_key_elem_t linked list
 *
 * \param [in] key Key root where the search stars
 * \param [in] name Name of the key elemnt.
 *
 * \return The function returns the pointer of the key element. The 
 * NULL value is returned iff the key is not found.
 */
const p4key_elem_t* find_key(const p4key_elem_t* key,const char* name) {
    const p4key_elem_t* tmp = key;
    do {
        /* Escape from the cycle iff the structure is found */
        if(!strncmp(name,tmp->name,255)) return tmp;
        /* Not equal, go to the next element */
        tmp = tmp->next;
    } while(tmp);

    return NULL;
}

/*!
 * \brief Pointer on the function with private comparator function
 */
typedef bool (*private_key_cmp_t)(const p4key_elem_t* key1,const p4key_elem_t* key2);

/*!
 * \brief Key comparator whic iterates over all keys, compares common parts and
 * it also calls the private comparator which take two keys.
 *
 * \param [in] key1 The first key linked list.
 * \param [in] key2 The second key linked list.
 * \param [in] cmpf Private key comparator function. The parameter may be NULL.
 *
 * \return The fucntion returns true iff keys are same.
 */
bool p4key_cmp(const p4key_elem_t* key1,const p4key_elem_t* key2, private_key_cmp_t cmpf) {
    /* Helping variables */
    const p4key_elem_t* tmp1;
    const p4key_elem_t* tmp2;


    /* Basic sanity check of the input */
    if(!key1 || !key2) {
        return false;
    }

    /* Both pointers are valid, compare all members of the structure. First of all
     * compare common parts, after that compare both parameters */
    tmp1 = key1;
    while(tmp1) {
        /* Find the key element in the second key chain. Break if both keys are same*/    
        tmp2 = find_key(key2,tmp1->name);
        if(!tmp2) return false;

        /* Compare common parts. After that, compare the key-related stuff. In this case,
         * the structure in opt attribute stands for the mask. */
        if(!p4key_cmp_common(tmp1,tmp2)) return false;
        
        /* Run the private comparison */
        if(cmpf && !cmpf(tmp1,tmp2)) return false;

        /* Go to the next element of the key1 */
        tmp1 = tmp1->next;
    }

    /* So far so good, check if all keys from key set2 is availabe in key set1 */
    if(!p4key_check_key_availabilty(key2,key1)) return false;

    return true;
}

/*!
 * \brief Private key comparator for TCAM
 *
 * \param [in] key1 First key to compare
 * \param [in] key2 Second key to compare
 *
 * \return The function return true iff key elements match.
 */
static bool tcam_private_cmp(const p4key_elem_t* key1, const p4key_elem_t* key2) {
    for(uint32_t i = 0; i < key1->val_size; i++) {
        if(key1->opt.mask[i] != key2->opt.mask[i]) return false;
    }

    return true;
}

API bool tcam_p4key_cmp(const p4key_elem_t* key1,const p4key_elem_t* key2) {
    return p4key_cmp(key1,key2,tcam_private_cmp);
}

API bool cuckoo_p4key_cmp(const p4key_elem_t* key1,const p4key_elem_t* key2) {
    return p4key_cmp(key1,key2,NULL);
}

/*!
 * \brief Private key comparator for BST LPM
 *
 * \param [in] key1 First key to compare
 * \param [in] key2 Second key to compare
 *
 * \return The function return true iff key elements match.
 */
static bool bstlpm_private_cmp(const p4key_elem_t* key1, const p4key_elem_t* key2) {
    if(key1->opt.prefix_len != key2->opt.prefix_len) return false;

    return true;
}

API bool bstlpm_p4key_cmp(const p4key_elem_t* key1,const p4key_elem_t* key2) {
    return p4key_cmp(key1,key2,bstlpm_private_cmp);
}


API p4param_t* p4param_create(const char * name, unsigned size, uint8_t * value) {
    if (!name)
        return NULL;
    if (size == 0)
        return NULL;
    if (!value)
        return NULL;

    p4param_t * retval = malloc(sizeof(p4param_t));
    if (!retval)
        return NULL;

    retval->param_name = strdup(name);
    if (!retval->param_name) {
        free(retval);
        return NULL;
    }
    retval->val_size = size;
    retval->value = malloc(size);
    if (!retval->value) {
        free((char *)retval->param_name);
        free(retval);
        return NULL;
    }
    memcpy(retval->value, value, size);

    retval->next = NULL;

    return retval;
}

API void p4param_free(p4param_t * param) {
    if (!param)
        return;

    if (param->param_name)
        free((char *)param->param_name);
    if (param->value)
        free(param->value);

    free(param);
}

