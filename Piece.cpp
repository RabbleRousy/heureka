#include "Piece.h"

short Piece::getType(short piece)
{
	return (piece & typeMask);
}

short Piece::getColor(short piece)
{
	return (piece & colorMask);
}

short Piece::getOppositeColor(short piece) {
	switch (piece) {
	case BLACK: return WHITE;
	case WHITE: return BLACK;
	default:
		return getOppositeColor(getColor(piece));
	}
}

std::string Piece::name(short piece) {
	std::string color = (getColor(piece) == WHITE) ? "White" : "Black";
	std::string type;
	switch (getType(piece))
	{
	case KING: type = "King";
		break;
	case PAWN: type = "Pawn";
		break;
	case KNIGHT: type = "Knight";
		break;
	case BISHOP: type = "Bishop";
		break;
	case ROOK: type = "Rook";
		break;
	case QUEEN: type = "Queen";
		break;
	default:
		type = "";
	}
	return color + " " + type;
}

char Piece::toChar(short piece) {
	bool white = getColor(piece) == WHITE;
	switch (getType(piece)) {
	case KING: return (white ? 'K' : 'k');
		break;
	case PAWN: return (white ? 'P' : 'p');
		break;
	case KNIGHT: return (white ? 'N' : 'n');
		break;
	case BISHOP: return (white ? 'B' : 'b');
		break;
	case ROOK: return (white ? 'R' : 'r');
		break;
	case QUEEN: return (white ? 'Q' : 'q');
		break;
	default:
		return '0';
	}
}

float Piece::getPieceValue(short piece) {
	switch (getType(piece)) {
	case NONE:
		return 0.0f;
	case PAWN:
		return 1.0f;
	case ROOK:
		return 5.0f;
	case BISHOP:
		return 3.2f;
	case KNIGHT:
		return 3.1f;
	case QUEEN:
		return 9.0f;
	default:
		return 0.0f;
	}
}
