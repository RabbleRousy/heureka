#include <SFML/Graphics.hpp>

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
	Texture board, blackPawn, whitePawn, blackRook, whiteRook,
		blackKnight, whiteKnight, blackBishop, whiteBishop,
		blackQueen, whiteQueen, blackKing, whiteKing;

	board.loadFromFile("Sprites/board.png");
	blackPawn.loadFromFile("Sprites/blackPawn.png");
	whitePawn.loadFromFile("Sprites/whitePawn.png");
	blackRook.loadFromFile("Sprites/blackRook.png");
	whiteRook.loadFromFile("Sprites/whiteRook.png");
	blackKnight.loadFromFile("Sprites/blackKnight.png");
	whiteKnight.loadFromFile("Sprites/whiteKnight.png");
	blackBishop.loadFromFile("Sprites/blackBishop.png");
	whiteBishop.loadFromFile("Sprites/whiteBishop.png");
	blackQueen.loadFromFile("Sprites/blackQueen.png");
	whiteQueen.loadFromFile("Sprites/whiteQueen.png");
	blackKing.loadFromFile("Sprites/blackKing.png");
	whiteKing.loadFromFile("Sprites/whiteKing.png");

	Sprite Board(board);
	Sprite BlackPawn(blackPawn);
	Sprite WhitePawn(whitePawn);
	Sprite BlackRook(blackRook);
	Sprite WhiteRook(whiteRook);
	Sprite BlackKnight(blackKnight);
	Sprite WhiteKnight(whiteKnight);
	Sprite BlackBishop(blackBishop);
	Sprite WhiteBishop(whiteBishop);
	Sprite BlackQueen(blackQueen);
	Sprite WhiteQueen(whiteQueen);
	Sprite BlackKing(blackKing);
	Sprite WhiteKing(whiteKing);


	
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
		window.display();
	}
	return 0;
}