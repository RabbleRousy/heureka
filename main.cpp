#include <SFML/Graphics.hpp>
#include <iostream>
#include "ChessGraphics.h"
#include "Board.h"

using namespace sf;

// Called when resize event fired
void keepAspectRatio(Window& window) {
	// Set screen size
	float screenWidth = 1160.0f;
	float screenHeight = 1160.0f;
	// Get the resized size
	sf::Vector2u size = window.getSize();
	// Setup desired aspect ratio
	float  heightRatio = screenHeight / screenWidth;
	float  widthRatio = screenWidth / screenHeight;
	// Adapt the resized window to desired aspect ratio
	if (size.y * widthRatio <= size.x)
	{
		size.x = size.y * widthRatio;
	}
	else if (size.x * heightRatio <= size.y)
	{
		size.y = size.x * heightRatio;
	}
	window.setSize(size);
}

int main() {
	// Size is size of the board sprite
	// TODO: scale board to window size
	RenderWindow window(VideoMode(1160, 1160), "Chess");
	ChessGraphics graphics(window.getSize().x);
	bool pieceSelected = false;
	unsigned short selectedSquare[2];
	Vector2i mousePos;

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
			else if (e.type == Event::Resized) {
				keepAspectRatio(window);
				graphics.setWindowRes(window.getSize().x);
			}

			else if (e.type == Event::MouseButtonPressed || e.type == Event::MouseButtonReleased || Mouse::isButtonPressed(Mouse::Button::Left)) {
				// Update mouse position and hovered Square
				mousePos = Mouse::getPosition(window);
				
				unsigned short clickedSquare[2];
				//graphics.getSquareAt(mousePos.x, mousePos.y, clickedSquare[0], clickedSquare[1]);
				clickedSquare[0] = (short)(mousePos.x / (window.getSize().x / 8));
				clickedSquare[1] = 7 - (short)(mousePos.y / (window.getSize().y / 8));
				
				// On mouse down
				if (e.type == Event::MouseButtonPressed) {
					//std::cout << "Mouse down at " << mousePos.x << "/" << mousePos.y << std::endl;
					// If a piece is currently selected, try move and unselect
					if (pieceSelected && board.tryMakeMove(selectedSquare, clickedSquare)) {
						pieceSelected = false;
					}
					// Try to select a hovered piece
					else if (board.getPiece(clickedSquare[0], clickedSquare[1]) != Piece().NONE) {
						pieceSelected = true;
						selectedSquare[0] = clickedSquare[0];
						selectedSquare[1] = clickedSquare[1];
						std::cout << "Selected piece at square " << selectedSquare[0] << "/" << selectedSquare[1] << std::endl;
					}
				}
				// On mouse released
				else if (e.type == Event::MouseButtonReleased) {
					//std::cout << "Mouse released at " << mousePos.x << "/" << mousePos.y << std::endl;
					// If there was a selected piece, try to place it
					if (pieceSelected) {
						if (board.tryMakeMove(selectedSquare, clickedSquare)) {
							pieceSelected = false;
						}
					}
				}
			}
		}

		window.clear();
		window.draw(graphics.boardSprite);

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				short p = board.getPiece(i, j);
				if (p != Piece().NONE) {
					bool dragging = false;
					if (pieceSelected) {
						if (selectedSquare[0] == i && selectedSquare[1] == j) {
							float squareWidth = window.getSize().x / 8.0f;
							// Highlight selected square
							RectangleShape highlightSquare(Vector2f(squareWidth, squareWidth));
							highlightSquare.setFillColor(Color(255, 204, 65, 100));
							highlightSquare.setPosition(i * squareWidth, (7 - j) * squareWidth);
							window.draw(highlightSquare);

							// Drag piece if mouse is held down
							dragging = Mouse::isButtonPressed(Mouse::Button::Left);
						}
					}

					if (dragging)
						graphics.setPiecePosition(p, mousePos.x, mousePos.y);
					else
						graphics.setPieceSquare(p, i, j);

					window.draw(graphics.getPieceSprite(p));
				}
			}
		}
		window.display();
	}
	return 0;
}