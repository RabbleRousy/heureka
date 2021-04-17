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
	short getPiece(unsigned int column, unsigned int row);
	void setPiece(unsigned int column, unsigned int row, short piece);
};

