/**
 * type-generic ordered map structure for C.
 * Author: Nergal
 * License: MIT
 */

#ifndef CMAP_INCLUDED
#	define CMAP_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "carray.h"
#include "cstr.h"

#define CMAP_API    static


static size_t str_hash(const char *const key) {
	size_t h = 0;
	for( size_t i=0; key[i] != 0; i++ ) {
		h = (h<<6) ^ (h>>26) ^ key[i];
	}
	return h;
}


enum MapEntryType {
	InvalidEntry,
	CellEntry,
	ArrayEntry,
	StrEntry
};

union MapEntryData {
	cell_t        i;
	struct CArray a; /// either cell_t* or char*.
};

CMAP_API union MapEntryData entry_data_from_int(const cell_t n) {
	union MapEntryData d = {0};
	d.i = n;
	return d;
}

CMAP_API union MapEntryData entry_data_from_array(uint8_t *arr, const size_t elen, size_t vlen, const bool is_str) {
	union MapEntryData d = {0};
	if( is_str && vlen==0 )
		vlen = strlen(( char* )arr);
	
	carray_reserve(&d.a, elen, (is_str)? vlen+1 : vlen);
	carray_insert(&d.a, arr, elen * vlen);
	d.a.len = vlen;
	return d;
}


/// Not as efficient as StringMap's entry techniques but meh.
struct MapEntry {
	union MapEntryData data;
	struct CStr        key;    /// string key;
	size_t             hash;
	enum MapEntryType  tag;
};


CMAP_API struct MapEntry *new_map_entry(const char *cstr, const enum MapEntryType tag, const union MapEntryData data) {
	struct MapEntry *entry = ( struct MapEntry* )calloc(1, sizeof *entry);
	if( entry != NULL ) {
		entry->data = data;
		entry->tag = tag;
		entry->key = cstring_create(cstr);
		entry->hash = str_hash(cstr);
	}
	return entry;
}

CMAP_API void map_entry_clear(struct MapEntry *entry) {
	cstring_clear(&entry->key);
	switch( entry->tag ) {
		case ArrayEntry:
		case StrEntry:
			carray_clear(&entry->data.a); break;
		default: break;
	}
	memset(entry, 0, sizeof *entry);
}

CMAP_API void map_entry_free(struct MapEntry **entry_ref) {
	if( *entry_ref==NULL )
		return;
	
	map_entry_clear(*entry_ref);
	free(*entry_ref); *entry_ref = NULL;
}

/*****************************************************************************************/


struct CMap {
	/// `vec` saves insertion order, `MapEntry*[cap]`.
	/// `buckets` is an array of arrays of `MapEntry*` aka `MapEntry*[1st cap][2nd cap]`.
	struct CArray  vec, *buckets;
	size_t         cap,  len;
};

CMAP_API struct CMap *new_map(const size_t def_size = 8ul) {
	struct CMap *map = ( struct CMap* )calloc(1, sizeof *map);
	if( map != NULL ) {
		map->vec = carray_make(sizeof(struct MapEntry*), def_size);
		map->buckets = ( struct CArray* )recalloc(map->buckets, def_size, sizeof *map->buckets, map->cap);
		map->cap = def_size;
		map->len = 0;
	}
	return map;
}

CMAP_API void map_clear(struct CMap *map) {
	/// easier to destroy the map from the order-preserving vector.
	for( size_t i=0; i<map->vec.len; i++ ) {
		struct MapEntry *entry = *( struct MapEntry** )carray_get(&map->vec, i, sizeof entry);
		map_entry_free(&entry);
	}
	carray_clear(&map->vec);
	
	for( size_t i=0; i<map->cap; i++ ) {
		carray_clear(&map->buckets[i]);
	}
	free(map->buckets); map->buckets = NULL;
	map->len = map->cap = 0;
}

CMAP_API void map_free(struct CMap **map_ref) {
	if( *map_ref==NULL )
		return;
	
	map_clear(*map_ref);
	free(*map_ref); *map_ref = NULL;
}

CMAP_API bool map_has_key(struct CMap *map, const char *key) {
	const size_t hash = str_hash(key);
	const size_t index = hash & (map->cap - 1);
	struct CArray *bucket = &map->buckets[index];
	for( size_t i=0; i<bucket->len; i++ ) {
		struct MapEntry *entry = *( struct MapEntry** )carray_get(bucket, i, sizeof entry);
		if( entry->hash==hash || !strcmp(entry->key.cstr, key) )
			return true;
	}
	return false;
}

CMAP_API bool map_insert_entry(struct CMap *map, struct MapEntry *entry) {
	const size_t index = entry->hash & (map->cap - 1);
	struct CArray *bucket = &map->buckets[index];
	
	/// this will run even if the bucket is empty
	/// as a cap of 0 with len of 0 is still technically full!
	if( carray_full(bucket) && !carray_grow(bucket, sizeof entry) )
		return false;
	
	return carray_insert(bucket, &entry, sizeof entry);
}

