/**
 * CArray is a low level, type-generic C array structure that can be used as either a fixed-size array substitute or a dynamic array substitute.
 * CArray container does not automatically grow or shrink the array when full or undersized.
 * 
 * Author: Nergal
 * License: MIT
 */

#ifndef CARRAY_INCLUDED
#	define CARRAY_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "recalloc.h"

#define CARRAY_API    static


enum { VEC_DEFAULT_SIZE = 4 };

CARRAY_API size_t _next_pow2(size_t x) {
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
#if SIZE_MAX==UINT64_MAX
	x |= x >> 32;
#endif
	return x + 1;
}


typedef struct CArray {
	uint8_t *table;
	size_t   cap, len;
} SCArray; /// beginning 'S' is for 'struct'.


CARRAY_API bool carray_resizer(struct CArray *const vec, const size_t new_size, const size_t element_size) {
	if( vec->cap==new_size ) {
		return true;
	} else {
		void *new_table = recalloc(vec->table, new_size, element_size, vec->cap);
		if( new_table != NULL ) {
			vec->table = ( uint8_t* )new_table;
			vec->cap = new_size;
			return true;
		}
		return false;
	}
}


CARRAY_API struct CArray carray_make(const size_t datasize, const size_t init_size) {
	struct CArray vec = { 0 };
	carray_resizer(&vec, (init_size < VEC_DEFAULT_SIZE ? VEC_DEFAULT_SIZE : init_size), datasize);
	return vec;
}
CARRAY_API struct CArray carray_make_from_array(void *const buf, const size_t cap, const size_t len) {
	return( struct CArray ){ .table = ( uint8_t* )buf, .cap = cap, .len = len };
}

/// creator funcs.
CARRAY_API struct CArray *carray_new(const size_t datasize, const size_t init_size) {
	struct CArray *const vec = ( struct CArray* )calloc(1, sizeof *vec);
	if( vec != NULL ) {
		*vec = carray_make(datasize, init_size);
	}
	return vec;
}
CARRAY_API struct CArray *carray_new_from_array(void *const buf, const size_t cap, const size_t len) {
	struct CArray *const vec = ( struct CArray* )calloc(1, sizeof *vec);
	if( vec != NULL ) {
		*vec = carray_make_from_array(buf, cap, len);
	}
	return vec;
}


/// clean up funcs.
CARRAY_API void carray_clear(struct CArray *const vec) {
	free(vec->table); vec->table = NULL;
	vec->cap = vec->len = 0;
}
CARRAY_API void carray_free(struct CArray **const vecref) {
	free(*vecref); *vecref = NULL;
}
CARRAY_API void carray_cleanup(struct CArray **const vecref) {
	if( *vecref != NULL ) {
		carray_clear(*vecref);
	}
	free(*vecref); *vecref = NULL;
}


/// array info getters.
CARRAY_API void *const *carray_data(const struct CArray *const vec) {
	return ( void *const * )&vec->table;
}
CARRAY_API size_t carray_cap(const struct CArray *const vec) {
	return vec->cap;
}
CARRAY_API size_t carray_len(const struct CArray *const vec) {
	return vec->len;
}


/// array table ops.
CARRAY_API bool carray_grow(struct CArray *const vec, const size_t datasize) {
	const size_t old_cap = vec->cap;
	carray_resizer(vec, (vec->cap==0 ? VEC_DEFAULT_SIZE : _next_pow2(vec->cap << 1)), datasize);
	return vec->cap > old_cap;
}
CARRAY_API bool carray_resize(struct CArray *const vec, const size_t datasize, const size_t new_cap) {
	const size_t old_cap = vec->cap;
	carray_resizer(vec, (vec->cap==0 || new_cap==0 ? VEC_DEFAULT_SIZE : new_cap), datasize);
	return vec->cap != old_cap;
}
CARRAY_API bool carray_shrink(struct CArray *const vec, const size_t datasize, const bool exact_fit) {
	if( vec->cap <= VEC_DEFAULT_SIZE || vec->len==0 ) {
		return false;
	} else {
		const size_t old_cap = vec->cap;
		carray_resizer(vec, (exact_fit ? vec->len : _next_pow2(vec->len)), datasize);
		return old_cap > vec->cap;
	}
}
CARRAY_API bool carray_reserve(struct CArray *const vec, const size_t datasize, const size_t amount) {
	return carray_resizer(vec, (amount==0 ? VEC_DEFAULT_SIZE : amount), datasize);
}

