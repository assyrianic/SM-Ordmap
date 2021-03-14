#include "natives.h"
#include <cstdlib>


static HandleSecurity MakeHandleSec() {
	HandleSecurity sec;
	sec.pOwner = NULL;
	sec.pIdentity = myself->GetIdentity();
	return sec;
}

static char *GetParamString(IPluginContext *pContext, const cell_t param) {
	char *key = NULL;
	const int err = pContext->LocalToString(param, &key);
	if( key==NULL ) {
		pContext->ThrowNativeError("Invalid Key String %x (error %d)", key, err);
	}
	return key;
}

static cell_t *GetCellAddr(IPluginContext *pContext, const cell_t param) {
	cell_t *a = NULL;
	const int err = pContext->LocalToPhysAddr(param, &a);
	if( a==NULL ) {
		pContext->ThrowNativeError("Invalid Array %x (error %d)", a, err);
	}
	return a;
}

/// OrdMap(int default_size = 8);
static cell_t Native_OrdMap_Ctor(IPluginContext *pContext, const cell_t *params)
{
	if( params[1] < 0 ) {
		pContext->ThrowNativeError("Invalid Default Size (%d) for OrdMap constructor", params[1]);
		return BAD_HANDLE;
	}
	
	const size_t default_size = ( size_t )params[1];
	CMap *map = new_map(default_size);
	if( map==nullptr )
		return BAD_HANDLE;
	
	return g_pHandleSys->CreateHandle(g_OrdMapType, map, pContext->GetIdentity(), myself->GetIdentity(), NULL);
}

/// property int Len.get
static cell_t Native_OrdMap_Len(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	return ( cell_t )map->vec.len;
}

/// bool HasKey(const char[] key);
static cell_t Native_OrdMap_HasKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0; /// already threw error msg.
	
	return( cell_t )map_has_key(map, key);
}

/// bool InsertCell(const char[] key, any item);
static cell_t Native_OrdMap_InsertCell(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	union MapEntryData d;
	d.i = params[3];
	return ( cell_t )map_insert(map, key, CellEntry, d);
}

/// bool InsertArray(const char[] key, const any[] items, int len);
static cell_t Native_OrdMap_InsertArray(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	cell_t *array = GetCellAddr(pContext, params[3]);
	if( array==NULL )
		return 0;
	
	const size_t array_len = ( size_t )params[4];
	union MapEntryData d = entry_data_from_array(( uint8_t* )array, sizeof(cell_t), array_len, false);
	return ( cell_t )map_insert(map, key, ArrayEntry, d);
}

/// bool InsertString(const char[] key, const char[] str);
static cell_t Native_OrdMap_InsertString(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	char *str = GetParamString(pContext, params[3]);
	if( str==NULL )
		return 0;
	
	union MapEntryData d = entry_data_from_array(( uint8_t* )str, sizeof(char), 0, true);
	return ( cell_t )map_insert(map, key, StrEntry, d);
}

/// bool GetCellByKey(const char[] key, any& item);
static cell_t Native_OrdMap_GetCellByKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	MapEntry *entry = map_key_get(map, key);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for key '%s'", key);
		return 0;
	} else if( entry->tag != CellEntry ) {
		pContext->ThrowNativeError("OrdMap entry '%s' is not a cell type", key);
		return 0;
	}
	
	cell_t *item = GetCellAddr(pContext, params[3]);
	if( item==NULL )
		return 0;
	
	*item = entry->data.i;
	return 1;
}

/// bool GetCellByIndex(int index, any& item);
static cell_t Native_OrdMap_GetCellByIndex(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[2] < 0 ) {
		pContext->ThrowNativeError("cannot use negative index (%d) to get a cell from OrdMap", params[2]);
		return 0;
	}
	
	const size_t index = ( size_t )params[2];
	MapEntry *entry = map_idx_get(map, index);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for index '%zu'", index);
		return 0;
	} else if( entry->tag != CellEntry ) {
		pContext->ThrowNativeError("OrdMap entry index '%zu' is not a cell type", index);
		return 0;
	}
	
	cell_t *item = GetCellAddr(pContext, params[3]);
	if( item==NULL )
		return 0;
	
	*item = entry->data.i;
	return 1;
}

/// int GetArrayLenByKey(const char[] key);
static cell_t Native_OrdMap_GetArrayLenByKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	MapEntry *entry = map_key_get(map, key);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for key '%s'", key);
		return 0;
	} else if( entry->tag != ArrayEntry ) {
		pContext->ThrowNativeError("OrdMap entry '%s' is not an array type", key);
		return 0;
	}
	return( cell_t )entry->data.a.len;
}

