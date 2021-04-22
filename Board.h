#pragma once
#include "Piece.h"
#include <vector>

#define UP 0b0001
#define DOWN 0b0100
#define RIGHT 0b0010
#define LEFT 0b1000

// Represents a move on the board
struct Move {
	// Square where the move starts from
	unsigned short startSquare[2];

	// Steps from startsquare
	// Each step is a 4 bit code, and the integer holds up to 8 steps (first is right)
	int steps;

	Move(unsigned short startX, unsigned short startY, int steps);

	// All the directions a knight may go
	// Each direction is a combination of 2 4bit steps
	static const int knightMoves[8];
};

class Board
{
private:
	short squares[8][8];
public:
	Board(std::string fen);
	short currentPlayer;
	std::vector<Move> possibleMoves;
	std::vector<Move> moveHistory;
	void clearBoard();
	bool readPosFromFEN(std::string fen);
	std::string getFENfromPos();
	short getPiece(unsigned short column, unsigned short row);
	void setPiece(unsigned short column, unsigned short row, short piece);
	void removePiece(unsigned short column, unsigned short row);
	bool tryMakeMove(const unsigned short from[2], const unsigned short to[2]);
	void generateMoves();
	short* stepsToDirection(int steps);
	void swapCurrentPlayer();
	std::string squareName(unsigned short column, unsigned short row);
};

