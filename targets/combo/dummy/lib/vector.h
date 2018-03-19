/*!
 * \file vector.h
 * \brief Vector array implementation.
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

#ifndef _LK_VECTOR_H_
#define _LK_VECTOR_H_

typedef struct bytes_vector *lkBytesVector;

lkBytesVector lkBytesVector_new();
void lkBytesVector_delete(lkBytesVector v);
unsigned lkBytesVector_size(lkBytesVector v);
unsigned lkBytesVector_capacity(lkBytesVector v);
int lkBytesVector_empty(lkBytesVector v);
void lkBytesVector_reserve(lkBytesVector v, unsigned n);
void lkBytesVector_resize(lkBytesVector v, unsigned n, unsigned char val);
void lkBytesVector_push_back(lkBytesVector v, unsigned char d);
void lkBytesVector_push_back_array(lkBytesVector v, unsigned char *d, unsigned size);
void lkBytesVector_push_back_vector(lkBytesVector v, lkBytesVector s);
void lkBytesVector_insert(lkBytesVector v, unsigned p, unsigned char d);
void lkBytesVector_insert_array(lkBytesVector v, unsigned p, unsigned char *d, unsigned size);
void lkBytesVector_insert_vector(lkBytesVector v, unsigned p, lkBytesVector s);
void lkBytesVector_erase(lkBytesVector v, unsigned p);
void lkBytesVector_erase_range(lkBytesVector v, unsigned f, unsigned l);
void lkBytesVector_pop_back(lkBytesVector v);
void lkBytesVector_clear(lkBytesVector v);
void lkBytesVector_clean(lkBytesVector v);
unsigned char lkBytesVector_at(lkBytesVector v, unsigned n);
#define lkBytesVector_front(v) lkBytesVector_at(v, 0)
#define lkBytesVector_back(v) lkBytesVector_at(v, lkBytesVector_size(v)-1)
unsigned char *lkBytesVector_data(lkBytesVector v);



typedef struct pointers_vector *lkPointersVector;

lkPointersVector lkPointersVector_new();
void lkPointersVector_delete(lkPointersVector v);
unsigned lkPointersVector_size(lkPointersVector v);
unsigned lkPointersVector_capacity(lkPointersVector v);
int lkPointersVector_empty(lkPointersVector v);
void lkPointersVector_reserve(lkPointersVector v, unsigned n);
void lkPointersVector_resize(lkPointersVector v, unsigned n);
void lkPointersVector_push_back(lkPointersVector v, void *d);
void lkPointersVector_push_back_array(lkPointersVector v, void **d, unsigned size);
void lkPointersVector_push_back_vector(lkPointersVector v, lkPointersVector s);
void lkPointersVector_insert(lkPointersVector v, unsigned p, void *d);
void lkPointersVector_insert_array(lkPointersVector v, unsigned p, void **d, unsigned size);
void lkPointersVector_insert_vector(lkPointersVector v, unsigned p, lkPointersVector s);
void lkPointersVector_erase(lkPointersVector v, unsigned p);
void lkPointersVector_erase_range(lkPointersVector v, unsigned f, unsigned l);
void lkPointersVector_pop_back(lkPointersVector v);
void lkPointersVector_clear(lkPointersVector v);
void lkPointersVector_clean(lkPointersVector v);
void *lkPointersVector_at(lkPointersVector v, unsigned n);
#define lkPointersVector_front(v) lkPointersVector_at(v, 0)
#define lkPointersVector_back(v) lkPointersVector_at(v, lkPointersVector_size(v)-1)
void **lkPointersVector_data(lkPointersVector v);

#endif
