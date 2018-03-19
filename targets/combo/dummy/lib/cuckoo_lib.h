/*!
 * \file cuckoo_lib.h
 * \brief Part of internal Hanic NG filter control implementation which 
 * was modified for the need of libp4dev.
 *
 * \author Lukas Kekely <kekely@cesnet.cz>
 * \author Pavel Benacek <benacek@cesnet.cz>
 * \date 2015
 */
 /*
 * Copyright (C) 2015,2018 CESNET
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
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "../engines/standard.h"

#ifndef _LIBEROUTER_FILTERNG_CORE_CUCKOO_H_
#define _LIBEROUTER_FILTERNG_CORE_CUCKOO_H_

/*!
 * \brief Perform a permutation on a key which is represented as an array
 * of bytes.
 *
 * \param [in] d Pointer on array of bytes to shift. Data are represented in little-endian order.
 * \param [in] p Shift length
 * \param [in] len Length of the key in bits 
 *
 * \warning The allocated array should be freed when it is not needed
 *
 * \return Function returns allocated array of bytes which should be freed
 * in user application. The function may return NULL in the case of any error.
 */
static inline uint32_t* __cuckoo_permutation(uint32_t d[], int p, int len) {
    /* Helping variables */
    uint32_t* dst = NULL;    // Destination array
    uint32_t  dst_i;         // Destination index 
    uint32_t  rem;           // Reminder from the previous shift
    uint32_t  data;          // Data to process
    uint32_t  first;         // First index
    uint32_t  last;          // Last index 
    uint32_t  rem_len;       // Length of the reminder
    uint32_t  mask;          // Helping mask for data preparation in the last step

    const uint32_t blocks = ALLOC_SIZE(len);

    /* Prepare destination array */
    dst = (uint32_t*) malloc(blocks*sizeof(uint32_t));
    if(!dst) {
        return NULL;
    }

    /* Shifting ... */
    rem = 0;
    for(uint32_t i = 0; i < blocks; i++) {
        /* Compute destination index of the record and, prepare data (shift + rem)
         * and identify the new reminder */
        dst_i = (i + p/32) % blocks; 
        data = d[i];
        
        if(p%32 != 0) {
            /* Shift differs from 0 --> we need to prepare data for the next iteration */
            dst[dst_i] = (data << (p%32)) | rem;
            rem   = data >> (32 - p%32);
        } else {
            /* No shift is needed --> just copy the byte from one to another position */
            dst[dst_i] = data;
        }
    }

    /* Last iteration ... appned the data from the last index to the data in the first index */
    first   = (p/32) % blocks;
    last    = (p/8 + (blocks-1)) % blocks;
    rem_len = len - (blocks-1)*32;

    /* Prepare the mask and perform the last iteration. The *data* variable contains 
     * data for the last step. The *last* index has to be masked with a mask (in the case when
     * the whole byte is not used). The *first* index is then filled with remaining data. */
    mask = (rem_len != 32) ? ~(0xffffffff << rem_len) : 0xffffffff;
    data = (d[blocks-1]  & mask) >> (rem_len - p%32);
    dst[last]  = dst[last] & mask;
    dst[first] = dst[first] | data;

    /* We are done :-) */
    return dst;
}

static const uint16_t crc16_tab[] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,  // 00
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,  // 08
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,  // 10
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,  // 18
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,  // 20
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,  // 28
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,  // 30
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,  // 38
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,  // 40
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,  // 48
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,  // 50
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,  // 58
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,  // 60
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,  // 68
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,  // 70
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,  // 78
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,  // 80
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,  // 88
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,  // 90
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,  // 98
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,  // A0
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,  // A8
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,  // B0
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,  // B8
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,  // C0
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,  // C8
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,  // D0
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,  // D8
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,  // E0
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,  // E8
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,  // F0
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78   // F8
};//     0       1       2       3       4       5       6       7

/*!
 * \brief Compute the CRC32 checksum in a byte oriented manner
 *
 * \param [in] key Byte representation of the key.Data are represented in little-endian order. 
 * \param [in] len Length of the key in bits 
 *
 * \return Computed CRC16 value.
 */
static inline uint16_t __cuckoo_crc(uint32_t* key, unsigned len) {
    uint16_t crc; 
    uint32_t data;
    uint32_t index;
    uint32_t iteration;

    /* Escape from the function when the length is zero */
    if(!len) return 0x0;

    data  = key[0];
    crc   = 0;
    index = 0;
    iteration = 0;
    while (len > 8) {
        crc  = crc16_tab[(crc ^ data) & 0xFF] ^ (crc >> 8);
        /* Prepare data for the next iteration */
        len -= 8;
        if(iteration < 3) {
            /* We are still processing the 32 bit block */
            iteration++;
            data >>= 8;
        } else {
            /* We need new block --> restart the iteration, select new data */
            index++;
            data = key[index];
            iteration = 0;
        } 
    }
    crc = crc16_tab[((crc ^ data) << (8-len)) & 0xFF] ^ (crc >> len);
    return crc;
}


