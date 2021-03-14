/**
 * string object type for C.
 * Author: Nergal
 * License: MIT
 */

#ifndef CSTR_INCLUDED
#	define CSTR_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "recalloc.h"

#define CSTR_API    static


typedef struct CStr {
	char  *cstr;
	size_t len;
} SCStr;

CSTR_API bool _resize_string(struct CStr *const str, const size_t new_size) {
	char *new_cstr = ( char* )recalloc(str->cstr, new_size + 1, sizeof(char), str->len);
	if( new_cstr==NULL ) {
		return false;
	} else {
		str->cstr = new_cstr;
		str->len  = new_size;
		return true;
	}
}

CSTR_API bool cstring_copy_cstr(struct CStr *const str, const char *cstr) {
	if( cstr==NULL ) {
		return false;
	} else {
		const size_t cstr_len = strlen(cstr);
		if( !_resize_string(str, cstr_len) ) {
			return false;
		} else {
			strncpy(str->cstr, cstr, str->len);
			return true;
		}
	}
}

CSTR_API struct CStr cstring_create(const char *const cstr) {
	struct CStr string = {0};
	cstring_copy_cstr(&string, cstr);
	return string;
}

CSTR_API struct CStr *cstring_new(const char *const cstr) {
	struct CStr *str = ( struct CStr* )calloc(1, sizeof *str);
	if( str != NULL ) {
		*str = cstring_create(cstr);
	}
	return str;
}

CSTR_API void cstring_clear(struct CStr *const str) {
	if( str->cstr != NULL ) {
		free(str->cstr);
		str->cstr = NULL;
	}
	*str = (struct CStr){0};
}
CSTR_API void cstring_free(struct CStr **const strref) {
	if( *strref==NULL ) {
		return;
	} else {
		cstring_clear(*strref);
		free(*strref), *strref=NULL;
	}
}

CSTR_API char *cstring_cstr(const struct CStr *const str) {
	return str->cstr;
}
CSTR_API size_t cstring_len(const struct CStr *const str) {
	return str->len;
}

CSTR_API bool cstring_add_char(struct CStr *const str, const char c) {
	if( !_resize_string(str, str->len + 1) ) {
		return false;
	} else {
		str->cstr[str->len-1] = c;
		return true;
	}
}
CSTR_API bool cstring_add_str(struct CStr *strA, const struct CStr *strB) {
	if( strB->cstr==NULL || !_resize_string(strA, strA->len + strB->len) ) {
		return false;
	} else {
		strncat(strA->cstr, strB->cstr, strB->len);
		return true;
	}
}
CSTR_API bool cstring_add_cstr(struct CStr *const str, const char *cstr) {
	if( cstr==NULL ) {
		return false;
	} else {
		const size_t cstr_len = strlen(cstr);
		if( !_resize_string(str, str->len + cstr_len) ) {
			return false;
		} else {
			strncat(str->cstr, cstr, cstr_len);
			return true;
		}
	}
}

CSTR_API bool cstring_copy_str(struct CStr *const strA, const struct CStr *const strB) {
	if( strA==strB ) {
		return true;
	} else if( strB->cstr==NULL || !_resize_string(strA, strB->len) ) {
		return false;
	} else {
		strncpy(strA->cstr, strB->cstr, strB->len);
		return true;
	}
}


CSTR_API int32_t cstring_format(struct CStr *const str, const char *fmt, ...) {
	va_list ap, st;
	va_start(ap, fmt);
	va_copy(st, ap);
	/**
		'*snprintf' family returns the size of how large the writing
		would be if the buffer was large enough.
	 */
	char c = 0;
	const int32_t size = vsnprintf(&c, 1, fmt, ap);
	va_end(ap);
	
	const size_t old_size = str->len;
	if( !_resize_string(str, size + old_size) ) {
		va_end(st);
		return -1;
	} else {
		/** vsnprintf always checks n-1 so gotta increase len a bit to accomodate. */
		const int32_t result = vsnprintf(str->cstr, str->len - old_size + 1, fmt, st);
		va_end(st);
		return result;
	}
}

CSTR_API int32_t cstring_scan(const struct CStr *const str, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	const int32_t result = vsscanf(str->cstr, fmt, args);
	va_end(args);
	return result;
}

CSTR_API int32_t cstring_cmpcstr(const struct CStr *const str, const char *cstr) {
	if( cstr==NULL || str->cstr==NULL ) {
		return -1;
	} else {
		const size_t cstr_len = strlen(cstr);
		return strncmp(cstr, str->cstr, (str->len > cstr_len) ? str->len : cstr_len);
	}
}
CSTR_API int32_t cstring_cmpstr(const struct CStr *const strA, const struct CStr *const strB) {
	return( strA->cstr==NULL || strB->cstr==NULL ) ? -1 : strncmp(strA->cstr, strB->cstr, strA->len > strB->len ? strA->len : strB->len);
}

CSTR_API bool cstring_is_empty(const struct CStr *const str) {
	return( str->cstr==NULL || str->len==0 || str->cstr[0]==0 );
}

CSTR_API bool cstring_read_file(struct CStr *const str, FILE *const file) {
	fseek(file, 0, SEEK_END);
	const long int filesize = ftell(file);
	rewind(file);
	if( filesize<=0 || !_resize_string(str, filesize) ) {
		return false;
	} else {
		str->len = fread(str->cstr, sizeof *str->cstr, filesize, file);
		return true;
	}
}

CSTR_API bool cstring_replace(struct CStr *const str, const char to_replace, const char with) {
	if( str->cstr==NULL || to_replace==0 || with==0 ) {
		return false;
	} else {
		for( size_t i=0; i < str->len+1; i++ ) {
			if( str->cstr[i]==to_replace ) {
				str->cstr[i] = with;
			}
		}
		return true;
	}
}
CSTR_API size_t cstring_count(const struct CStr *const str, const char occurrence) {
	if( str->cstr==NULL ) {
		return 0;
	} else {
		size_t counts = 0;
		for( size_t i=0; i < str->len+1; i++ ) {
			if( str->cstr[i]==occurrence ) {
				++counts;
			}
		}
		return counts;
	}
}

CSTR_API bool cstring_upper(struct CStr *const str) {
	if( str->cstr==NULL ) {
		return false;
	} else {
		for( size_t i=0; i < str->len+1; i++ ) {
			if( islower(str->cstr[i]) ) {
				str->cstr[i] = toupper(str->cstr[i]);
			}
		}
		return true;
	}
}
CSTR_API bool cstring_lower(struct CStr *const str) {
	if( str->cstr==NULL ) {
		return false;
	} else {
		for( size_t i=0; i < str->len+1; i++ ) {
			if( isupper(str->cstr[i]) ) {
				str->cstr[i] = tolower(str->cstr[i]);
			}
		}
		return true;
	}
}
CSTR_API bool cstring_reverse(struct CStr *const str) {
	if( str->cstr==NULL ) {
		return false;
	} else {
		const size_t len = str->len / 2;
		for( size_t i=0, n=str->len-1; i<len; i++, n-- ) {
			const int t = str->cstr[i];
			str->cstr[i] = str->cstr[n];
			str->cstr[n] = t;
		}
		return true;
	}
}
/********************************************************************/


#ifdef __cplusplus
}
#endif

#endif /** CSTR_INCLUDED */