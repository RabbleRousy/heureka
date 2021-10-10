#pragma once
#include "Piece.h"
#include <iostream>
#include <fstream>
#include <random>

typedef unsigned __int64 bitboard;
#define C64(constantU64) constantU64##ULL

class Bitboard
{
private:
	std::mt19937_64 randomBitboardGenerator;

	bitboard allPieces[23];
	unsigned short whiteKingPos, blackKingPos;

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

	bitboard connectingRays[64][64];

	void initConnectingRays();
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
	void writeMagicNumbers();
	void readMagicNumbers();
	void fillAttackTables();

	int shittyHash(bitboard occupancy, unsigned long long magicNumber, unsigned short bitCount);

	bool attacksNeedRebuilding;
	bitboard allAttacks;
	bitboard pinsOnWhiteKing[8], pinsOnBlackKing[8];
	bitboard checksOnWhiteKing[2], checksOnBlackKing[2];

public:
	const bitboard notAfile = ~(0x0101010101010101);
	const bitboard notBfile = ~(0x0202020202020202);
	const bitboard notGfile = ~(0x4040404040404040);
	const bitboard notHfile = ~(0x8080808080808080);
	const bitboard notFirstRank = ~(0x00000000000000FF);
	const bitboard notEightRank = ~(0xFF00000000000000);
	const bitboard secondRank = 0x000000000000FF00;
	const bitboard seventhRank = 0x00FF000000000000;
	// Relevant bits for White's short castle
	const bitboard OO = 0x000000000000060;
	// Relevant bits for White's long castle
	const bitboard OOO = 0x000000000000000E;
	// Relevant bits for Black's short castle
	const bitboard oo = 0x6000000000000000;
	// Relevant bits for Black's long castle
	const bitboard ooo = 0x0E00000000000000;

	bool pinsExist, checkExists, doubleCheck;

	Bitboard();
	~Bitboard();
	/// <param name="p">is a Piece consisting of Type | Color, or only a Color.</param>
	/// <returns>a bitboard containing the current positions of the given piece or all pieces of that color. </returns>
	bitboard getBitboard(short p);
	void setPiece(short p, unsigned short index);
	void removePiece(short p, unsigned short index);
	bitboard getOccupied();
	bitboard getEmpty();
	/// <param name="color">of the attacking side.</param>
	/// <returns>a bitboard with all squares marked that are attacked by any piece of the given color.</returns>
	bitboard getAllAttacks(short color);
	/// <param name="color">of the pawns to generate the step bitboard</param>
	/// <returns>a bitboard with the squares marked that all pawns of that color can reach by stepping one field ahead.</returns>
	bitboard getSinglePawnSteps(short color);
	/// <param name="color">of the pawns to generate the step bitboard</param>
	/// <returns>a bitboard with the squares marked that all pawns of that color can reach by stepping two fields ahead.</returns>
	bitboard getDoublePawnSteps(short color);
	/// <param name="color">of the pawns to generate the attack bitboard</param>
	/// <returns>a bitboard with the squares marked that all pawns of that color are attacking.</returns>
	bitboard getPawnAttacks(bool left, short color);
	/// <param name="pos">of the knight that's attacking.</param>
	/// <returns>a bitboard of the fields the knight is attacking from that position.</returns>
	bitboard getKnightAttacks(unsigned short pos);
	/// <param name="pos">of the king.</param>
	/// <param name="includeCastle">signals wether to include the castling steps in the bitboard.</param>
	/// <returns>a bitboard of the fields the king is attacking from that position.</returns>
	bitboard getKingAttacks(unsigned short pos, bool includeCastle = false);
	/// <param name="pos">of the rook that's attacking.</param>
	/// <returns>a bitboard of the fields the rook is attacking from that position, including possible blocker's squares.</returns>
	bitboard getRookAttacks(unsigned short pos, bitboard blockers);
	/// <param name="pos">of the bishop that's attacking.</param>
	/// <returns>a bitboard of the fields the bishop is attacking from that position, including possible blocker's squares.</returns>
	bitboard getBishopAttacks(unsigned short pos, bitboard blockers);
	/// <param name="pos">of the queen that's attacking.</param>
	/// <returns>a bitboard of the fields the queen is attacking from that position, including possible blocker's squares.</returns>
	bitboard getQueenAttacks(unsigned short pos, bitboard blockers);
	/// <returns>a bitboard where the bits on a straight or diagonal line between from and to are set.
	/// Returns an empty bitboard if there is no connecting ray.</returns>
	bitboard getConnectingRay(unsigned short king, unsigned short attacker, short pieceType);
	void calculateAttacks(short pinnedPiecesColor);
	bitboard isPinned(unsigned short pos, short color);
	bitboard getCheckRays(short playerInCheck);
	/// <returns>wether the given bitboard has the bit for the given square set to 1.</returns>
	bool containsSquare(bitboard b, unsigned short square);
	/// <summary>
	/// Removes the least significant bit from the bitboard.
	/// </summary>
	/// <returns>index of the popped bit.</returns>
	unsigned short pop(bitboard* b);
	/// <returns>number of 1s set in the given bitboard.</returns>
	unsigned short count(bitboard b);
	/// <summary>
	/// Sets the bit on the given bitboard to 1.
	/// </summary>
	void set(bitboard* b, unsigned short bit);
	/// <returns>a string representation of the bitboard.</returns>
	std::string toString(bitboard b);
};

