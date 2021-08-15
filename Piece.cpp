#include "Piece.h"

short Piece::getType(short piece)
{
	return (piece & typeMask);
}

short Piece::getColor(short piece)
{
	return (piece & colorMask);
}

short Piece::getOppositeColor(short color)
{
	return color == WHITE ? BLACK : WHITE;
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
