#include "Move.h"
#include "Piece.h"
#include "Board.h"

Move::Move(short p, short capture, unsigned short startX, unsigned short startY, int s, bool ep)
	: piece(p), capturedPiece(capture), steps(s), enpassant(ep)
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

const int Move::knightMoves[8] = {
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