#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Piece.h"
using namespace sf;

class ChessGraphics
{
private:
	static const Piece PIECE;
	Texture board, pieces;
public:
	unsigned int squareRes;
	Sprite boardSprite;
	Sprite pieceSprites[12];
	ChessGraphics();
	Sprite& getPieceSprite(short piece);
	void setPieceSquare(short piece, unsigned int column, unsigned int row);
	void setPiecePosition(short piece, float x, float y);
};

