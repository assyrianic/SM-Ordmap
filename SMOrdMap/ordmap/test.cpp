#include <iostream>
#include "ordmap.h"

const char *get_tag_str(const MapEntryType tag) {
	switch( tag ) {
		case StrEntry: return "string";
		case CellEntry: return "cell";
		case ArrayEntry: return "array";
		default: return "invalid";
	}
}

void print_entry(MapEntry *entry) {
	if( entry==nullptr ) {
		std::cout << "entry is nil\n";
		return;
	}
	
	std::cout << entry->key.cstr << " | " << get_tag_str(entry->tag) << " | ";
	switch( entry->tag ) {
		case StrEntry:
			std::cout << ( char* )entry->data.a.table << " | " << entry->data.a.len << "\n"; break;
		case CellEntry:
			std::cout << entry->data.i << "\n"; break;
		default: break;
	}
}

void print_map(CMap *map) {
	std::cout << "\nprinting entire map\nlen: " << map->vec.len << "\n";
	for( size_t i=0; i<map->vec.len; i++ )
		print_entry(map_idx_get(map, i));
	
	std::cout << "end of map printing\n\n";
}

int main() {
	CMap *map = new_map();
	map_insert(map, "a", CellEntry, (union MapEntryData){1});
	map_insert(map, "b", CellEntry, (union MapEntryData){200});
	map_insert(map, "c", CellEntry, (union MapEntryData){3});
	print_entry(map_key_get(map, "a"));
	
	MapEntryData str = entry_data_from_array(( uint8_t* )"kektus", sizeof(char), sizeof "kektus" - 1, true);
	map_insert(map, "d", StrEntry, str);

	map_insert(map, "e", CellEntry, (union MapEntryData){111});
	map_insert(map, "f", CellEntry, (union MapEntryData){222});
	map_insert(map, "g", CellEntry, (union MapEntryData){333});
	map_insert(map, "h", CellEntry, (union MapEntryData){444});
	map_insert(map, "i", CellEntry, (union MapEntryData){555});
	map_insert(map, "j", CellEntry, (union MapEntryData){666});
	map_insert(map, "k", CellEntry, (union MapEntryData){777});
	print_map(map);

	print_entry(map_key_get(map, "a"));
	print_entry(map_key_get(map, "i"));
	
	print_entry(map_idx_get(map, 4));
	map_key_rm(map, "b");
	map_key_rm(map, "e");
	map_key_rm(map, "h");
	map_key_rm(map, "j");
	print_map(map);
	
	map_idx_rm(map, 0);
	print_map(map);
	map_idx_rm(map, 0);
	print_map(map);
	map_idx_rm(map, 0);
	print_map(map);
	map_idx_rm(map, 0);
	print_map(map);
	map_idx_rm(map, 0);
	print_map(map);
	map_idx_rm(map, 0);
	print_map(map);
	map_idx_rm(map, 0);
	print_map(map);
	map_idx_rm(map, 0);
	print_map(map);
	map_idx_rm(map, 0);
	print_map(map);

	map_free(&map);
}
