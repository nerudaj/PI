/*!
 * \file p4dev.c
 *
 * \brief Implementation of P4DEV library.
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

#include "p4dev.h"
#include "p4dev_tree.h"
#include "p4dev_private.h"
#include <libfdt.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

/* If you want to add the new function, include the header file with prototypes of implemented
 * callback functions and fill the callback structure with pointers on the implementation. 
 * To extend the set of callback functions, see the p4dev_private.h file for more details. 
 */
#include "engines/tcam.h"
#include "engines/cuckoo.h"
#include "engines/bstlpm.h"

/*!
 * \brief Helping structure with mapping between the human readable representation
 * of the search engine type and library based P4ENGINE_* enum type.
 */
static const engine_map_t ENGINE_DEV_HANDLER[] = {
    /*!< TCAM handler */
    {.name = "mtcam", 
     .type = P4ENGINE_TCAM, 
     .callback = {
        .insert_rules      = tcam_insert_rules,
        .initialize_table  = tcam_initialize_table,
        .enable            = tcam_enable,
        .disable           = tcam_disable,
        .get_capacity      = dt_get_table_capacity 
      }
    }, /* End of TCAM declaration */

    /*!< Cuckoo handler */
    {.name = "cuckoo", 
     .type = P4ENGINE_CUCKOO, 
     .callback = {
        .insert_rules      = cuckoo_insert_rules,
        .initialize_table  = cuckoo_initialize_table,
        .enable            = cuckoo_enable,
        .disable           = cuckoo_disable,
        .get_capacity      = dt_get_table_capacity 
      }
    }, /* End of LPM declaration */

    /*!< LPM BST handler */
    {.name = "lpmbst", 
     .type = P4ENGINE_LPM, 
     .callback = {
        .insert_rules      = bstlpm_insert_rules,
        .initialize_table  = bstlpm_initialize_table,
        .enable            = bstlpm_enable,
        .disable           = bstlpm_disable,
        .get_capacity      = dt_get_table_capacity 
      }
    } /* End of Cuckoo declaration */
};

const uint32_t TABLE_COUNT = 3;

char TABLE_NAMES[][16] = {
	"ipv4_lpm",
	"forward",
	"tcam"
};

uint32_t TABLE_CAPACITIES[] = {
	3, 4, 8
};

p4engine_type_t TABLE_TYPES[] = {
	P4ENGINE_LPM, P4ENGINE_CUCKOO, P4ENGINE_TCAM
};

/*! 
 * \brief This macro is used for the computation of the total number of supported engines.
 *
 * The number of mapping codes in the array is computed as the size of allocated static array divided
 * by the size of one mapping structure.
 */
#define P4ENGINES_COUNT (sizeof(ENGINE_DEV_HANDLER)/sizeof(engine_map_t))

/*!
 * \brief Get the handler of given table
 *
 * \param [in] dev The P4 device structur
 * \param [in] name Name of the table to process
 *
 * \return The pointer on identified handler function (see \ref engine_map_t structure for more details). 
 * The NULL is returned if no match is found.
 */
static const engine_map_t*  get_table_handler(const p4dev_t* dev,const char* name) {
    /* Declaration of helping variables */
    int32_t         node_offset;
    uint32_t        xret;    
    const void*     engine_prop;
    int32_t         dt_lenp;
    int32_t         cmp_ret;

    /* Perform the basic sanity check */
    if(!dev || !name) {
        return NULL;
    } 

    /* Find the match subnode of the node given table node */
    xret = dt_get_table_node(dev->dt, dev->dt_p4offset, name, &node_offset);
    if(xret != P4DEV_OK) {
        return NULL;
    }

    node_offset = fdt_subnode_offset(dev->dt, node_offset, "match");
    if(node_offset < 0) {
        return NULL;
    }

    /* Get the engine type from the device tree and translate it into library type P4ENGINE_* */
    engine_prop = fdt_getprop(dev->dt,node_offset, "search-engine", &dt_lenp);
    if(!engine_prop) {
        return NULL;
    }

    /* Itarate over the list of elements and find the appropriate mapping */
    for(uint8_t tmp; tmp < P4ENGINES_COUNT; tmp++) {
        cmp_ret = strncmp((const char*) engine_prop, ENGINE_DEV_HANDLER[tmp].name, 128);
        if(cmp_ret == 0) {
            /* Strings are same, mapping is found */
            return &ENGINE_DEV_HANDLER[tmp];
        }
    }

    /* Uuups ... no engine was found. */
    return NULL; 
}