/*!
 *  \brief Basis structure which holds information about the cuckoo configuration
 */
typedef struct {
    unsigned keylen;        /*!< The lenght of used key */
    unsigned tables;        /*!< Tables number of allocated tables */
    unsigned tlogsize;      /*!< Required address space for the table */
    unsigned tmask;         /*!< Pre-computed mask which is used for finalization of the address */
    unsigned items;         /*!< Number of items in the filter */
    uint32_t **keys;        /*!< Array of allocated keys */
    uint8_t *valid;         /*!< Valid bits for occupied positions */
} __cuckoo_table_t;


/*! 
 * \brief Compute the cuckoo hash value for a given key and table 
 *
 * This macro calls a series of functions which computes the address of the used row.
 *
 * \param [in] t Cuckoo table structure
 * \param [in] key Pointer on the key byte representation
 * \param [in] tab Selected table which is used for the computation
 * \param [out] addr Computed address
 *
 * \return The function returns non-zero value in the case of success.
 */
static inline int __cuckoo_hash(__cuckoo_table_t* t, uint32_t* key, unsigned tab, unsigned* addr) {
    uint32_t* perm = __cuckoo_permutation(key,tab,t->keylen);
    if(!perm) return 0;

    *addr = ((tab) << (t)->tlogsize) | (__cuckoo_crc(perm, (t)->keylen) & (t)->tmask);
    free(perm);
    return 1; 
}


/*!
 * \brief Initilization of the table 
 *
 * \warning Init shoudl be called before the \ref __cuckoo_table_alloc is called!
 */
static inline void __cuckoo_table_init(__cuckoo_table_t *t) {
    /* Setup default values */
    t->keys  = NULL;
    t->valid = NULL;
}

/*!
 * \brief Free all allocated structures by the cuckoo engine
 *
 * \param [in] t Pointer on the table cuckoo table structure
 */
static inline void __cuckoo_table_free(__cuckoo_table_t *t) {
    /* Sanity check */
    if(!t) return;

    /* Iterate over items and free allocated pointers */
    uint64_t records = ((sizeof(uint32_t*) * t->tables) << t->tlogsize)/sizeof(uint32_t*);
    for(uint64_t i = 0; i < records; i++) {
        if(!t->keys[i]) continue;

        free(t->keys[i]);
        t->keys[i] = NULL;
    }

    /* Free remaining parts */
    if(t->keys)
        free(t->keys);
    if(t->valid)
        free(t->valid);
    __cuckoo_table_init(t);
}

/*!
 * \brief Compare two keys if the byte copy match.
 *
 * \param [in] key1 Pointer on the first key.
 * \param [in] key2 Pointer on the second key.
 * \param [in] len Length of the key in bits
 *
 * \return The function returns true iff both keys are same.
 */
static inline bool __cuckoo_cmp_keys(const uint32_t* key1, const uint32_t* key2, const uint32_t len) {
    uint32_t blocks = ALLOC_SIZE(len);
    uint32_t mask;
    uint32_t tmp1,tmp2;

    /* Check if we have something to compare */
    if(!key1 || !key2) return false;

    for(uint32_t i = 0; i < blocks; i++) {
        /* Check if we are working with the last block ... if yes, apply the mask 
         * and compare keys based on the bit length. */
        if(i == blocks-1) {
            mask = (len%32 != 0) ? ~(0xffffffff << (len%32)) : 0xffffffff;
            tmp1 = key1[i] & mask;
            tmp2 = key2[i] & mask;

            if(tmp1 != tmp2) return false;
          
            /* Escape from the cycle */ 
            break;
        }

        /* Other iterations are using the full key */
        if(key1[i] != key2[i]) {
            return false;
        }
    }

    return true;
} 

/*!
 * \brief Copy the key to new structure.
 *
 * The function allocates new memory block which is then filled
 * with data from key array
 *
 * \warning User should deallocate the allocated memory on his own.
 *
 * \param [in] src Source pointer where the key starts
 * \param [in] len Length of the key in bits
 *
 * \return The function returns NULL when the copying was not successfull.
 */
static inline uint32_t*  __cp_cuckoo_key(const uint32_t* src, const uint32_t len) {  
    /* Allocate new array */
    const uint32_t blocks = ALLOC_SIZE(len);
    uint32_t* dst = (uint32_t*) malloc(sizeof(uint32_t)*blocks);
    if(!dst) return NULL;

    /* Copy data */
    memcpy((void*) dst, (void*) src, sizeof(uint32_t)*blocks); 
    return dst;
}