CMAP_API bool map_rehash(struct CMap *map, const size_t new_size) {
	const size_t old_cap = map->cap;
	struct CArray *curr = map->buckets;
	map->buckets = ( struct CArray* )calloc(new_size, sizeof *curr);
	if( map->buckets==NULL ) {
		map->buckets = curr;
		return false;
	}
	
	map->cap = new_size;
	for( size_t i=0; i<old_cap; i++ )
		carray_clear(&curr[i]);
	
	free(curr);
	
	for( size_t i=0; i<map->vec.len; i++ ) {
		struct MapEntry *entry = *( struct MapEntry** )carray_get(&map->vec, i, sizeof entry);
		map_insert_entry(map, entry);
	}
	return true;
}

CMAP_API bool map_insert(struct CMap *map, const char *key, const enum MapEntryType tag, const union MapEntryData data) {
	if( map_has_key(map, key) )
		return false;
	else if( map->len >= map->cap && !map_rehash(map, map->cap << 1) )
		return false;
	
	struct MapEntry *entry = new_map_entry(key, tag, data);
	if( entry==NULL ) {
		return false;
	} else if( !map_insert_entry(map, entry)
				|| (carray_full(&map->vec) && !carray_grow(&map->vec, sizeof entry))
				|| !carray_insert(&map->vec, &entry, sizeof entry) ) {
		/// if we can't insert the entry, increase ptr vec size, or insert to ptr vec.
		map_entry_free(&entry);
		return false;
	}
	map->len++;
	return true;
}

CMAP_API struct MapEntry *map_key_get(struct CMap *map, const char *key) {
	if( !map_has_key(map, key) )
		return NULL;
	
	const size_t hash = str_hash(key);
	const size_t index = hash & (map->cap - 1);
	struct CArray *bucket = &map->buckets[index];
	for( size_t i=0; i<bucket->len; i++ ) {
		struct MapEntry *entry = *( struct MapEntry** )carray_get(bucket, i, sizeof entry);
		if( entry->hash==hash || !strcmp(entry->key.cstr, key) )
			return entry;
	}
	return NULL;
}

CMAP_API struct MapEntry *map_idx_get(struct CMap *map, const size_t index) {
	if( index >= map->vec.len )
		return NULL;
	
	struct MapEntry **entry_ref = ( struct MapEntry** )carray_get(&map->vec, index, sizeof *entry_ref);
	if( entry_ref==NULL )
		return NULL;
	
	return *entry_ref;
}

CMAP_API bool map_key_set(struct CMap *map, const char *key, const enum MapEntryType tag, const union MapEntryData data) {
	if( !map_has_key(map, key) )
		return map_insert(map, key, tag, data);
	
	struct MapEntry *entry = map_key_get(map, key);
	if( entry==NULL )
		return false;
	
	if( tag != entry->tag ) {
		switch( entry->tag ) {
			case StrEntry:
			case ArrayEntry:
				carray_clear(&entry->data.a); break;
			default: break;
		}
		entry->tag = tag;
	}
	entry->data = data;
	return true;
}

CMAP_API bool map_idx_set(struct CMap *map, const size_t index, const enum MapEntryType tag, const union MapEntryData data) {
	struct MapEntry *entry = map_idx_get(map, index);
	if( entry==NULL )
		return false;
	
	if( tag != entry->tag ) {
		switch( entry->tag ) {
			case StrEntry:
			case ArrayEntry:
				carray_clear(&entry->data.a); break;
			default: break;
		}
		entry->tag = tag;
	}
	entry->data = data;
	return true;
}

CMAP_API bool map_key_rm(struct CMap *map, const char *key) {
	if( !map_has_key(map, key) )
		return false;
	
	const size_t hash = str_hash(key);
	const size_t index = hash & (map->cap - 1);
	struct CArray *bucket = &map->buckets[index];
	for( size_t i=0; i<bucket->len; i++ ) {
		struct MapEntry *entry = *( struct MapEntry** )carray_get(bucket, i, sizeof entry);
		if( entry->hash==hash || !strcmp(entry->key.cstr, key) ) {
			const size_t entry_idx = carray_index_of(&map->vec, &entry, sizeof entry, 0);
			if( entry_idx==SIZE_MAX )
				continue;
			
			map_entry_free(&entry);
			carray_del_by_index(bucket,    i,         sizeof entry);
			carray_del_by_index(&map->vec, entry_idx, sizeof entry);
			return true;
		}
	}
	return false;
}

CMAP_API bool map_idx_rm(struct CMap *map, const size_t n) {
	struct MapEntry *entry = map_idx_get(map, n);
	if( entry==NULL )
		return false;
	
	const size_t index = entry->hash & (map->cap - 1);
	struct CArray *bucket = &map->buckets[index];
	const size_t entry_idx = carray_index_of(bucket, &entry, sizeof entry, 0);
	if( entry_idx==SIZE_MAX )
		return false;
	
	const bool bucket_res = carray_del_by_index(bucket, entry_idx, sizeof entry);
	const bool vec_res = carray_del_by_index(&map->vec, n, sizeof entry);
	if( bucket_res && vec_res ) {
		map_entry_free(&entry);
		return true;
	}
	return false;
}

/********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /** CMAP_INCLUDED */