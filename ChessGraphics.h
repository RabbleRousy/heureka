#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Piece.h"
#include "Board.h"
using namespace sf;

// This class manages the board- and piece-sprites, scales and repositions them.
class ChessGraphics
{
private:
	RenderWindow* window;
	Texture boardTexture, pieces;
	unsigned int windowRes;
	unsigned int pieceRes;
	unsigned int squareRes;
	Board board;
	bool pieceSelected;
	unsigned short selectedSquare[2];
	Vector2i mousePos;
	bool debugPossibleMoves;
	bool waitingForBoard;
	float searchTime;
	std::thread searchThread;

public:
	Sprite boardSprite;
	Sprite pieceSprites[12];

	/// <summary>
	/// Constructor loads the textures from Sprites directory and initiates the sprite objects.
	/// Then enters the main loop.
	/// </summary>
	ChessGraphics();

	void initGraphics();

	void initGame();

	void mainLoop();

	void draw();

	void startNewGame();

	/// <param name="piece">indicates which sprite is needed.</param>
	/// <returns>the sprite by reference so it may be drawn to the screen.</returns>
	Sprite& getPieceSprite(short piece);

	/// <summary>
	/// Sets the sprite of the desired piece to the scaled position of that square.
	/// </summary>
	/// <param name="piece"> to reposition.</param>
	/// <param name="column">from 0 to 7 (a to h).</param>
	/// <param name="row">from 0 to 7 (1 to 8).</param>
	void setPieceSquare(short piece, unsigned int column, unsigned int row);

	/// <summary>
	/// Sets the sprite of the desired piece to a scaled position on a board (does not have to be on a square).
	/// </summary>
	/// <param name="piece">to reposition.</param>
	/// <param name="x">position in window space.</param>
	/// <param name="y">position in window space.</param>
	void setPiecePosition(short piece, float x, float y);

	/// <summary>
	/// Returns the board coordinates to a given window position.
	/// </summary>
	/// <param name="x">position in window space.</param>
	/// <param name="y">position in window space.</param>
	/// <param name="column">from 0 to 7 (a to h).</param>
	/// <param name="row">from 0 to 7 (1 to 8).</param>
	void getSquareAt(float x, float y, unsigned short& column, unsigned short& row);

	/// <param name="column">from 0 to 7 (a to h).</param>
	/// <param name="row">from 0 to 7 (1 to 8).</param>
	/// <returns>a scaled highlightsquare at the position of the desired square.</returns>
	RectangleShape getHighlightSquare(unsigned int column, unsigned int row);

	/// <returns>a white rectangle overlay that is scaled to fit the whole board sprite.</returns>
	RectangleShape getBoardOverlay();

	void keepAspectRatio();
};

