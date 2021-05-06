#pragma once
#include "Piece.h"
#include <vector>
#include <stack>

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

	// Wether it was an en passant capture
	bool enpassant;

	Move(unsigned short startX, unsigned short startY, int steps, bool enpassant = false);

	// All the directions a knight may go
	// Each direction is a combination of 2 4bit steps
	static const int knightMoves[8];

	// All the directions a bishop may go
	// Each direction is a diagonal step
	static short bishopDirections[4];

	// All the directions a rook may go
	// Each direction is a horizontal or vertical step
	static short rookDirections[4];
};

class Board
{
private:
	short squares[8][8];
public:
	Board(std::string fen);
	short currentPlayer;
	std::vector<Move> possibleMoves;
	std::stack<Move> moveHistory;
	void clearBoard();
	bool readPosFromFEN(std::string fen);
	std::string getFENfromPos();
	short getPiece(unsigned short column, unsigned short row);
	void setPiece(unsigned short column, unsigned short row, short piece);
	void removePiece(unsigned short column, unsigned short row);
	bool tryMakeMove(const unsigned short from[2], const unsigned short to[2]);
	void generateMoves();
	bool tryAddMove(unsigned short x, unsigned short y, int steps, bool canCapture, short target[2] = NULL);
	void stepsToDirection(int steps, short dir[2]);
	void swapCurrentPlayer();
	std::string squareName(unsigned short column, unsigned short row);
};