/// int GetArrayLenByIndex(int index);
static cell_t Native_OrdMap_GetArrayLenByIndex(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[2] < 0 ) {
		pContext->ThrowNativeError("cannot use negative index (%d) to get array length from OrdMap", params[2]);
		return 0;
	}
	
	const size_t index = ( size_t )params[2];
	MapEntry *entry = map_idx_get(map, index);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for index '%zu'", index);
		return 0;
	} else if( entry->tag != ArrayEntry ) {
		pContext->ThrowNativeError("OrdMap entry index '%zu' is not an array type", index);
		return 0;
	}
	return( cell_t )entry->data.a.len;
}

/// int GetStringLenByKey(const char[] key);
static cell_t Native_OrdMap_GetStringLenByKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	MapEntry *entry = map_key_get(map, key);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for key '%s'", key);
		return 0;
	} else if( entry->tag != StrEntry ) {
		pContext->ThrowNativeError("OrdMap entry '%s' is not a string type", key);
		return 0;
	}
	return( cell_t )entry->data.a.len + 1;
}

/// int GetStringLenByIndex(int index);
static cell_t Native_OrdMap_GetStringLenByIndex(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[2] < 0 ) {
		pContext->ThrowNativeError("cannot use negative index (%d) to get string length from OrdMap", params[2]);
		return 0;
	}
	
	const size_t index = ( size_t )params[2];
	MapEntry *entry = map_idx_get(map, index);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for index '%zu'", index);
		return 0;
	} else if( entry->tag != StrEntry ) {
		pContext->ThrowNativeError("OrdMap entry index '%zu' is not a string type", index);
		return 0;
	}
	return( cell_t )entry->data.a.len + 1;
}

/// bool GetArrayByKey(const char[] key, any[] items, int len);
static cell_t Native_OrdMap_GetArrayByKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[4] < 0 ) {
		pContext->ThrowNativeError("cannot use negative length (%d) as buffer length for OrdMap", params[4]);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	MapEntry *entry = map_key_get(map, key);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for key '%s'", key);
		return 0;
	} else if( entry->tag != ArrayEntry ) {
		pContext->ThrowNativeError("OrdMap entry key '%s' is not an array type", key);
		return 0;
	}
	
	/// only allow an equal or larger buffer size.
	const size_t given_len = ( size_t )params[4];
	if( entry->data.a.len > given_len ) {
		pContext->ThrowNativeError("buffer is too small for array entry of key '%s'", key);
		return 0;
	}
	
	cell_t *item = NULL;
	pContext->LocalToPhysAddr(params[3], &item);
	const cell_t *datum = ( const cell_t* )entry->data.a.table;
	for( size_t i=0; i<given_len; i++ ) {
		item[i] = datum[i];
	}
	return 1;
}

/// bool GetArrayByIndex(int index, any[] items, int len);
static cell_t Native_OrdMap_GetArrayByIndex(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[2] < 0 ) {
		pContext->ThrowNativeError("cannot use negative index (%d) to get array from OrdMap", params[2]);
		return 0;
	} else if( params[4] < 0 ) {
		pContext->ThrowNativeError("cannot use negative length (%d) as buffer length from OrdMap", params[4]);
		return 0;
	}
	
	const size_t index = ( size_t )params[2];
	MapEntry *entry = map_idx_get(map, index);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for index '%zu'", index);
		return 0;
	} else if( entry->tag != ArrayEntry ) {
		pContext->ThrowNativeError("OrdMap entry index '%zu' is not an array type", index);
		return 0;
	}
	
	/// only allow an equal or larger buffer size.
	const size_t given_len = ( size_t )params[4];
	if( entry->data.a.len > given_len ) {
		pContext->ThrowNativeError("buffer is too small for array entry of index '%zu'", index);
		return 0;
	}
	
	cell_t *item = NULL;
	pContext->LocalToPhysAddr(params[3], &item);
	const cell_t *datum = ( const cell_t* )entry->data.a.table;
	for( size_t i=0; i<given_len; i++ ) {
		item[i] = datum[i];
	}
	return 1;
}

/// bool GetStringByKey(const char[] key, char[] buffer, int len);
static cell_t Native_OrdMap_GetStringByKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[4] < 0 ) {
		pContext->ThrowNativeError("cannot use negative length (%d) as buffer length for OrdMap", params[4]);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	MapEntry *entry = map_key_get(map, key);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for key '%s'", key);
		return 0;
	} else if( entry->tag != StrEntry ) {
		pContext->ThrowNativeError("OrdMap entry key '%s' is not a string type", key);
		return 0;
	}
	
	/// only allow an equal or larger buffer size.
	const size_t given_len = ( size_t )params[4];
	if( entry->data.a.len > given_len ) {
		pContext->ThrowNativeError("buffer is too small for string entry of key '%s'", key);
		return 0;
	}
	
	char *buf = GetParamString(pContext, params[3]);
	if( buf==NULL )
		return 0;
	
	const char *datum = ( const char* )entry->data.a.table;
	for( size_t i=0; i<given_len; i++ ) {
		buf[i] = datum[i];
	}
	return 1;
}

