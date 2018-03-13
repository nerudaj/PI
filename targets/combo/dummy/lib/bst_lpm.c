/*!
 * \file bst_lpm.c
 * \brief Implementation of BST LPM filter configuration functions.
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

#include <stdio.h>
#include <stdlib.h>

#include "bst_lpm.h"
#include "vector.h"



// ////////////////////////////////////////////////////////////////////////////
// Prefix
// ////////////////////////////////////////////////////////////////////////////

typedef struct {
    unsigned length;
    lkBytesVector key;
    lkBytesVector data;
} __prefix;

static inline int prefix_init(__prefix *a, unsigned ksize, unsigned dsize) {
    a->length = 0;
    a->key = lkBytesVector_new();
    lkBytesVector_resize(a->key, (((ksize-1)>>5)+1)<<2, 0);
    a->data = lkBytesVector_new();
    lkBytesVector_resize(a->data, (((dsize-1)>>5)+1)<<2, 0);
}

static inline void prefix_free(__prefix *a) {
    lkBytesVector_delete(a->key);
    lkBytesVector_delete(a->data);
}

static inline void prefix_mask(__prefix *a) {
    unsigned char *ak = lkBytesVector_data(a->key);
    unsigned as = lkBytesVector_size(a->key);
    unsigned bytes = a->length>>3, last = 0xff00>>(a->length&7);
    if(bytes < as) {
        ak[bytes] &= last;
        as -= bytes+1;
        if(as)
            memset(ak+bytes+1, 0, as);
    }
}

static inline int prefix_get_last_key(__prefix *p, unsigned char *lkey) {
    if(p->length == 0)
        return 1;
    unsigned char *pk = lkBytesVector_data(p->key);
    unsigned ps = lkBytesVector_size(p->key);
    unsigned bytes = (p->length-1)>>3, last = 0x80>>((p->length-1)&7);
    memcpy(lkey, pk, ps);
    for(int i=bytes; (i>=0) && last; i--) {
        last += lkey[i];
        lkey[i] = last;
        last >>= 8;
    }
    return last;
}

static inline void prefix_display(__prefix *p) {
    printf("0x");
    for(int i=0; i<lkBytesVector_size(p->key); i++)
        printf("%02x", lkBytesVector_at(p->key, i));
    printf("/%d  ->  0x",p->length);
    for(int i=0; i<lkBytesVector_size(p->data); i++)
        printf("%02x", lkBytesVector_at(p->data, i));
}

static inline int prefix_match(__prefix *a, __prefix *b) {
    if(lkBytesVector_size(b->key) != lkBytesVector_size(a->key))
        return 0;
    if(b->length < a->length)
        return 0;
    unsigned bytes = a->length>>3, last = 0xff00>>(a->length&7);
    unsigned char *ak = lkBytesVector_data(a->key), *bk = lkBytesVector_data(b->key);
    for(int i=0; i<bytes; i++)
        if(ak[i] != bk[i])
            return 0;
    if(bytes < lkBytesVector_size(a->key) && ak[bytes] != (bk[bytes]&last))
        return 0;
    return 1;
}

static inline int prefix_cmp(__prefix *a, __prefix *b) {
    int cmp = memcmp(lkBytesVector_data(a->key), lkBytesVector_data(b->key), lkBytesVector_size(a->key));
    if(!cmp)
        return a->length - b->length;
}



// ////////////////////////////////////////////////////////////////////////////
// Prefix Set
// ////////////////////////////////////////////////////////////////////////////

typedef struct _prefix_set_item {
    __prefix pref;
    lkPointersVector childs;
} __prefix_set_item;

typedef struct _prefix_set_root_item {
    __prefix_set_item root;
    unsigned rules;
} __prefix_set_root_item;


bst_lpm_prefix_set_t bst_lpm_prefix_set_new(bst_lpm_component_t *c) {
    bst_lpm_prefix_set_t s = malloc(sizeof(__prefix_set_root_item));
    if(!s)
        return NULL;
    prefix_init(&(s->root.pref), c->key_width, c->data_width);
    s->root.childs = lkPointersVector_new();
    s->rules = 1;
    return s;
}

void prefix_set_delete_item(__prefix_set_item *s) {
    prefix_free(&(s->pref));
    for(unsigned i=0; i<lkPointersVector_size(s->childs); i++) {
        __prefix_set_item *ptr = lkPointersVector_at(s->childs, i);
        prefix_set_delete_item(ptr);
        free(ptr);
    }
    lkPointersVector_delete(s->childs);
}

void bst_lpm_prefix_set_delete(bst_lpm_prefix_set_t s) {
    prefix_set_delete_item(&(s->root));
    free(s);
}

void prefix_set_item_display(__prefix_set_item *s) {
    static int level = 0;
    for(int i=0; i<level; i++)
        printf(" | ");
    prefix_display(&(s->pref));
    printf("\n");
    level++;
    for(unsigned i=0; i<lkPointersVector_size(s->childs); i++)
        prefix_set_item_display(lkPointersVector_at(s->childs,i));
    level--;
}

void bst_lpm_prefix_set_display(bst_lpm_prefix_set_t s) {
    printf("\n------------------------------------------------------------\n");
    printf("-- PREFIX SET (%d prefixes)", s->rules);
    printf("\n------------------------------------------------------------\n");
    prefix_set_item_display(&(s->root));
    printf("\n");
}

void bst_lpm_prefix_set_set_default_data(bst_lpm_prefix_set_t s, unsigned char *data) {
    memcpy(lkBytesVector_data(s->root.pref.data), data, lkBytesVector_size(s->root.pref.data));
}

void bst_lpm_prefix_set_add(bst_lpm_prefix_set_t s, unsigned char *key, unsigned length, unsigned char *data) {
    if(length == 0) {
        memcpy(lkBytesVector_data(s->root.pref.data), data, lkBytesVector_size(s->root.pref.data));
        return;
    }
    // prepare new item
    __prefix_set_item *item = malloc(sizeof(__prefix_set_item));
    item->pref.length = length;
    item->pref.key = lkBytesVector_new();
    lkBytesVector_push_back_array(item->pref.key, key, lkBytesVector_size(s->root.pref.key));
    prefix_mask(&(item->pref));
    item->pref.data = lkBytesVector_new();
    lkBytesVector_push_back_array(item->pref.data, data, lkBytesVector_size(s->root.pref.data));
    item->childs = lkPointersVector_new();
    // perform add
    __prefix_set_item *node = &(s->root);
    while(node) {
        unsigned cs = lkPointersVector_size(node->childs), i, j;
        __prefix_set_item **c = (__prefix_set_item **)lkPointersVector_data(node->childs);
        for(i=0; i<cs; i++) {
            int cmp = prefix_cmp(&(c[i]->pref),&(item->pref));
            if(cmp == 0) { // prefix exists => change data
                lkBytesVector tmp = c[i]->pref.data;
                c[i]->pref.data = item->pref.data;
                item->pref.data = tmp;
                prefix_set_delete_item(item);
                free(item);
                return;
            } else if(cmp > 0) {
                break;
            }
        }
        if(i && prefix_match(&(c[i-1]->pref),&(item->pref))) { // new prefix in child
            node = c[i-1];
        } else { // add new prefix here
            s->rules++;
            lkPointersVector_insert(node->childs, i, item);
            c = (__prefix_set_item **)lkPointersVector_data(node->childs);
            cs = lkPointersVector_size(node->childs);
            for(j=++i; j<cs; j++)
                if(prefix_match(&(item->pref),&(c[j]->pref)))
                    lkPointersVector_push_back(item->childs, c[j]);
                else
                    break;
            lkPointersVector_erase_range(node->childs, i, j-1);
            return;
        }
    }
}

unsigned bst_lpm_prefix_set_size(bst_lpm_prefix_set_t s) {
    return s->rules;
}

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

unsigned char *prefix_set_dump(__prefix_set_item *s, unsigned char *dump, unsigned char *start) {
    unsigned item_bytes = lkBytesVector_size(s->pref.key) + lkBytesVector_size(s->pref.data) + 4;
    unsigned data_bytes = lkBytesVector_size(s->pref.data) + 4;
    for(int i=0; i<lkPointersVector_size(s->childs); i++) {
        __prefix_set_item *c = lkPointersVector_at(s->childs, i);
        if(memcmp(dump-item_bytes, lkBytesVector_data(c->pref.key), lkBytesVector_size(c->pref.key)) == 0)
            dump -= item_bytes; // optimization: if previously dumped key is the same as mine then remove it
        if((dump == start) || (memcmp(dump-data_bytes, lkBytesVector_data(c->pref.data), lkBytesVector_size(c->pref.data))!=0)) {
            // optimization: only if previously dumped data are different than mine then ->
            // add start record
            memcpy(dump, lkBytesVector_data(c->pref.key), lkBytesVector_size(c->pref.key));
            dump += lkBytesVector_size(s->pref.key);
            memcpy(dump, lkBytesVector_data(c->pref.data), lkBytesVector_size(c->pref.data));
            dump += data_bytes;
        }
        dump = prefix_set_dump(c, dump, start);
        if(prefix_get_last_key(&(c->pref), dump) == 0) { // rule end key
            if(memcmp(dump-data_bytes, lkBytesVector_data(s->pref.data), lkBytesVector_size(s->pref.data)) == 0) {
                // optimization: if previously dumped data are the same as my parent's then do nothing
            } else if(memcmp(dump-item_bytes, dump, lkBytesVector_size(c->pref.key)) == 0) {
                // optimization: if previously dumped key is the same as mine then rewrite its data
                memcpy(dump-data_bytes, lkBytesVector_data(s->pref.data), lkBytesVector_size(s->pref.data));
            } else {
                // add end record
                dump += lkBytesVector_size(s->pref.key);
                memcpy(dump, lkBytesVector_data(s->pref.data), lkBytesVector_size(s->pref.data)); // parent data
                dump += data_bytes;
            }
        }
    }
    return dump;
}

unsigned char *bst_lpm_prefix_set_dump(bst_lpm_prefix_set_t s) {
    unsigned item_bytes = lkBytesVector_size(s->root.pref.key) + lkBytesVector_size(s->root.pref.data) + 4;
    unsigned item_words = item_bytes >> 2;
    unsigned items = s->rules<<1, records;
    uint32_t *ptr32;
    unsigned char *data = malloc((items+1)*item_bytes), *ptr8;
    if(!data)
        return NULL;

    memset(data, 0, (items+1)*item_bytes);
    memcpy(data+lkBytesVector_size(s->root.pref.key), lkBytesVector_data(s->root.pref.data), lkBytesVector_size(s->root.pref.data)); // default data
    ptr8 = prefix_set_dump(&(s->root), data+item_bytes, data); // keys and data
    records = (ptr8-data) / item_bytes;
    ptr32=(uint32_t*)(data+item_bytes-4);
    for(unsigned i=0; i<records; i++, ptr32+=item_words) // addresses
        *ptr32 = i | 0x80000000;

    return data;
}

void dump_display(bst_lpm_component_t *c, unsigned char *data) {
    unsigned char *ptr8=data; unsigned ks=(((c->key_width-1)>>5)+1)<<2, ds=(((c->data_width-1)>>5)+1)<<2;
    for(unsigned i=0; 1; i++) {
        printf("key=0x");
        for(unsigned j=0; j<ks; j++)
            printf("%02x",*ptr8++);
        printf(",\tdata=0x");
        for(unsigned j=0; j<ds; j++)
            printf("%02x",*ptr8++);
        printf(",\taddr=%3u%s\n",*((uint32_t*)ptr8)&0x7fffffff,(*((uint32_t*)ptr8)&0x80000000)?" (vld)":"");
        if(!(*((uint32_t*)ptr8)&0x80000000))
            break;
        ptr8+=4;
    }
    fflush(stdout);
}

bst_lpm_prefix_set_t bst_lpm_prefix_set_restore(bst_lpm_component_t *c, unsigned char *data) {
    bst_lpm_prefix_set_t ps = bst_lpm_prefix_set_new(c);
    unsigned item_words = ((c->key_width-1)>>5) + ((c->data_width-1)>>5) + 3;
    unsigned item_bytes = item_words << 2;
    unsigned items = 1<<c->tree_stages;
    unsigned char buffer[item_bytes];
    uint32_t *ptr32;
    int end = 0;
    __prefix p;
    prefix_init(&p, c->key_width, c->data_width);

    ptr32 = (uint32_t*)(data + item_bytes);
    memcpy(lkBytesVector_data(p.data), data+lkBytesVector_size(p.key), lkBytesVector_size(p.data));
    memset(lkBytesVector_data(p.key), 0, lkBytesVector_size(p.key));
    unsigned ks = lkBytesVector_size(p.key);
    while(!end)
        for(p.length=c->key_width; 1; p.length--) {
            int cmp = prefix_get_last_key(&p, buffer);
            if(cmp && !ptr32[item_words-1]) {
                bst_lpm_prefix_set_add(ps, lkBytesVector_data(p.key), p.length, lkBytesVector_data(p.data)); // add globaly last prefix
                end = 1;
                break;
            }
            if(!cmp)
                cmp = (ptr32[item_words-1]) ? memcmp(buffer, ptr32, ks) : -1;
            if(p.length != c->key_width && (lkBytesVector_data(p.key)[p.length>>3] & (0x80>>(p.length&7))))
                cmp = 1;
            if(cmp == 0) {
                bst_lpm_prefix_set_add(ps, lkBytesVector_data(p.key), p.length, lkBytesVector_data(p.data)); // add last prefix of current range (move to another start & data)
                memcpy(lkBytesVector_data(p.data), ptr32+(ks>>2), lkBytesVector_size(p.data));
                memcpy(lkBytesVector_data(p.key), ptr32, ks);
                ptr32 += item_words;
                break;
            } else if(cmp > 0) {
                bst_lpm_prefix_set_add(ps, lkBytesVector_data(p.key), ++p.length, lkBytesVector_data(p.data)); // add subprefix of current range
                prefix_get_last_key(&p, buffer);
                memcpy(lkBytesVector_data(p.key), buffer, lkBytesVector_size(p.key));
                break;
            }
        }

    prefix_free(&p);
    return ps;
}


// ////////////////////////////////////////////////////////////////////////////
// Firmware component access
// ////////////////////////////////////////////////////////////////////////////

unsigned bst_lpm_find(cs_device_t *dev, bst_lpm_component_t *c, int index) {
    cs_component_t *comp = NULL;
    if(cs_component_find(dev, &comp, NULL, "filter_bst_lpm", index) != 0)
        return 0;
    c->version = cs_component_version(comp);
    if(cs_component_space(comp, &(c->space)) != 0)
        return 0;
    unsigned tmp = cs_space_read_4(dev, c->space, 0);
    c->key_width = tmp >> 16;
    c->data_width = (tmp >> 8) & 0xff;
    c->tree_stages = tmp & 0xff;
    if(c->key_width < 1 || c->data_width < 1 || c->tree_stages < 1)
        return 0;
    return c->version;
}

unsigned bst_lpm_capacity(bst_lpm_component_t *c) {
    return 1<<c->tree_stages;
}

int bst_lpm_configure(cs_device_t *dev, bst_lpm_component_t *c, unsigned char *data) {
    unsigned item_words = ((c->key_width-1)>>5) + ((c->data_width-1)>>5) + 3;
    unsigned item_bytes = item_words << 2;
    uint32_t *ptr32;
    unsigned rules = 1;
    unsigned records, addr;

    ptr32 = (uint32_t*)(data) + (item_words<<1) - 1; // check size of the set
    while(*ptr32) {
        ptr32 += item_words;
        rules += 1;
    }
    if(rules > bst_lpm_capacity(c))
        return bst_lpm_capacity(c) - rules;
    records = rules;

    dump_display(c, data);

    // configure firmware filter
    ptr32 = (uint32_t*)data;
    cs_space_write_4(dev, c->space, 0, 0); // disable filtering
        // configure valid data
        for(unsigned i=0; i<records; i++, ptr32+=item_words)
            cs_space_write_multi_4(dev, c->space, 4, item_words, ptr32);
        // invalidate some records
        addr = records - 1;
        for(unsigned i=0; i<c->tree_stages; i++)
            if(addr & (1<<i))
                addr -= 1<<i;
            else
                cs_space_write_4(dev, c->space, item_words<<2, addr | (1<<i));
    cs_space_write_4(dev, c->space, 0, 1); // enable filtering

    return bst_lpm_capacity(c) - rules;
}

unsigned char *bst_lpm_get_configuration(cs_device_t *dev, bst_lpm_component_t *c) {
    unsigned item_words = ((c->key_width-1)>>5) + ((c->data_width-1)>>5) + 3;
    unsigned item_bytes = item_words << 2;
    unsigned items = 1<<c->tree_stages;
    unsigned char *data = malloc((items+1)*item_bytes);
    if(!data)
        return NULL;
    uint32_t *ptr32 = (uint32_t*)data;

    for(unsigned i=0; i<items; i++, ptr32+=item_words) {
        cs_space_write_4(dev, c->space, item_bytes, i | 0x40000000);
        cs_space_read_multi_4(dev, c->space, 4, item_words, ptr32);
        if(i>0 && !(ptr32[item_words-1]&0x80000000))
            break;
    }
    ptr32[item_words-1] = 0; // end guard
    ptr32 = (uint32_t*)data; // implicit /0 prefix restored
    memset(ptr32, 0, (((c->key_width-1)>>5)+1)<<2);
    ptr32[item_words-1] = 0x80000000;

    dump_display(c, data);

    return data;
}

