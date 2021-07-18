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
	unsigned int windowRes;
	unsigned int pieceRes;
	unsigned int squareRes;
public:
	Sprite boardSprite;
	Sprite pieceSprites[12];
	ChessGraphics(unsigned int res);
	void setWindowRes(unsigned int res);
	Sprite& getPieceSprite(short piece);
	void setPieceSquare(short piece, unsigned int column, unsigned int row);
	void setPiecePosition(short piece, float x, float y);
	void getSquareAt(float x, float y, unsigned short& column, unsigned short& row);
	RectangleShape getHighlightSquare(unsigned int column, unsigned int row);
	RectangleShape getBoardOverlay();
};