/*!
 * \brief Allocate the table structure
 *
 * \param [in] t Cuckoo table structure
 * \param [in] tables Number of used tables
 * \param [in] tlogsize Required address space of the block (typically, we pass the binary logarithm from the number of 
 *                      supported records
 * \param [in] keylen Length of the key
 *  
 * \return The function returns positive (non-zero) number iff everything was OK.
 */ 
static inline int __cuckoo_table_alloc(__cuckoo_table_t *t, unsigned tables, unsigned tlogsize, unsigned keylen) {
    t->keylen = keylen;
    t->tables = tables;
    t->tlogsize = tlogsize;
    t->items = 0;
    t->tmask = (1 << tlogsize) - 1;
    t->keys = malloc((sizeof(uint32_t*) * tables) << t->tlogsize);
    t->valid = malloc(tables << (t->tlogsize - 3));
    if(!t->keys || !t->valid) {
        __cuckoo_table_free(t);
        return 0;
    }
    memset(t->valid, 0, tables << (t->tlogsize - 3));
    memset(t->keys, 0, (sizeof(uint32_t*) * tables) << t->tlogsize);
    return 1;
}

/*!
 * \brief Extract the validity of the address in a table
 *
 * \param [in] t Cuckoo table structure
 * \param [in] a Address to inspect
 */
#define __cuckoo_table_valid(t,a) ( (t)->valid[(a) >> 3] & (1 << ((a) & 7)) )

/*!
 * \brief Invalidate the given address in the table
 *
 * \param [in] t Cuckoo table structure
 * \param [in] a Address to inspect
 */
#define __cuckoo_table_invalidate(t,a) (t)->valid[(a) >> 3] &= ~(1 << ((a) & 7))

/*!
 * \brief Add the key into the cuckoo table
 *
 * \param [in] t Cuckoo table structure
 * \param [in] key Byte representation of the key
 *
 * \return The function returns non-zero value iff insertion was successfull.
 */
static inline int __cuckoo_table_add(__cuckoo_table_t *t, uint32_t* key) {
    unsigned history[t->tmask+1], victim, addr, j, i;
    uint32_t* tmp;
    uint32_t* key_clone;

    key_clone = __cp_cuckoo_key(key, t->keylen);
    for(j=0; 1; j++) {
        for(i=0; i<t->tables; i++) {
            __cuckoo_hash(t, key, i, &addr);
            if(!__cuckoo_table_valid(t, addr)) {
                t->keys[addr] = key_clone;
                if(!t->keys[addr]) {
                    free(key_clone);
                    return 0;
                }

                t->valid[addr >> 3] |= 1 << (addr & 7);
                t->items++;
                return 1;
            }
        }
        if(j > t->tmask)
            break;
        victim = j % t->tables;
        __cuckoo_hash(t, key, victim, &addr);

        history[j] = addr;
        tmp = t->keys[addr];
        t->keys[addr] = key_clone;
        key = tmp;
    }
    int k;
    for(k = t->tmask; k >= 0; k--) { // undo
        addr = history[k];
        tmp = t->keys[addr];
        t->keys[addr] = key;
        key = tmp;
    }

    free(key_clone);
    return 0;
}

/*!
 * \brief Remove item from the table
 *
 * \param [in] t Cuckoo table structure
 * \param [in] key Byte representation of the key
 *
 * \return The function returns the numbner of removed keys
 */
static inline int __cuckoo_table_remove(__cuckoo_table_t *t, uint32_t* key) {
    int ret = 0;
    unsigned i;
    unsigned addr;

    for(i = 0; i < t->tables; i++) {
        __cuckoo_hash(t, key, i,&addr);

        if(__cuckoo_cmp_keys(t->keys[addr], key, t->keylen) && __cuckoo_table_valid(t, addr)) {
            // Free allocated space 
            free(t->keys[addr]);
            t->keys[addr] = NULL;
            // Setup validity bit and decrement the number of items
            t->valid[addr >> 3] &= ~(1 << (addr & 7));
            ret++;
            t->items--;
        }
    }
    return ret;
}

/*!
 * \brief Search for a given key in current key set
 *
 * \param [in] t Cuckoo table structure
 * \param [in] key Byte representation of the searched key
 * \param [out] addr Pointer on output variable where the assigned address will be stored
 *
 * \return The function returns non-zero value iff the key was found.
 */
static inline int __cuckoo_table_find(__cuckoo_table_t *t, uint32_t* key, unsigned *addr) {
    unsigned i;
    unsigned a;
    for(i = 0; i < t->tables; i++) {
        __cuckoo_hash(t, key, i,&a);
        if(__cuckoo_cmp_keys(t->keys[a], key, t->keylen) && __cuckoo_table_valid(t, a)) {
            *addr = a;
            return 1;
        }
    }
    return 0;
}

#endif