/// bool GetStringByIndex(int index, char[] buffer, int len);
static cell_t Native_OrdMap_GetStringByIndex(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[2] < 0 ) {
		pContext->ThrowNativeError("cannot use negative index (%d) to get string from OrdMap", params[2]);
		return 0;
	} else if( params[4] < 0 ) {
		pContext->ThrowNativeError("cannot use negative length (%d) as buffer length from OrdMap", params[4]);
		return 0;
	}
	
	const size_t index = ( size_t )params[2];
	MapEntry *entry = map_idx_get(map, index);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for index '%zu'", index);
		return 0;
	} else if( entry->tag != StrEntry ) {
		pContext->ThrowNativeError("OrdMap entry index '%zu' is not a string type", index);
		return 0;
	}
	
	/// only allow an equal or larger buffer size.
	const size_t given_len = ( size_t )params[4];
	if( entry->data.a.len > given_len ) {
		pContext->ThrowNativeError("buffer is too small for array entry of index '%zu'", index);
		return 0;
	}
	char *buf = GetParamString(pContext, params[3]);
	if( buf==NULL )
		return 0;
		
	const char *datum = ( const char* )entry->data.a.table;
	for( size_t i=0; i<given_len; i++ ) {
		buf[i] = datum[i];
	}
	return 1;
}

/// bool SetCellByKey(const char[] key, any item);
static cell_t Native_OrdMap_SetCellByKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	union MapEntryData d;
	d.i = params[3];
	return ( cell_t )map_key_set(map, key, CellEntry, d);
}

/// bool SetCellByIndex(int index, any item);
static cell_t Native_OrdMap_SetCellByIndex(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[2] < 0 ) {
		pContext->ThrowNativeError("cannot use negative index (%d) to set a cell from OrdMap", params[2]);
		return 0;
	}
	const size_t index = ( size_t )params[2];
	union MapEntryData d;
	d.i = params[3];
	return ( cell_t )map_idx_set(map, index, CellEntry, d);
}

/// bool SetArrayByKey(const char[] key, const any[] items, int len);
static cell_t Native_OrdMap_SetArrayByKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	cell_t *array = GetCellAddr(pContext, params[3]);
	if( array==NULL )
		return 0;

	const size_t array_len = ( size_t )params[4];
	union MapEntryData d = entry_data_from_array(( uint8_t* )array, sizeof(cell_t), array_len, false);
	return ( cell_t )map_key_set(map, key, ArrayEntry, d);
}

/// bool SetArrayByIndex(int index, const any[] items, int len);
static cell_t Native_OrdMap_SetArrayByIndex(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[2] < 0 ) {
		pContext->ThrowNativeError("cannot use negative index (%d) to set array from OrdMap", params[2]);
		return 0;
	} else if( params[4] < 0 ) {
		pContext->ThrowNativeError("cannot use negative length (%d) as buffer length from OrdMap", params[4]);
		return 0;
	}
	const size_t index = ( size_t )params[2];
	
	cell_t *array = GetCellAddr(pContext, params[3]);
	if( array==NULL )
		return 0;

	const size_t array_len = ( size_t )params[4];
	union MapEntryData d = entry_data_from_array(( uint8_t* )array, sizeof(cell_t), array_len, false);
	return ( cell_t )map_idx_set(map, index, ArrayEntry, d);
}

/// bool SetStringByKey(const char[] key, const char[] str);
static cell_t Native_OrdMap_SetStringByKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	char *str = GetParamString(pContext, params[3]);
	if( str==NULL )
		return 0;
	
	union MapEntryData d = entry_data_from_array(( uint8_t* )str, sizeof(char), 0, true);
	return ( cell_t )map_key_set(map, key, StrEntry, d);
}

/// bool SetStringByIndex(int index, const char[] str);
static cell_t Native_OrdMap_SetStringByIndex(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[2] < 0 ) {
		pContext->ThrowNativeError("cannot use negative index (%d) to set string from OrdMap", params[2]);
		return 0;
	}
	const size_t index = ( size_t )params[2];
	char *str = NULL;
	pContext->LocalToString(params[3], &str);
	union MapEntryData d = entry_data_from_array(( uint8_t* )str, sizeof(char), 0, true);
	return ( cell_t )map_idx_set(map, index, StrEntry, d);
}

