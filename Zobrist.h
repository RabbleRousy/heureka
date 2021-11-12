#pragma once
#include "Bitboard.h"

class Zobrist {
private:
	static unsigned long long castleHashes[16];
	// 65 values because square 64 means there is no ep square
	static unsigned long long epHashes[65];

public:
	static void initializeHashes();
	static unsigned long long getZobristKey(const Bitboard* bitboard, short castleRights, unsigned short epSquare);
	static void updateZobristKey(unsigned long long &oldHash, bitboard oldBB, bitboard newBB);
	static void updateZobristKey(unsigned long long &oldHash, short oldCastle, short newCastle);
	static void updateZobristKey(unsigned long long &oldHash, unsigned short oldEP, unsigned short newEP);
};
