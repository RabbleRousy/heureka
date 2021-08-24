#include "Move.h"
#include "Piece.h"
#include "Board.h"

Move::Move() : piece(0), capturedPiece(0), steps(0), flags(0)
{
	startSquare[0] = 0;
	startSquare[1] = 0;
}

Move::Move(short p, short capture, unsigned short startX, unsigned short startY, int s, short castle, short f)
	: piece(p), capturedPiece(capture), steps(s), flags(f), previousCastlerights(castle)
{
	startSquare[0] = startX;
	startSquare[1] = startY;
}

std::string Move::toString(Move m)
{
	std::string name = Board::squareName(m.startSquare[0], m.startSquare[1]);
	short dir[2];
	Board::stepsToDirection(m.steps, dir);
	name += Board::squareName(m.startSquare[0] + dir[0], m.startSquare[1] + dir[1]);

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