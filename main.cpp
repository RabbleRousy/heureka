#include <SFML/Graphics.hpp>
#include <iostream>
#include "ChessGraphics.h"
#include "Board.h"

using namespace sf;

int main() {
	// Size is size of the board sprite
	// TODO: scale board to window size
	RenderWindow window(VideoMode(1160, 1160), "Chess");
	ChessGraphics graphics;

	window.draw(graphics.boardSprite);
	window.display();

	std::string fen;
	std::cout << "Please enter start position in FEN notation:" << std::endl;
	std::cin >> fen;

	Board board(fen);

	
	// Main loop
	while (window.isOpen()) {
		// Check for events
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed) {
				window.close();
			}
			else if (e.type == Event::MouseButtonPressed) {
				// If there is a selected piece, move it along

				// If not, try to select a hovered piece
			}
			else if (e.type == Event::MouseButtonReleased) {
				// If there was a selected piece, try to place it
			}
		}

		window.clear();
		window.draw(graphics.boardSprite);

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				short p = board.getPiece(i, j);
				if (p != Piece().NONE) {
					graphics.setPieceSquare(p, i, j);
					window.draw(graphics.getPieceSprite(p));
				}
			}
		}

		window.display();
	}
	return 0;
}