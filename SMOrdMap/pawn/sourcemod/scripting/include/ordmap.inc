#if defined _ordmap_included
	#endinput
#endif
#define _ordmap_included

#include <core>
#include <handles>


enum MapEntryType {
	InvalidEntry,
	CellEntry,
	ArrayEntry,
	StringEntry
};

methodmap OrdMap < Handle {
	public native OrdMap(int default_size = 8);
	
	property int Len {
		public native get();
	}
	
	/**
	 * HasKey
	 * Returns `true` if the OrderedMap has the key, `false` otherwise.
	 */
	public native bool HasKey(const char[] key);
	
	public bool HasCellKey(any key) {
		char str_key[6]; PackCellToStr(key, str_key);
		return this.HasKey(str_key);
	}
	
	/**
	 * InsertCell, InsertArray, InsertString
	 * Returns `true` if the item is inserted, `false` if key exists or unable to insert.
	 */
	public native bool InsertCell(const char[] key, any item);
	public native bool InsertArray(const char[] key, const any[] items, int len);
	public native bool InsertString(const char[] key, const char[] str);
	
	public bool CellKeyInsertCell(any key, any item) {
		char str_key[6]; PackCellToStr(key, str_key);
		return this.InsertCell(str_key, item);
	}
	public bool CellKeyInsertArray(any key, const any[] items, int len) {
		char str_key[6]; PackCellToStr(key, str_key);
		return this.InsertArray(str_key, items, len);
	}
	public bool CellKeyInsertString(any key, const char[] str) {
		char str_key[6]; PackCellToStr(key, str_key);
		return this.InsertString(str_key, str);
	}
	
	/**
	 * GetCellByKey, GetCellByIndex
	 * Returns `true` if the item exists, `false` if key doesn't exist or unable to get.
	 */
	public native bool GetCellByKey(const char[] key, any& item);
	public native bool GetCellByIndex(int index, any& item);
	
	public bool GetCellByCellKey(any key, any& item) {
		char str_key[6]; PackCellToStr(key, str_key);
		return this.GetCellByKey(str_key, item);
	}
	
	/**
	 * GetArrayLenByKey, GetArrayLenByIndex, GetStringLenByKey, GetStringLenByIndex
	 * Returns length of string/array, returns `0` if key doesn't exist or item isn't a string/array.
	 * 
	 * NOTE: `GetStringLenByKey` & `GetStringLenByIndex` both get string length + 1 for null-terminator.
	 */
	public native int GetArrayLenByKey(const char[] key);
	public native int GetArrayLenByIndex(int index);
	
	public int GetArrayLenByCellKey(any cell_key) {
		char str_key[6]; PackCellToStr(cell_key, str_key);
		return this.GetArrayLenByKey(str_key);
	}
	
	public native int GetStringLenByKey(const char[] key);
	public native int GetStringLenByIndex(int index);
	
	public int GetStringLenByCellKey(any cell_key) {
		char str_key[6]; PackCellToStr(cell_key, str_key);
		return this.GetStringLenByKey(str_key);
	}
	
	/**
	 * GetArrayByKey, GetArrayByIndex, GetStringByKey, GetStringByIndex
	 * Returns `true` if the item exists and buffer is written, `false` if key doesn't exist or unable to write to buffer.
	 */
	public native bool GetArrayByKey(const char[] key, any[] items, int len);
	public native bool GetArrayByIndex(int index, any[] items, int len);
	
	public bool GetArrayByCellKey(any cell_key, any[] items, int len) {
		char str_key[6]; PackCellToStr(cell_key, str_key);
		return this.GetArrayByKey(str_key, items, len);
	}
	
	public native bool GetStringByKey(const char[] key, char[] buffer, int len);
	public native bool GetStringByIndex(int index, char[] buffer, int len);
	
	public bool GetStringByCellKey(any cell_key, char[] buffer, int len) {
		char str_key[6]; PackCellToStr(cell_key, str_key);
		return this.GetStringByKey(str_key, buffer, len);
	}
	
	/**
	 * SetCellByKey, SetCellByIndex, SetArrayByKey, SetArrayByIndex, SetStringByKey, SetStringByIndex
	 * Returns `true` if the entry data is set, `false` if unable to.
	 * If the key doesn't exist, `Set*` acts as `Insert*`.
	 *
	 * NOTE: Swapping data by indices will RUIN insertion order.
	 * NOTE: Setting a data is NOT type-protected,
	 * it WILL overwrite the key/index with what data you're putting.
	 */
	public native bool SetCellByKey(const char[] key, any item);
	public native bool SetCellByIndex(int index, any item);
	
	public bool SetCellByCellKey(any cell_key, any item) {
		char str_key[6]; PackCellToStr(cell_key, str_key);
		return this.SetCellByKey(str_key, item);
	}
	
	public native bool SetArrayByKey(const char[] key, const any[] items, int len);
	public native bool SetArrayByIndex(int index, const any[] items, int len);
	
	public bool SetArrayByCellKey(any cell_key, const any[] items, int len) {
		char str_key[6]; PackCellToStr(cell_key, str_key);
		return this.SetArrayByKey(str_key, items, len);
	}
	
	public native bool SetStringByKey(const char[] key, const char[] str);
	public native bool SetStringByIndex(int index, const char[] str);
	
	public bool SetStringByCellKey(any cell_key, const char[] str) {
		char str_key[6]; PackCellToStr(cell_key, str_key);
		return this.SetStringByKey(str_key, str);
	}
	
	/**
	 * GetEntryTypeByKey, GetEntryTypeByIndex
	 * Returns the type of the entry, `InvalidEntry` if key/index doesn't exist or operation failure.
	 */
	public native MapEntryType GetEntryTypeByKey(const char[] key);
	public native MapEntryType GetEntryTypeByIndex(int index);
	
	public MapEntryType GetEntryTypeByCellKey(any cell_key) {
		char str_key[6]; PackCellToStr(cell_key, str_key);
		return this.GetEntryTypeByKey(str_key);
	}
	
	/**
	 * RemoveByKey, RemoveByIndex
	 * Returns `true` if the entry is removed, `false` if unable to.
	 */
	public native bool RemoveByKey(const char[] key);
	public native bool RemoveByIndex(int index);
	
	public bool RemoveByCellKey(any cell_key) {
		char str_key[6]; PackCellToStr(cell_key, str_key);
		return this.RemoveByKey(str_key);
	}
	
	/**
	 * Clear
	 * Removes ALL entries.
	 */
	public native void Clear();
};