API uint32_t p4dev_direct_init(const void* dt, p4dev_t* dev, const p4dev_name_t name) {
	(void)dt;
	(void)dev;
	(void)name;
    return P4DEV_OK;
}

API uint32_t p4dev_init(p4dev_t* dev, const p4dev_name_t name) {
	(void)name;
	
    #ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Init device\n");
	#endif
	
	int *ptr = (int*)malloc(sizeof(int) * 1000);
	dev->dt = (void*)ptr;
	dev->dt_p4offset = 0;
	
	return P4DEV_OK;
}

API void p4dev_free(p4dev_t* dev) {
	(void)dev;
	
	#ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Free device\n");
	#endif
	
	free((void*)dev->dt);
	dev->dt = NULL;
	dev->dt_p4offset = NULL;
}

API uint32_t p4dev_enable(const p4dev_t* dev) {
	(void)dev;
	
	#ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Enable device\n");
	#endif
	
	return P4DEV_OK;
}

API uint32_t p4dev_disable(const p4dev_t* dev) {
	(void)dev;
	
	#ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Enable device\n");
	#endif
	
	return P4DEV_OK;
}

API uint32_t p4dev_get_table_capacity(const p4dev_t* dev, const char* name, uint32_t* capacity) {
	(void)dev;
	
	#ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Get table capacity\n");
	#endif
	
	for (uint32_t i = 0; i < TABLE_COUNT; i++) {
		if (strcmp(TABLE_NAMES[i], name) == 0) {
			*capacity = TABLE_CAPACITIES[i];
		}
	}
	
	return P4DEV_OK;
}

API uint32_t p4dev_initialize_table(const p4dev_t* dev, const char* name) {
	(void)dev;
	
	#ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Initialize table with name %s\n", name);
	#endif
	
	return P4DEV_OK;
}

API uint32_t p4dev_reset_device(const p4dev_t* dev) {
	(void)dev;
	
	#ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Reset device\n");
	#endif
	
	return P4DEV_OK;
}

API uint32_t p4dev_get_table_names(const p4dev_t* dev, char*** names, uint32_t* count) {
	(void)dev;
	
	#ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Get table names\n");
	#endif
	
	*count = TABLE_COUNT;
	*names = (char**)(malloc(sizeof(char*) * 3));
	for (uint32_t i = 0; i < *count; i++) {
		(*names)[i] = (char*)(malloc(sizeof(char) * 16));
		strcpy((*names)[i], TABLE_NAMES[i]);
	}
	
	return P4DEV_OK;
}

API void p4dev_free_table_names(char*** names, uint32_t* count) {
	#ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Free table names\n");
	#endif
	
	for (uint32_t i = 0; i < *count; i++) {
		free((*names)[i]);
	}
	free(*names);
	*names = NULL;
	*count = 0;
}

API p4engine_type_t  p4dev_get_table_type(const p4dev_t* dev, const char* name) {
	(void)dev;
	
	#ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Get type of table %s\n", name);
	#endif
	
	for (uint32_t i = 0; i < TABLE_COUNT; i++) {
		if (strcmp(TABLE_NAMES[i], name) == 0) {
			return TABLE_TYPES[i];
		}
	}
	
	return P4ENGINE_UNKNOWN;
}

