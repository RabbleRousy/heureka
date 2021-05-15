#pragma once
#include <string>

// Represents a move on the board
struct Move {
	// The piece that moved
	short piece;

	// Captured piece, Piece::NONE if no capture
	short capturedPiece;

	// Square where the move starts from
	unsigned short startSquare[2];

	// Steps from startsquare
	// Each step is a 4 bit code, and the integer holds up to 8 steps (first is right)
	int steps;

	enum Promotion {
		ToQueen, ToRook, ToBishop, ToKnight
	};

	// Last 3 bits: Promotion type, 4th bit: en passant flag
	short flags;

	// Default constructor, creates Move with all values 0
	Move();

	// Standard constructor
	Move(short piece, short capture, unsigned short startX, unsigned short startY, int steps, short flags = 0);

	// All the directions a knight may go
	// Each direction is a combination of 2 4bit steps
	static const int knightMoves[8];

	// All the directions a bishop may go
	// Each direction is a diagonal step
	static short bishopDirections[4];

	// All the directions a rook may go
	// Each direction is a horizontal or vertical step
	static short rookDirections[4];

	// Converts move to string
	static std::string toString(Move m);

	// Returns wether the en passant flag is set
	bool isEnPassant();
	
	// Returns wether one of the promotion flags is set
	bool isPromotion();
};

