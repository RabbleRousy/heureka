#include "Piece.h"

short Piece::getType(short piece)
{
	return (piece & typeMask);
}

short Piece::getColor(short piece)
{
	return (piece & colorMask);
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