/**
 * PackCellToStr
 * Credit: Asher 'Asherkin' Baker
 * Packs a key, as an integer, into a null-terminated buffer.
 */
stock void PackCellToStr(any key, char buffer[6]) {
	int i = key;
	buffer[0] = ((i >> 28) & 0x7F) | 0x80;
	buffer[1] = ((i >> 21) & 0x7F) | 0x80;
	buffer[2] = ((i >> 14) & 0x7F) | 0x80;
	buffer[3] = ((i >> 7) & 0x7F) | 0x80;
	buffer[4] = ((i) & 0x7F) | 0x80;
	buffer[5] = 0;
}


/**
 * Do not edit below this line!
 */
public Extension __ext_ordmap = {
	name = "OrdMap",
	file = "ordmap.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};

public void __ext_ordmap_SetNTVOptional()
{
	MarkNativeAsOptional("OrdMap.OrdMap");
	MarkNativeAsOptional("OrdMap.Len.get");
	MarkNativeAsOptional("OrdMap.HasKey");
	
	MarkNativeAsOptional("OrdMap.InsertCell");
	MarkNativeAsOptional("OrdMap.InsertArray");
	MarkNativeAsOptional("OrdMap.InsertString");
	
	MarkNativeAsOptional("OrdMap.GetCellByKey");
	MarkNativeAsOptional("OrdMap.GetCellByIndex");
	
	MarkNativeAsOptional("OrdMap.GetArrayLenByKey");
	MarkNativeAsOptional("OrdMap.GetArrayLenByIndex");
	MarkNativeAsOptional("OrdMap.GetStringLenByKey");
	MarkNativeAsOptional("OrdMap.GetStringLenByIndex");
	
	MarkNativeAsOptional("OrdMap.GetArrayByKey");
	MarkNativeAsOptional("OrdMap.GetArrayByIndex");
	MarkNativeAsOptional("OrdMap.GetStringByKey");
	MarkNativeAsOptional("OrdMap.GetStringByIndex");
	
	MarkNativeAsOptional("OrdMap.SetCellByKey");
	MarkNativeAsOptional("OrdMap.SetCellByIndex");
	MarkNativeAsOptional("OrdMap.SetArrayByKey");
	MarkNativeAsOptional("OrdMap.SetArrayByIndex");
	MarkNativeAsOptional("OrdMap.SetStringByKey");
	MarkNativeAsOptional("OrdMap.SetStringByIndex");
	
	MarkNativeAsOptional("OrdMap.GetEntryTypeByKey");
	MarkNativeAsOptional("OrdMap.GetEntryTypeByIndex");
	
	MarkNativeAsOptional("OrdMap.RemoveByKey");
	MarkNativeAsOptional("OrdMap.RemoveByIndex");
	
	MarkNativeAsOptional("OrdMap.Clear");
}