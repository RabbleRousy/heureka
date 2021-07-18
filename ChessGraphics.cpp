#include "ChessGraphics.h"
#include <iostream>

// Constructor loads the textures from Sprites directory
// and initiates the sprites
ChessGraphics::ChessGraphics(unsigned int res)
{
	windowRes = res;
	// Board texture is 1160 x 1160 px
	board.loadFromFile("Sprites/board.png");
	// Pieces texture is 2000 x 667 px
	pieces.loadFromFile("Sprites/pieces.png");
	// One piece sprite is 333 x 333 px
	pieceRes = pieces.getSize().y / 2;
	std::cout << "Piece resolution: " << pieceRes << std::endl;
	squareRes = board.getSize().y / 8;
	std::cout << "Square resolution: " << squareRes << std::endl;
	const float pieceScale = (float)squareRes / pieceRes;
	std::cout << "Piece scale: " << pieceScale << std::endl;

	for (int i = 0; i < 12; i++) {
		Sprite sprite(pieces, IntRect((i % 6) * pieceRes, (i / 6) * pieceRes, pieceRes, pieceRes));
		sprite.setPosition(i / 2 * squareRes, (i % 2) * squareRes);
		sprite.setScale(pieceScale, pieceScale);
		pieceSprites[i] = sprite;
	}

	boardSprite = Sprite(board);
}

void ChessGraphics::setWindowRes(unsigned int res)
{
	windowRes = res;
}

Sprite& ChessGraphics::getPieceSprite(short p)
{

	// White sprites first
	short color = (PIECE.getColor(p) == PIECE.WHITE) ? 0 : 1;
	short type = 0;

	if (PIECE.getType(p) == PIECE.KING) {
		type = 0;
	}
	else if (PIECE.getType(p) == PIECE.QUEEN) {
		type = 1;
	}
	else if (PIECE.getType(p) == PIECE.BISHOP) {
		type = 2;
	}
	else if (PIECE.getType(p) == PIECE.KNIGHT) {
		type = 3;
	}
	else if (PIECE.getType(p) == PIECE.ROOK) {
		type = 4;
	}
	else if (PIECE.getType(p) == PIECE.PAWN) {
		type = 5;
	}
	//std::cout << "Color = " << color << ", Type = " << type << std::endl;
	return pieceSprites[color * 6 + type];
}

// For setting piece position on a specific square
void ChessGraphics::setPieceSquare(short piece, unsigned int column, unsigned int row)
{
	if (column > 7 || row > 7) return;
	float x = column * squareRes;
	float y = (7 - row) * squareRes;
	getPieceSprite(piece).setPosition(x, y);
	//std::cout << "Setting " << PIECE.name(piece) << " on square [" << column << "][" << row << "], position: " << x << ", " << y << std::endl;
}

// For setting piece's screen position directly, e.g. to mouse
void ChessGraphics::setPiecePosition(short piece, float x, float y)
{
	float scale = board.getSize().x / (float)windowRes;
	float offset = (windowRes / 8.0f) * 0.5f;
	x = (x - offset) * scale;
	y = (y - offset) * scale;
	getPieceSprite(piece).setPosition(x, y);
}

void ChessGraphics::getSquareAt(float x, float y, unsigned short& column, unsigned short& row)
{
	column = (short)(x / (windowRes / 8.0f));
	row = 7 - (short)(y / (windowRes / 8.0f));
}

RectangleShape ChessGraphics::getHighlightSquare(unsigned int column, unsigned int row)
{
	RectangleShape r = RectangleShape((Vector2f)board.getSize() / 8.0f);
	r.setFillColor(Color(255, 204, 65, 100));
	float scale = board.getSize().x / (float)windowRes;
	float x = column * squareRes;
	float y = (7 - row) * squareRes;
	r.setPosition(x, y);
	return r;
}

RectangleShape ChessGraphics::getBoardOverlay()
{
	RectangleShape r = RectangleShape((Vector2f)board.getSize());
	r.setFillColor(Color(255, 255, 255, 100));
	r.setPosition(0, 0);
	return r;
}