CARRAY_API void carray_wipe(struct CArray *const vec, const size_t datasize) {
	if( vec->table==NULL ) {
		return;
	} else {
		vec->len = 0;
		memset(&vec->table[0], 0, vec->cap * datasize);
	}
}

CARRAY_API bool carray_empty(const struct CArray *const vec) {
	return( vec->table==NULL || vec->cap==0 || vec->len==0 );
}
CARRAY_API bool carray_full(const struct CArray *const vec) {
	return( vec->len >= vec->cap );
}


/// array to array ops.
CARRAY_API bool carray_add(struct CArray *const vecA, const struct CArray *const vecB, const size_t datasize) {
	if( vecA->table==NULL || vecB->table==NULL || (vecA->len + vecB->len) >= vecA->cap ) {
		return false;
	} else {
		memcpy(&vecA->table[vecA->len * datasize], vecB->table, vecB->len * datasize);
		vecA->len += vecB->len;
		return true;
	}
}
CARRAY_API bool carray_copy(struct CArray *const vecA, const struct CArray *const vecB, const size_t datasize) {
	if( vecA==vecB ) {
		return true;
	} else if( vecB->table==NULL || vecA->table==NULL ) {
		return false;
	} else {
		carray_wipe(vecA, datasize); /// TODO: write over then wipe remaining bytes?
		vecA->len = (vecA->cap < vecB->len ? vecA->cap : vecB->len);
		memcpy(vecA->table, vecB->table, vecA->len * datasize);
		return true;
	}
}
CARRAY_API size_t carray_len_diff(const struct CArray *const vecA, const struct CArray *const vecB) {
	return( vecA->len > vecB->len ) ? (vecA->len - vecB->len) : (vecB->len - vecA->len);
}
CARRAY_API size_t carray_cap_diff(const struct CArray *const vecA, const struct CArray *const vecB) {
	return( vecA->cap > vecB->cap ) ? (vecA->cap - vecB->cap) : (vecB->cap - vecA->cap);
}


/// array data ops.
CARRAY_API bool carray_insert(struct CArray *const vec, const void *const val, const size_t datasize) {
	if( vec->table==NULL || vec->len >= vec->cap ) {
		return false;
	} else {
		memcpy(&vec->table[vec->len++ * datasize], val, datasize);
		return true;
	}
}
CARRAY_API size_t carray_append(struct CArray *const vec, const void *const val, const size_t datasize) {
	if( vec->table==NULL || vec->len >= vec->cap ) {
		return SIZE_MAX;
	} else {
		const size_t index = vec->len;
		vec->len++;
		memcpy(&vec->table[index * datasize], val, datasize);
		return index;
	}
}
CARRAY_API bool carray_fill(struct CArray *const vec, const void *const val, const size_t datasize) {
	if( vec->table==NULL ) {
		return false;
	} else {
		for( size_t i=0; i<vec->cap; i++ ) {
			memcpy(&vec->table[i * datasize], val, datasize);
		}
		vec->len = vec->cap;
		return true;
	}
}

CARRAY_API void *carray_pop(struct CArray *const vec, const size_t datasize) {
	return( vec->table==NULL || vec->len==0 ) ? NULL : &vec->table[--vec->len * datasize];
}
CARRAY_API bool carray_pop_ex(struct CArray *const vec, void *const val, const size_t datasize) {
	if( vec->table==NULL || vec->len==0 ) {
		return false;
	} else {
		memcpy(val, &vec->table[--vec->len * datasize], datasize);
		return true;
	}
}

CARRAY_API void *carray_peek(struct CArray *const vec, const size_t datasize) {
	return( vec->table==NULL || vec->len==0 ) ? NULL : &vec->table[(vec->len - 1) * datasize];
}
CARRAY_API bool carray_peek_ex(struct CArray *const vec, void *const val, const size_t datasize) {
	if( vec->table==NULL || vec->len==0 ) {
		return false;
	} else {
		memcpy(val, &vec->table[(vec->len - 1) * datasize], datasize);
		return true;
	}
}

