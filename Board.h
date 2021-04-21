#pragma once
#include "Piece.h"
class Board
{
private:
	short squares[8][8];
	const Piece PIECE;
public:
	Board(std::string fen);
	void clearBoard();
	bool readPosFromFEN(std::string fen);
	std::string getFENfromPos();
	short getPiece(unsigned short column, unsigned short row);
	void setPiece(unsigned short column, unsigned short row, short piece);
	void removePiece(unsigned short column, unsigned short row);
	bool tryMakeMove(const unsigned short from[2], const unsigned short to[2]);
	std::string squareName(unsigned short column, unsigned short row);
};

