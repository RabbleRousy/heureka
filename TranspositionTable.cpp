#include "TranspositionTable.h"

std::unordered_map<unsigned long long, TableEntry> TranspositionTable::hashTable;

void TranspositionTable::add(unsigned long long z, Move m, int e, TableEntry::scoreType t, unsigned int d) {
	TableEntry entry(z, m, e, t, d);
	unsigned long long key = z % hashTable.max_size();

	if (hashTable.insert(std::make_pair(key, entry)).second) {
		// Insertion worked
		//DEBUG_COUT("Inserted! Transposition Table size: " + std::to_string(hashTable.size()) + ", Max size: " + std::to_string(hashTable.max_size()) + '\n');
	}
	else {
		// Collision, do stuff...
		DEBUG_COUT("Insertion failed...");
		TableEntry* currentEntry = &hashTable[key];

		if (currentEntry->zobristKey != entry.zobristKey) {
			// Different zobrist keys map to same value
			DEBUG_CERR("Collision!");
			if ((t == TableEntry::scoreType::EXACT) && d >= currentEntry->depth) {
				// Replace old entry if new one is exact and has atleast same depth
				hashTable[key] = entry;
			}
		}

		else if (currentEntry->depth < entry.depth ||
			((entry.type == TableEntry::scoreType::EXACT) && (currentEntry->type != TableEntry::scoreType::EXACT))) {
			// Replace current entry
			*currentEntry = entry;
			DEBUG_COUT("Replaced!");
		}
		DEBUG_COUT('\n');
	}
}

TableEntry* TranspositionTable::get(unsigned long long zobristKey) {
	unsigned long long key = zobristKey % hashTable.max_size();
	if (hashTable.count(key)) {
		TableEntry* entry = &hashTable[key];
		if (entry->zobristKey == zobristKey)
			return entry;
	}
	return nullptr;
}

void TranspositionTable::clear() {
	hashTable.clear();
}
