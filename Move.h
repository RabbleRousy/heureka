#pragma once
#include <string>

#define UP 0b0001
#define DOWN 0b0100
#define RIGHT 0b0010
#define LEFT 0b1000


// Represents a move on the board

/// <summary>
/// Struct to handle moves on the board
/// </summary>
struct Move {
	// The piece that moved
	short piece;

	// Captured piece, Piece::NONE if no capture
	short capturedPiece;

	// Square where the move starts from
	unsigned short startSquare;

	// Square where the move ends
	unsigned short targetSquare;

	enum Promotion {
		ToQueen = 0b001, ToRook = 0b010, ToBishop = 0b011, ToKnight = 0b100
	};

	// Last 3 bits: Promotion type, 4th bit: en passant flag
	short flags;

	short previousCastlerights;
	unsigned short previousEPsquare;

	// Default constructor, creates Move with all values 0
	Move();

	// Standard constructor
	Move(short piece, short capture, unsigned short start, unsigned short target, short flags = 0);

	// All the directions a knight may go
	// Each direction is a combination of 2 4bit steps
	static short knightMoves[8];

	// All the directions a bishop may go
	// Each direction is a diagonal step
	static short bishopDirections[4];

	// All the directions a rook may go
	// Each direction is a horizontal or vertical step
	static short rookDirections[4];

	// Converts move to string
	static std::string toString(Move m);

	// Returns wether the en passant flag is set
	bool isEnPassant() const;
	
	/// <returns>wether one of the promotion flags is set.</returns>
	bool isPromotion() const;

	/// <returns>the piece that this move promotes to. If move is not a promotion, returns this->piece.</returns>
	short getPromotionResult() const;
};

