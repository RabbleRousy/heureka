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
	std::string color;
	switch (getColor(piece)) {
	case WHITE: color = "White";
		break;
	case BLACK: color = "Black";
		break;
	default: color = "";
	}

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

int Piece::getPieceValue(short piece) {
	switch (getType(piece)) {
	case NONE:
		return 0;
	case PAWN:
		return 100;
	case ROOK:
		return 500;
	case BISHOP:
		return 320;
	case KNIGHT:
		return 310;
	case QUEEN:
		return 900;
	default:
		return 0;
	}
}
