#pragma once
#include <string>


// Represents a move on the board

/// <summary>
/// Struct to handle moves on the board
/// </summary>
struct Move {
	enum Promotion {
		None = 0, ToQueen = 0b001, ToRook = 0b010, ToBishop = 0b011, ToKnight = 0b100
	};

	static const Move NULLMOVE;

	// The piece that moved
	short piece;

	// Captured piece, Piece::NONE if no capture
	short capturedPiece;

	// Square where the move starts from
	unsigned short startSquare;

	// Square where the move ends
	unsigned short targetSquare;

	// Last 3 bits: Promotion type, 4th bit: en passant flag
	short flags;

	short previousCastlerights;
	unsigned short previousEPsquare;
	unsigned short previousHalfMoves;

	float score;

	// Default constructor, creates Move with all values 0
	Move();

	// Standard constructor
	Move(short piece, short capture, unsigned short start, unsigned short target, short flags = 0);

	// Converts move to string
	static std::string toString(Move m);

	// Returns wether the en passant flag is set
	bool isEnPassant() const;
	
	/// <returns>wether one of the promotion flags is set.</returns>
	bool isPromotion() const;

	/// <returns>the piece that this move promotes to. If move is not a promotion, returns this->piece.</returns>
	short getPromotionResult() const;

	bool operator==(const Move& other) {
		return (this->piece == other.piece) && (this->capturedPiece == other.piece)
			&& (this->startSquare == other.startSquare) && (this->targetSquare == other.targetSquare)
			&& (this->flags == other.flags) && (this->previousCastlerights == other.previousCastlerights)
			&& (this->previousEPsquare == other.previousEPsquare) && (this->score == other.score);
	};

	bool operator!=(const Move& other) {
		return (this->piece != other.piece) || (this->capturedPiece != other.piece)
			|| (this->startSquare != other.startSquare) || (this->targetSquare != other.targetSquare)
			|| (this->flags != other.flags) || (this->previousCastlerights != other.previousCastlerights)
			|| (this->previousEPsquare != other.previousEPsquare) || (this->score != other.score);
	};
};

