#pragma once
#include "Piece.h"

typedef unsigned __int64 bitboard;
#define C64(constantU64) constantU64##ULL

class Bitboard
{
private:
	bitboard allPieces[23];
public:
	bitboard getBitboard(short p);
	void setPiece(short p, short column, short row);
	void removePiece(short p, short column, short row);
	bitboard getOccupied();
	bitboard getEmpty();
	std::string toString(bitboard b);
};