void printEngine(FILE *fh, p4engine_type_t type) {
	switch(type) {
	case P4ENGINE_LPM:
		fprintf(fh, "LPM\n");
		break;
	
	case P4ENGINE_TCAM:
		fprintf(fh, "TCAM\n");
		break;
	
	case P4ENGINE_CUCKOO:
		fprintf(fh, "EXACT\n");
		break;
	
	default:
		fprintf(fh, "Unknown\n");
		break;
	}
}

void printKeyValue(FILE *fh, p4key_elem_t *key, p4engine_type_t type) {
	fprintf(fh, "(");
	
	switch(type) {
	case P4ENGINE_LPM:
		fprintf(fh, "%d", (uint8_t)(key->value[0]));
		for (uint32_t i = 1; i < key->val_size; i++) {
			fprintf(fh, ".%d", (uint8_t)(key->value[i]));
		}
		fprintf(fh, "/%d", key->opt.prefix_len);
		break;
	
	case P4ENGINE_TCAM:
		for (uint32_t i = 0; i < key->val_size; i++) {
			fprintf(fh, "%d", (uint8_t)(key->value[i]));
		}
		fprintf(fh, ":");
		for (uint32_t i = 0; i < key->val_size; i++) {
			fprintf(fh, "%d", (uint8_t)(key->opt.mask[i]));
		}
		break;
	
	case P4ENGINE_CUCKOO:
		for (uint32_t i = 0; i < key->val_size; i++) {
			fprintf(fh, "%d", (uint8_t)(key->value[i]));
		}
		break;
	
	default:
		fprintf(fh, "???\n");
		break;
	}
	
	fprintf(fh, ")");
}

void printKeys(FILE *fh, p4key_elem_t *key, p4engine_type_t type) {
	fprintf(fh, "\tMatchKey: %s", key->name);
	printKeyValue(fh, key, type);
	p4key_elem_t *k = key->next;
	
	while(k != NULL) {
		fprintf(fh, "->%s", k->name);
	printKeyValue(fh, k, type);
		k = k->next;
	}
	
	fprintf(fh, "\n");
}

void printParamValue(FILE *fh, p4param_t *param) {
	fprintf(fh, "(");
	for (uint32_t i = 0; i < param->val_size; i++) {
		fprintf(fh, "%d", (uint8_t)(param->value[i]));
	}
	fprintf(fh, ")");
}

void printParams(FILE *fh, p4param_t *param) {
	fprintf(fh, "\tParams: %s", param->param_name);
	printParamValue(fh, param);
	p4param_t *p = param->next;
	
	while (p != NULL) {
		fprintf(fh, "->%s", p->param_name);
		printParamValue(fh, p);
		p = p->next;
	}
	
	fprintf(fh, "\n");
}

API uint32_t p4dev_insert_rules(const p4dev_t* dev, const p4rule_t** p4rules, uint32_t rule_count) {
	(void)dev;
	
	#ifdef DEBUG_LOGS
	fprintf(stderr, "p4dev: Insert rules. Rule count: %d\n", rule_count);
	#endif
	
	FILE *log = fopen("log.txt", "a");
	if (log == NULL) return P4DEV_ERROR;
	
	fprintf(log, "=== RULES DUMP ===\n");
	
	for (uint32_t i = 0; i < rule_count; i++) {
		const p4rule_t *rule = p4rules[i];
		
		fprintf(log, "Rule for table: %s\n", rule->table_name);
		fprintf(log, "\tEngine: ");
		printEngine(log, rule->engine);
		
		if (rule->def) {
			fprintf(log, "\tDefault rule\n");
		}
		else {
			if (rule->key != NULL) printKeys(log, rule->key, rule->engine);
		}
		
		fprintf(log, "\tAction: %s\n", rule->action);
		if (rule->param != NULL) printParams(log, rule->param);
	}
	
	fprintf(log, "\n");
	fclose(log);
	
	return P4DEV_OK;
}

