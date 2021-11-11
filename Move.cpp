#include "Move.h"
#include "Piece.h"
#include "Board.h"

Move::Move() : piece(0), capturedPiece(0), startSquare(0), targetSquare(0), flags(0), score(0.0f), previousCastlerights(0), previousEPsquare(0) {};

Move::Move(short p, short capture, unsigned short start, unsigned short target, short f)
	: piece(p), capturedPiece(capture), startSquare(start), targetSquare(target), flags(f), score(0.0f), previousCastlerights(Board::castleRights), previousEPsquare(Board::enPassantSquare) {
	// Capturing with less valuable pieces is better
	if (Piece::getType(capturedPiece) != Piece::NONE) {
		float myValue = Piece::getPieceValue(p);
		float captureValue = Piece::getPieceValue(capturedPiece);
		if (myValue < captureValue) {
			score = captureValue - myValue;
		}
	}
	// Promoting is good
	if (isPromotion()) {
		score += Piece::getPieceValue(getPromotionResult());
	}
	// Moving to a square attacked by a pawn is probably bad
	if (Piece::getType(p) == Piece::PAWN) return;

	Bitboard* bb = &Board::bb;
	short enemyColor = Piece::getOppositeColor(p);
	bitboard enemyPawns = bb->getBitboard(Piece::PAWN | enemyColor);
	bitboard attackedByEnemyPawns = bb->getPawnAttacks(enemyPawns, true, enemyColor) | bb->getPawnAttacks(enemyPawns, false, enemyColor);
	if (bb->containsSquare(attackedByEnemyPawns, target)) {
		score -= Piece::getPieceValue(p) - Piece::getPieceValue(Piece::PAWN);
	}
}

std::string Move::toString(Move m)
{
	std::string name = Board::getSquareName(m.startSquare);
	name += Board::getSquareName(m.targetSquare);

	if (m.isPromotion()) {
		switch (m.flags & 0b111) {
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

const Move Move::NULLMOVE;

bool Move::isEnPassant() const
{
	return (flags & 0b1000);
}

bool Move::isPromotion() const
{
	return (Piece::getType(piece) == Piece::PAWN) && (flags & 0b111);
}

short Move::getPromotionResult() const
{
	short color = Piece::getColor(piece);
	switch (flags & 0b0111) {
	case Promotion::ToBishop:
		return Piece::BISHOP | color;
		break;
	case Promotion::ToKnight:
		return Piece::KNIGHT | color;
		break;
	case Promotion::ToQueen:
		return Piece::QUEEN | color;
		break;
	case Promotion::ToRook:
		return Piece::ROOK | color;
		break;
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