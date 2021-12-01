#include "TranspositionTable.h"

std::unordered_map<unsigned long long, TableEntry> TranspositionTable::hashTable;
unsigned long long TranspositionTable::MAX_BUCKETS;
const unsigned int TranspositionTable::maxMB = 16000;

void TranspositionTable::add(unsigned long long z, Move m, int e, TableEntry::scoreType t, unsigned int d) {
	return;
	TableEntry entry(z, m, e, t, d);
	unsigned long long key = z % hashTable.max_size();

	/*std::cout << "Max table size in byte: "
		<< std::to_string(hashTable.max_size()) << " * ("
		<< std::to_string(sizeof(unsigned long long)) << " + "
		<< std::to_string(sizeof(TableEntry)) << ") = "
		<< std::to_string(hashTable.max_size() * (sizeof(unsigned long long) + sizeof(TableEntry))) << '\n';
	*/

	if (hashTable.size() >= MAX_BUCKETS) {
		hashTable.clear();
	}

	if (hashTable.insert(std::make_pair(key, entry)).second) {
		// Insertion worked
		//DEBUG_COUT("Inserted! Transposition Table size: " + std::to_string(hashTable.size()) + ", Max size: " + std::to_string(hashTable.max_size())
		//+ ", Bucket count: " + std::to_string(hashTable.bucket_count()) + ", Max Buckets: " + std::to_string(hashTable.max_bucket_count())
		//+ " (Load factor " + std::to_string(hashTable.max_load_factor()) + ")\n"
		//+ " Table size in MB: " + std::to_string(sizeof(hashTable)) + '\n');
	}
	else {
		// Collision, do stuff...
		//DEBUG_COUT("Insertion failed...");
		TableEntry* currentEntry = &hashTable[key];

		if (currentEntry->zobristKey != entry.zobristKey) {
			// Different zobrist keys map to same value
			//DEBUG_CERR("Collision!");
			if ((t == TableEntry::scoreType::EXACT) && d >= currentEntry->depth) {
				// Replace old entry if new one is exact and has atleast same depth
				hashTable[key] = entry;
			}
		}

		else if (currentEntry->depth < entry.depth ||
			((entry.type == TableEntry::scoreType::EXACT) && (currentEntry->type != TableEntry::scoreType::EXACT))) {
			// Replace current entry
			*currentEntry = entry;
			//DEBUG_COUT("Replaced!");
		}
		//DEBUG_COUT('\n');
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

void TranspositionTable::setSize(unsigned int mb) {
	if (mb > maxMB)
		mb = maxMB;

	// Rough approximation for an upper bound of the bucket size
	const auto BYTE_PER_ENTRY = (sizeof(TableEntry) + sizeof(unsigned long long)) * 1.5f;
	MAX_BUCKETS = (unsigned long long)((1000000 * mb) / BYTE_PER_ENTRY);
}