CARRAY_API void *carray_get(const struct CArray *const vec, const size_t index, const size_t datasize) {
	return( vec->table==NULL || index >= vec->len ) ? NULL : &vec->table[index * datasize];
}
CARRAY_API bool carray_get_ex(const struct CArray *const vec, const size_t index, void *const val, const size_t datasize) {
	if( vec->table==NULL || index >= vec->len ) {
		return false;
	} else {
		memcpy(val, &vec->table[index * datasize], datasize);
		return true;
	}
}

CARRAY_API bool carray_set(struct CArray *const vec, const size_t index, const void *const val, const size_t datasize) {
	if( vec->table==NULL || index >= vec->len ) {
		return false;
	} else {
		memcpy(&vec->table[index * datasize], val, datasize);
		return true;
	}
}

CARRAY_API bool carray_swap(struct CArray *const vec, const size_t datasize) {
	if( vec->table==NULL ) {
		return false;
	} else {
		for( size_t i=0, n = vec->len-1; i < vec->len/2; i++, n-- ) {
			for( size_t x=0; x<datasize; x++ ) {
				const size_t i_offs = (i * datasize) + x;
				const size_t n_offs = (n * datasize) + x;
				const int data = vec->table[n_offs];
				vec->table[n_offs] = vec->table[i_offs];
				vec->table[i_offs] = data;
			}
		}
		return true;
	}
}

CARRAY_API bool carray_shift_up(struct CArray *const vec, const size_t index, const size_t datasize, const size_t amount) {
	if( vec->table==NULL || index >= vec->len ) {
		return false;
	} else {
		const size_t _amnt = amount==0 ? 1 : amount;
		const size_t
			i = index + _amnt,
			j = index
		;
		if( i<vec->len ) {
			vec->len -= _amnt;
			memmove(&vec->table[j * datasize], &vec->table[i * datasize], (vec->len - j) * datasize);
			memset(&vec->table[vec->len * datasize], 0, _amnt * datasize);
		} else {
			/// if i goes out of range, zero everything after and lower the count.
			memset(&vec->table[j * datasize], 0, (vec->len - j) * datasize);
			vec->len = j;
		}
		return true;
	}
}

CARRAY_API size_t carray_item_count(const struct CArray *const vec, const void *const val, const size_t datasize) {
	if( vec->table==NULL ) {
		return 0;
	} else {
		size_t count = 0;
		for( size_t i=0; i<vec->len; i++ ) {
			count += !memcmp(&vec->table[i * datasize], val, datasize);
		}
		return count;
	}
}
CARRAY_API size_t carray_index_of(const struct CArray *const vec, const void *const val, const size_t datasize, const size_t starting_index) {
	if( vec->table==NULL ) {
		return SIZE_MAX;
	} else {
		for( size_t i=starting_index; i<vec->len; i++ ) {
			if( !memcmp(&vec->table[i * datasize], val, datasize) ) {
				return i;
			}
		}
		return SIZE_MAX;
	}
}


/// deletion funcs.
CARRAY_API bool carray_del_by_index(struct CArray *const vec, const size_t index, const size_t datasize) {
	if( vec->table==NULL ) {
		return false;
	} else if( index==vec->len-1 ) {
		memset(&vec->table[--vec->len * datasize], 0, datasize);
		return true;
	} else {
		return carray_shift_up(vec, index, datasize, 1);
	}
}
CARRAY_API bool carray_del_by_range(struct CArray *const vec, const size_t index, const size_t datasize, const size_t range) {
	if( vec->table==NULL ) {
		return false;
	} else if( index==0 && (index + range >= vec->len) ) {
		carray_wipe(vec, datasize);
		return true;
	} else if( index==vec->len-1 ) {
		memset(&vec->table[--vec->len * datasize], 0, datasize);
		return true;
	} else {
		return carray_shift_up(vec, index, datasize, range);
	}
}
CARRAY_API bool carray_del_by_val(struct CArray *const vec, const void *const val, const size_t datasize) {
	const size_t index = carray_index_of(vec, val, datasize, 0);
	return( index != SIZE_MAX ) ? carray_del_by_index(vec, index, datasize) : false;
}
/********************************************************************/


#ifdef __cplusplus
} /** extern "C" */
#endif

#endif /** CARRAY_INCLUDED */