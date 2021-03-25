#include <SFML/Graphics.hpp>
#include <iostream>

#define LENGTH 7
#define BLACK_PAWN -1
#define WHITE_PAWN 1
#define BLACK_ROOK -2
#define WHITE_ROOK 2
#define BLACK_KNIGHT -3
#define WHITE_KNIGHT 3
#define BLACK_BISHOP -4
#define WHITE_BISHOP 4
#define BLACK_QUEEN -5
#define WHITE_QUEEN 5
#define BLACK_KING -6
#define WHITE_KING 6

using namespace sf;

struct pos {
	int x, y;
} oldPos, whiteKing, blackKing, promotionPosWhite, promotionPosBlack;

int size = 100;
int isMoving;
int board[8][8] = {
	-2,-3,-4,-5,-6,-4,-3,-2,
	-1,-1,-1,-1,-1,-1,-1,-1,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 1, 1, 1, 1, 1, 1, 1, 1,
	 2, 3, 4, 5, 6, 4, 3, 2
};

int whiteOO, whiteOOO, whiteStart;
int blackOO, blackOOO, blackStart;

int move; // 0 = white, 1 = black

int whiteCheck, blackCheck;
int promotionWhite, promotionBlack;

int main() {
	// Size is size of the board sprite
	// TODO: scale board to window size
	RenderWindow window(VideoMode(1160, 1160), "Chess");
	Texture board, pieces;

	// Board texture is 1160 x 1160 px
	board.loadFromFile("Sprites/board.png");
	// Pieces texture is 2000 x 667 px
	pieces.loadFromFile("Sprites/pieces.png");
	// One piece sprite is 333 x 333 px
	const int pieceRes = pieces.getSize().y / 2;
	std::cout << "Piece resolution: " << pieceRes << std::endl;
	const int squareRes = board.getSize().y / 8;
	std::cout << "Square resolution: " << squareRes << std::endl;
	const float pieceScale = (float)squareRes / pieceRes;
	std::cout << "Piece scale: " << pieceScale << std::endl;
	

	Sprite pieceSprites[12];
	for (int i = 0; i < 12; i++) {
		Sprite sprite(pieces, IntRect(i/2 * pieceRes, (i % 2) * pieceRes, pieceRes, pieceRes));
		sprite.setPosition(i/2 * squareRes, (i % 2) * squareRes);
		sprite.setScale(pieceScale, pieceScale);
		pieceSprites[i] = sprite;
	}

	Sprite Board(board);


	
	// Main loop
	while (window.isOpen()) {
		// Check for events
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed) {
				window.close();
			}
		}

		window.clear();
		window.draw(Board);

		for (int i = 0; i < 12; i++) {
			window.draw(pieceSprites[i]);
		}

		window.display();
	}
	return 0;
}