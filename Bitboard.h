#pragma once
#include "Piece.h"
#include <iostream>

typedef unsigned __int64 bitboard;
#define C64(constantU64) constantU64##ULL

class Bitboard
{
private:
	bitboard allPieces[23];
	void initKnightAttacks();
	void initKingAttacks();
public:
	const bitboard notAfile = ~(0x0101010101010101);
	const bitboard notBfile = ~(0x0202020202020202);
	const bitboard notGfile = ~(0x4040404040404040);
	const bitboard notHfile = ~(0x8080808080808080);
	const bitboard notFirstRank = ~(0x00000000000000FF);
	const bitboard notEightRank = ~(0xFF00000000000000);
	
	bitboard knightAttacks[64];
	bitboard kingAttacks[64];

	Bitboard();
	bitboard getBitboard(short p);
	void setPiece(short p, unsigned short index);
	void removePiece(short p, unsigned short index);
	bitboard getOccupied();
	bitboard getEmpty();
	bitboard getAllAttacks(short color);
	bitboard getKnightAttacks(unsigned short pos);
	bitboard getKingAttacks(unsigned short pos);
	bool containsSquare(bitboard b, unsigned short square);
	std::string toString(bitboard b);
};

