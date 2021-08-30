#include "Move.h"
#include "Piece.h"
#include "Board.h"

Move::Move() : piece(0), capturedPiece(0), startSquare(0), targetSquare(0), flags(0), previousCastlerights(Board::castleRights), previousEPsquare(Board::enPassantSquare) {};

Move::Move(short p, short capture, unsigned short start, unsigned short target, short f)
	: piece(p), capturedPiece(capture), startSquare(start), targetSquare(target), flags(f), previousCastlerights(Board::castleRights), previousEPsquare(Board::enPassantSquare) {}

std::string Move::toString(Move m)
{
	std::string name = Board::getSquareName(m.startSquare);
	name += Board::getSquareName(m.targetSquare);

	if (m.isPromotion()) {
		switch (m.getPromotionResult()) {
		case (Promotion::ToQueen):
			name += 'q';
			break;
		case (Promotion::ToBishop):
			name += 'b';
			break;
		case (Promotion::ToRook):
			name += 'r';
			break;
		case (Promotion::ToKnight):
			name += 'n';
			break;
		}
	}

	return name;
}

bool Move::isEnPassant() const
{
	return (flags & 0b1000) == 0b1000;
}

bool Move::isPromotion() const
{
	return (flags & 0b111) != 0;
}

short Move::getPromotionResult() const
{
	short color = Piece::getColor(piece);
	switch (flags & 0b0111) {
	case Promotion::ToBishop:
		return Piece::BISHOP | color;
	case Promotion::ToKnight:
		return Piece::KNIGHT | color;
	case Promotion::ToQueen:
		return Piece::QUEEN | color;
	case Promotion::ToRook:
		return Piece::ROOK | color;
	default:
		return piece;
	}
}

short Move::knightMoves[8] = {
		(UP << 4) | UP | RIGHT ,
		(UP << 4) | UP | LEFT,
		(RIGHT << 4) | UP | RIGHT,
		(RIGHT << 4) | DOWN | RIGHT,
		(DOWN << 4) | DOWN | RIGHT,
		(DOWN << 4) | DOWN | LEFT,
		(LEFT << 4) | UP | LEFT,
		(LEFT << 4) | DOWN | LEFT
};

short Move::bishopDirections[4] = {
	UP | RIGHT,
	UP | LEFT,
	DOWN | RIGHT,
	DOWN | LEFT
};

short Move::rookDirections[4] = {
	UP, DOWN, LEFT, RIGHT
};