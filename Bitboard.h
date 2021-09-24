#pragma once
#include "Piece.h"
#include <iostream>
#include <random>

typedef unsigned __int64 bitboard;
#define C64(constantU64) constantU64##ULL

class Bitboard
{
private:
	std::mt19937_64 randomBitboardGenerator;

	bitboard allPieces[23];

	bitboard bishopMasks[64];
	bitboard rookMasks[64];
	unsigned short bitsInBishopMask[64];
	unsigned short bitsInRookMask[64];
	unsigned long long rookMagics[64];
	unsigned long long bishopMagics[64];

	bitboard knightAttacks[64];
	bitboard kingAttacks[64];
	bitboard* bishopAttacks[64];
	bitboard* rookAttacks[64];

	void initKnightAttacks();
	void initKingAttacks();
	void initBishopMasks();
	void initRookMasks();

	bitboard getOccupancy(int index, bitboard blockerMask);
	bitboard scanRookDirections(unsigned short pos, bitboard blockers);
	bitboard scanBishopDirections(unsigned short pos, bitboard blockers);
	unsigned long long getMagicNumberCandidate();
	unsigned long long findMagicNumber(unsigned short pos, bool forRook);
	void initMagicNumbers();

	int shittyHash(bitboard occupancy, unsigned long long magicNumber, unsigned short bitCount);

public:
	const bitboard notAfile = ~(0x0101010101010101);
	const bitboard notBfile = ~(0x0202020202020202);
	const bitboard notGfile = ~(0x4040404040404040);
	const bitboard notHfile = ~(0x8080808080808080);
	const bitboard notFirstRank = ~(0x00000000000000FF);
	const bitboard notEightRank = ~(0xFF00000000000000);

	Bitboard();
	~Bitboard();
	bitboard getBitboard(short p);
	void setPiece(short p, unsigned short index);
	void removePiece(short p, unsigned short index);
	bitboard getOccupied();
	bitboard getEmpty();
	bitboard getAllAttacks(short color);
	bitboard getKnightAttacks(unsigned short pos);
	bitboard getKingAttacks(unsigned short pos);
	bitboard getRookAttacks(unsigned short pos);
	bitboard getBishopAttacks(unsigned short pos);
	bitboard getQueenAttacks(unsigned short pos);
	bool containsSquare(bitboard b, unsigned short square);
	unsigned short pop(bitboard* b);
	unsigned short count(bitboard b);
	void set(bitboard* b, unsigned short bit);
	std::string toString(bitboard b);
};

