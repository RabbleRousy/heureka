#pragma once
#include "Move.h"
#include "Board.h"
#include <iostream>
#include <unordered_map>


struct TableEntry {
	enum scoreType
	{
		EXACT, LOWER_BOUND, UPPER_BOUND
	};

	unsigned long long zobristKey;
	Move bestMove;
	int evaluation;
	scoreType type;
	unsigned int depth;

	TableEntry() : zobristKey(0), bestMove(Move::NULLMOVE), evaluation(0), type(EXACT), depth(0) {}
	TableEntry(unsigned long long z, Move m, int e, scoreType t, unsigned int d) : zobristKey(z), bestMove(m), evaluation(e), type(t), depth(d) {}
};

class TranspositionTable {
private:
	static std::unordered_map<unsigned long long, TableEntry> hashTable;
	static unsigned int MAX_BUCKETS;

public:
	static void add(unsigned long long z, Move m, int e, TableEntry::scoreType t, unsigned int d);
	static TableEntry* get(unsigned long long zobristKey);
	static void clear();
	static void setSize(unsigned int mb);
};

