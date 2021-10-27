#pragma once
#include <string>
class Piece
{
private:
	static const short colorMask = 0b11000;
	static const short typeMask = 0b00111;

public:
	static const short NONE = 0;
	static const short KING = 0b001;
	static const short PAWN = 0b010;
	static const short KNIGHT = 0b011;
	static const short BISHOP = 0b100;
	static const short ROOK = 0b101;
	static const short QUEEN = 0b110;

	static const short WHITE = 0b01000;
	static const short BLACK = 0b10000;

	static short getType(short piece);
	static short getColor(short piece);
	static short getOppositeColor(short color);
	static std::string name(short piece);
	static char toChar(short piece);
	static int getPieceValue(short piece);
};

