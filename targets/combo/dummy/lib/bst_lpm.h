/*!
 * \file bst_lpm.h
 * \brief Header with functions for BST LPM filter configuration.
 * \author Lukas Kekely <kekely@cesnet.cz>
 * \date 2015
 */
 /*
 * Copyright (C) 2015 CESNET
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

#ifndef _LIBEROUTER_BST_LPM_FILTER_H_
#define _LIBEROUTER_BST_LPM_FILTER_H_

#include <combo.h>
#include <commlbr.h>
#include <string.h>

/*!
 * Prefix set.
 */
typedef struct _prefix_set_root_item *bst_lpm_prefix_set_t;

/*!
 * Filter component.
 */
typedef struct {
    cs_space_t *space;    /*!< Pointer to filter component space. */
    unsigned version;     /*!< Filter version. */
    unsigned key_width;   /*!< Filter parameter: width of key in bits. */
    unsigned data_width;  /*!< Filter parameter: width of data in bits. */
    unsigned tree_stages; /*!< Filter parameter: height of the tree. */
} bst_lpm_component_t;

/**
 * \brief Create new prefix set
 * \param c Filter component
 * \return Pointer to prefix set or NULL if out of memory
 */
bst_lpm_prefix_set_t bst_lpm_prefix_set_new(bst_lpm_component_t *c);

/**
 * \brief Destruct prefix set
 * \param s Pointer to prefix set
 */
void bst_lpm_prefix_set_delete(bst_lpm_prefix_set_t s);

/**
 * \brief Display prefix set
 * \param s Pointer to prefix set
 */
void bst_lpm_prefix_set_display(bst_lpm_prefix_set_t s);

/**
 * \brief Set default data (used as result when no rule matches the key)
 * \param s Pointer to prefix set
 * \param data Pointer to data bytes
 */
void bst_lpm_prefix_set_set_default_data(bst_lpm_prefix_set_t s, unsigned char *data);

/**
 * \brief Add record into prefix set
 * \param s Pointer to prefix set
 * \param key Pointer to key bytes
 * \param length Length of prefix
 * \param data Pointer to data bytes
 */
void bst_lpm_prefix_set_add(bst_lpm_prefix_set_t s, unsigned char *key, unsigned length, unsigned char *data);

/**
 * \brief Get number of prefixes inside the set
 * \param s Pointer to prefix set
 * \return Number of prefixes inside the set
 */
unsigned bst_lpm_prefix_set_size(bst_lpm_prefix_set_t s);

/**
 * \brief Dumps prefix set into raw data for firmware filter
 * \param ps Prefix set to dump
 * \return allocated pointer to dumped data
 */
unsigned char *bst_lpm_prefix_set_dump(bst_lpm_prefix_set_t ps);

/**
 * \brief Restores prefix set from raw firmware filter data (resulting prefix set may differ from original due to optimizations)
 * \param c Filter component
 * \return restored prefix set
 */
bst_lpm_prefix_set_t bst_lpm_prefix_set_restore(bst_lpm_component_t *c, unsigned char *data);

/**
 * \brief Find filter component space inside active design
 * \param dev Combo device structure
 * \param c Returned filter component
 * \param index Filter index (from zero)
 * \return zero on failure otherwise a filter component version
 */
unsigned bst_lpm_find(cs_device_t *dev, bst_lpm_component_t *c, int index);

/**
 * \brief Get number of HW records in filter
 * \param c Filter component
 * \return zero on failure otherwise a number of HW filter records
 */
unsigned bst_lpm_capacity(bst_lpm_component_t *c);

/**
 * \brief Push rules into filter
 * \param dev Combo device structure
 * \param c Filter component
 * \param data Data for firmware (dumped prefix set)
 * \return number of free records in HW (negative number means unable to configure)
 */
int bst_lpm_configure(cs_device_t *dev, bst_lpm_component_t *c, unsigned char *data);

/**
 * \brief Read active rules from filter
 * \param dev Combo device structure
 * \param c Filter component
 * \return allocated pointer to dumped data
 */
unsigned char *bst_lpm_get_configuration(cs_device_t *dev, bst_lpm_component_t *c);

#endif