/// MapEntryType GetEntryTypeByKey(const char[] key);
static cell_t Native_OrdMap_GetEntryTypeByKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[4] < 0 ) {
		pContext->ThrowNativeError("cannot use negative length (%d) as buffer length for OrdMap", params[4]);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	MapEntry *entry = map_key_get(map, key);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for key '%s'", key);
		return 0;
	}
	return( cell_t )entry->tag;
}

/// MapEntryType GetEntryTypeByIndex(int index);
static cell_t Native_OrdMap_GetEntryTypeByIndex(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[2] < 0 ) {
		pContext->ThrowNativeError("cannot use negative index (%d) to get entry type from OrdMap", params[2]);
		return 0;
	}
	const size_t index = ( size_t )params[2];
	MapEntry *entry = map_idx_get(map, index);
	if( entry==nullptr ) {
		pContext->ThrowNativeError("Unable to retrieve OrdMap entry for index '%zu'", index);
		return 0;
	}
	return( cell_t )entry->tag;
}

/// bool RemoveByKey(const char[] key);
static cell_t Native_OrdMap_RemoveByKey(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	
	char *key = GetParamString(pContext, params[2]);
	if( key==NULL )
		return 0;
	
	return ( cell_t )map_key_rm(map, key);
}

/// bool RemoveByIndex(int index);
static cell_t Native_OrdMap_RemoveByIndex(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	} else if( params[2] < 0 ) {
		pContext->ThrowNativeError("cannot use negative index (%d) to remove from OrdMap", params[2]);
		return 0;
	}
	const size_t n = ( size_t )params[2];
	return ( cell_t )map_idx_rm(map, n);
}

/// void Clear();
static cell_t Native_OrdMap_Clear(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = static_cast< Handle_t >(params[1]);
	HandleSecurity sec = MakeHandleSec();
	
	CMap *map = NULL;
	HandleError err;
	if( (err = g_pHandleSys->ReadHandle(hndl, g_OrdMapType, &sec, ( void** )&map)) != HandleError_None ) {
		pContext->ThrowNativeError("Invalid OrdMap Handle %x (error %d)", hndl, err);
		return 0;
	}
	map_clear(map);
	return 1;
}

sp_nativeinfo_t g_Natives[] = {
	{"OrdMap.OrdMap",              Native_OrdMap_Ctor},
	{"OrdMap.Len.get",             Native_OrdMap_Len},
	
	{"OrdMap.HasKey",              Native_OrdMap_HasKey},
	
	{"OrdMap.InsertCell",          Native_OrdMap_InsertCell},
	{"OrdMap.InsertArray",         Native_OrdMap_InsertArray},
	{"OrdMap.InsertString",        Native_OrdMap_InsertString},
	
	{"OrdMap.GetCellByKey",        Native_OrdMap_GetCellByKey},
	{"OrdMap.GetCellByIndex",      Native_OrdMap_GetCellByIndex},
	
	{"OrdMap.GetArrayLenByKey",    Native_OrdMap_GetArrayLenByKey},
	{"OrdMap.GetArrayLenByIndex",  Native_OrdMap_GetArrayLenByIndex},
	{"OrdMap.GetStringLenByKey",   Native_OrdMap_GetStringLenByKey},
	{"OrdMap.GetStringLenByIndex", Native_OrdMap_GetStringLenByIndex},
	
	{"OrdMap.GetArrayByKey",       Native_OrdMap_GetArrayByKey},
	{"OrdMap.GetArrayByIndex",     Native_OrdMap_GetArrayByIndex},
	{"OrdMap.GetStringByKey",      Native_OrdMap_GetStringByKey},
	{"OrdMap.GetStringByIndex",    Native_OrdMap_GetStringByIndex},
	
	{"OrdMap.SetCellByKey",        Native_OrdMap_SetCellByKey},
	{"OrdMap.SetCellByIndex",      Native_OrdMap_SetCellByIndex},
	
	{"OrdMap.SetArrayByKey",       Native_OrdMap_SetArrayByKey},
	{"OrdMap.SetArrayByIndex",     Native_OrdMap_SetArrayByIndex},
	{"OrdMap.SetStringByKey",      Native_OrdMap_SetStringByKey},
	{"OrdMap.SetStringByIndex",    Native_OrdMap_SetStringByIndex},
	
	{"OrdMap.GetEntryTypeByKey",   Native_OrdMap_GetEntryTypeByKey},
	{"OrdMap.GetEntryTypeByIndex", Native_OrdMap_GetEntryTypeByIndex},
	
	{"OrdMap.RemoveByKey",         Native_OrdMap_RemoveByKey},
	{"OrdMap.RemoveByIndex",       Native_OrdMap_RemoveByIndex},
	
	{"OrdMap.Clear",               Native_OrdMap_Clear},
	
	{NULL,                         NULL}
};