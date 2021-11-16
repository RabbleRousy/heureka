#pragma once
#include "Bitboard.h"

class Zobrist {
private:
	// 23 is a random artifact from the way pieces are stored, might change
	static unsigned long long pieceHashes[23][64];
	static unsigned long long castleHashes[16];
	// 65 values because square 64 means there is no ep square
	static unsigned long long epHashes[65];
	static unsigned long long whiteToMoveHash;

public:
	static void initializeHashes();
	static unsigned long long getZobristKey(const Bitboard* bitboard, short castleRights, unsigned short epSquare, bool whiteToMove);
	/// <summary>
	/// Either adds a piece at the given position to the hash or removes it.
	/// </summary>
	/// <param name="oldHash"> is the zobrist key to be modified.</param>
	/// <param name="piece"> that changed.</param>
	/// <param name="pos"> where it appears/disappears.</param>
	static void updatePieceHash(unsigned long long &oldHash, short piece, unsigned short pos);
	static void swapPlayerHash(unsigned long long& oldHash);
	static void updateZobristKey(unsigned long long &oldHash, short oldCastle, short newCastle);
	static void updateZobristKey(unsigned long long &oldHash, unsigned short oldEP, unsigned short newEP);
};
