/*!
 * \file vector.c
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

#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>




// ////////////////////////////////////////////////////////////////////////////
// Bytes Vector
// ////////////////////////////////////////////////////////////////////////////

struct bytes_vector {
    unsigned char *data;
    unsigned alloc;
    unsigned size;
};

lkBytesVector lkBytesVector_new() {
    lkBytesVector v = malloc(sizeof(struct bytes_vector));
    if(!v) {
        fprintf(stderr, "lkBytesVector error: Out of memory!\n");
        exit(1);
    }
    v->data = NULL;
    v->alloc = v->size = 0;
    return v;
}

void lkBytesVector_delete(lkBytesVector v) {
    free(v->data);
    free(v);
}

unsigned lkBytesVector_size(lkBytesVector v) {
    return v->size;
}

unsigned lkBytesVector_capacity(lkBytesVector v) {
    return v->alloc;
}

int lkBytesVector_empty(lkBytesVector v) {
    return v->size == 0;
}

void lkBytesVector_push_back(lkBytesVector v, unsigned char d) {
    if(++v->size > v->alloc)
        lkBytesVector_reserve(v, (v->alloc)?(v->alloc<<1):1);
    v->data[v->size-1] = d;
}

void lkBytesVector_push_back_array(lkBytesVector v, unsigned char *d, unsigned size) {
    unsigned ns = v->size + size;
    lkBytesVector_reserve(v, ns);
    memcpy(v->data+v->size, d, size);
    v->size = ns;
}

void lkBytesVector_push_back_vector(lkBytesVector v, lkBytesVector s) {
    unsigned ns = v->size + s->size;
    lkBytesVector_reserve(v, ns);
    memcpy(v->data+v->size, s->data, s->size);
    v->size = ns;
}

void lkBytesVector_pop_back(lkBytesVector v) {
    if(v->size)
        v->size--;
}

void lkBytesVector_reserve(lkBytesVector v, unsigned n) {
    if(n <= v->alloc)
        return;
    unsigned char *tmp = realloc(v->data,n);
    if(!tmp) {
        fprintf(stderr, "lkBytesVector error: Out of memory!\n");
        exit(1);
    }
    v->data = tmp;
    v->alloc = n;
}

void lkBytesVector_resize(lkBytesVector v, unsigned n, unsigned char val) {
    if(n <= v->size)
        v->size = n;
    else {
        lkBytesVector_reserve(v, n);
        memset(v->data+v->size, val, n-v->size);
        v->size = n;
    }
}

void lkBytesVector_clear(lkBytesVector v) {
    v->size = 0;
}

void lkBytesVector_clean(lkBytesVector v) {
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->alloc = 0;
}

unsigned char lkBytesVector_at(lkBytesVector v, unsigned n) {
    if(n >= v->size) {
        fprintf(stderr, "lkBytesVector error: Access out of range!\n");
        exit(1);
    }
    return v->data[n];
}

unsigned char *lkBytesVector_data(lkBytesVector v) {
    return v->data;
}

void lkBytesVector_insert(lkBytesVector v, unsigned p, unsigned char d) {
    if(p >= v->size) {
        lkBytesVector_push_back(v, d);
        return;
    }
    if(++v->size > v->alloc)
        lkBytesVector_reserve(v, (v->alloc)?(v->alloc<<1):1);
    memmove(v->data+p+1, v->data+p, v->size-p-1);
    v->data[p] = d;
}

void lkBytesVector_insert_array(lkBytesVector v, unsigned p, unsigned char *d, unsigned size) {
    if(p >= v->size) {
        lkBytesVector_push_back_array(v, d, size);
        return;
    }
    lkBytesVector_reserve(v, v->size+size);
    memmove(v->data+p+size, v->data+p, v->size-p);
    memcpy(v->data+p, d, size);
    v->size += size;
}

void lkBytesVector_insert_vector(lkBytesVector v, unsigned p, lkBytesVector s) {
    if(p >= v->size) {
        lkBytesVector_push_back_vector(v, s);
        return;
    }
    lkBytesVector_reserve(v, v->size+s->size);
    memmove(v->data+p+s->size, v->data+p, v->size-p);
    memcpy(v->data+p, s->data, s->size);
    v->size += s->size;
}

void lkBytesVector_erase(lkBytesVector v, unsigned p) {
    if(p >= v->size)
        return;
    memmove(v->data+p, v->data+p+1, v->size-p-1);
    v->size--;
}

void lkBytesVector_erase_range(lkBytesVector v, unsigned f, unsigned l) {
    if(f >= v->size || l < f)
        return;
    if(l > v->size)
        l = v->size;
    if(l < v->size) {
        memmove(v->data+f, v->data+l+1, v->size-l);
        v->size -= l-f+1;
    } else
        v->size = f;
}





// ////////////////////////////////////////////////////////////////////////////
// Pointers Vector
// ////////////////////////////////////////////////////////////////////////////

struct pointers_vector{
    void **data;
    unsigned alloc;
    unsigned size;
};

lkPointersVector lkPointersVector_new() {
    lkPointersVector v = malloc(sizeof(struct pointers_vector));
    if(!v){
        fprintf(stderr, "lkPointersVector error: Out of memory!\n");
        exit(1);
    }
    v->data = NULL;
    v->alloc = v->size = 0;
    return v;
}

void lkPointersVector_delete(lkPointersVector v) {
    free(v->data);
    free(v);
}

unsigned lkPointersVector_size(lkPointersVector v) {
    return v->size;
}

unsigned lkPointersVector_capacity(lkPointersVector v) {
    return v->alloc;
}

int lkPointersVector_empty(lkPointersVector v) {
    return v->size == 0;
}

void lkPointersVector_push_back(lkPointersVector v, void *d) {
    if(++v->size > v->alloc)
        lkPointersVector_reserve(v, (v->alloc)?(v->alloc<<1):1);
    v->data[v->size-1] = d;
}

void lkPointersVector_push_back_array(lkPointersVector v, void **d, unsigned size) {
    unsigned ns = v->size + size;
    lkPointersVector_reserve(v, ns);
    memcpy(v->data+v->size, d, sizeof(void*)*size);
    v->size = ns;
}

void lkPointersVector_push_back_vector(lkPointersVector v, lkPointersVector s) {
    unsigned ns = v->size + s->size;
    lkPointersVector_reserve(v, ns);
    memcpy(v->data+v->size, s->data, sizeof(void*)*s->size);
    v->size = ns;
}

void lkPointersVector_pop_back(lkPointersVector v) {
    if(v->size)
        v->size--;
}

void lkPointersVector_reserve(lkPointersVector v, unsigned n) {
    if(n <= v->alloc)
        return;
    void **tmp = realloc(v->data, sizeof(void*)*n);
    if(!tmp) {
        fprintf(stderr, "lkPointersVector error: Out of memory!\n");
        exit(1);
    }
    v->data = tmp;
    v->alloc = n;
}

void lkPointersVector_resize(lkPointersVector v, unsigned n) {
    if(n <= v->size)
        v->size = n;
    else {
        lkPointersVector_reserve(v, n);
        memset(v->data+v->size, 0, sizeof(void*)*(n-v->size));
        v->size = n;
    }
}

void lkPointersVector_clear(lkPointersVector v) {
    v->size = 0;
}

void lkPointersVector_clean(lkPointersVector v) {
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->alloc = 0;
}

void *lkPointersVector_at(lkPointersVector v, unsigned n) {
    if(n >= v->size) {
        fprintf(stderr, "lkPointersVector error: Access out of range!\n");
        exit(1);
    }
    return v->data[n];
}

void **lkPointersVector_data(lkPointersVector v) {
    return v->data;
}

void lkPointersVector_insert(lkPointersVector v, unsigned p, void *d) {
    if(p >= v->size) {
        lkPointersVector_push_back(v, d);
        return;
    }
    if(++v->size > v->alloc)
        lkPointersVector_reserve(v, (v->alloc)?(v->alloc<<1):1);
    memmove(v->data+p+1, v->data+p, sizeof(void*)*(v->size-p-1));
    v->data[p] = d;
}

void lkPointersVector_insert_array(lkPointersVector v, unsigned p, void **d, unsigned size) {
    if(p >= v->size) {
        lkPointersVector_push_back_array(v, d, size);
        return;
    }
    lkPointersVector_reserve(v, v->size+size);
    memmove(v->data+p+size, v->data+p, sizeof(void*)*(v->size-p));
    memcpy(v->data+p, d, sizeof(void*)*size);
    v->size += size;
}

void lkPointersVector_insert_vector(lkPointersVector v, unsigned p, lkPointersVector s) {
    if(p >= v->size) {
        lkPointersVector_push_back_vector(v, s);
        return;
    }
    lkPointersVector_reserve(v, v->size+s->size);
    memmove(v->data+p+s->size, v->data+p, sizeof(void*)*(v->size-p));
    memcpy(v->data+p, s->data, sizeof(void*)*s->size);
    v->size += s->size;
}

void lkPointersVector_erase(lkPointersVector v, unsigned p) {
    if(p >= v->size)
        return;
    memmove(v->data+p, v->data+p+1, sizeof(void*)*(v->size-p-1));
    v->size--;
}

void lkPointersVector_erase_range(lkPointersVector v, unsigned f, unsigned l) {
    if(f >= v->size || l < f)
        return;
    if(l > v->size)
        l = v->size;
    if(l < v->size) {
        memmove(v->data+f, v->data+l+1, sizeof(void*)*(v->size-l));
        v->size -= l-f+1;
    } else
        v->size = f;
}
