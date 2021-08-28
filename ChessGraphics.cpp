#include "ChessGraphics.h"
#include "Board.h"
#include "Timer.h"

using namespace sf;

ChessGraphics::ChessGraphics() : window(VideoMode(1160, 1160), "Chess")
{
	initGraphics();

	initGame();

	mainLoop();
}

void ChessGraphics::initGraphics()
{
	windowRes = window.getSize().x;
	// Board texture is 1160 x 1160 px
	boardTexture.loadFromFile("Sprites/board.png");
	// Pieces texture is 2000 x 667 px
	pieces.loadFromFile("Sprites/pieces.png");
	// One piece sprite is 333 x 333 px
	pieceRes = pieces.getSize().y / 2;
	std::cout << "Piece resolution: " << pieceRes << std::endl;
	squareRes = boardTexture.getSize().y / 8;
	std::cout << "Square resolution: " << squareRes << std::endl;
	const float pieceScale = (float)squareRes / pieceRes;
	std::cout << "Piece scale: " << pieceScale << std::endl;

	for (int i = 0; i < 12; i++) {
		Sprite sprite(pieces, IntRect((i % 6) * pieceRes, (i / 6) * pieceRes, pieceRes, pieceRes));
		sprite.setPosition(i / 2 * squareRes, (i % 2) * squareRes);
		sprite.setScale(pieceScale, pieceScale);
		pieceSprites[i] = sprite;
	}

	boardSprite = Sprite(boardTexture);
}

void ChessGraphics::initGame()
{
	window.draw(boardSprite);
	window.display();

	std::string input;
	std::cout << "Do you want to debug the move generation? Enter Y for yes.\n";
	std::cin >> input;
	debugPossibleMoves = (input == "Y" || input == "y");
	// Clear cin
	std::getline(std::cin, input);

	std::string fen;
	std::cout << "Please enter start position in FEN notation:" << std::endl;
	std::getline(std::cin, fen);

	std::cout << fen << std::endl;

	board = Board(debugPossibleMoves, fen);
}

void ChessGraphics::mainLoop() {
	while (window.isOpen()) {
		// Check for events
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed) {
				window.close();
			}
			else if (e.type == Event::Resized) {
				keepAspectRatio();
			}

			else if (e.type == Event::MouseButtonPressed || e.type == Event::MouseButtonReleased || Mouse::isButtonPressed(Mouse::Button::Left)) {
				// Update mouse position and hovered Square
				mousePos = Mouse::getPosition(window);

				unsigned short clickedSquare[2];
				//getSquareAt(mousePos.x, mousePos.y, clickedSquare[0], clickedSquare[1]);
				clickedSquare[0] = (short)(mousePos.x / (window.getSize().x / 8));
				clickedSquare[1] = 7 - (short)(mousePos.y / (window.getSize().y / 8));

				// On mouse down
				if (e.type == Event::MouseButtonPressed && !board.wantsToPromote) {
					//std::cout << "Mouse down at " << mousePos.x << "/" << mousePos.y << std::endl;
					// If a piece is currently selected, try move and unselect
					if (pieceSelected && board.handleMoveInput(selectedSquare, clickedSquare)) {
						pieceSelected = false;
					}
					// Try to select a hovered piece
					else if (board.getPiece(clickedSquare[0], clickedSquare[1]) != Piece().NONE) {
						pieceSelected = true;
						selectedSquare[0] = clickedSquare[0];
						selectedSquare[1] = clickedSquare[1];
						//std::cout << "Selected piece at square " << selectedSquare[0] << "/" << selectedSquare[1] << std::endl;
					}
				}
				// On mouse released
				else if (e.type == Event::MouseButtonReleased) {
					// Promotion selection
					if (board.wantsToPromote) {
						if (clickedSquare[0] == 3 && clickedSquare[1] == 4) {
							board.handleMoveInput(selectedSquare, clickedSquare, Piece::QUEEN);
						}
						else if (clickedSquare[0] == 4 && clickedSquare[1] == 4) {
							board.handleMoveInput(selectedSquare, clickedSquare, Piece::ROOK);
						}
						else if (clickedSquare[0] == 3 && clickedSquare[1] == 3) {
							board.handleMoveInput(selectedSquare, clickedSquare, Piece::BISHOP);
						}
						else if (clickedSquare[0] == 4 && clickedSquare[1] == 3) {
							board.handleMoveInput(selectedSquare, clickedSquare, Piece::KNIGHT);
						}
					}
					//std::cout << "Mouse released at " << mousePos.x << "/" << mousePos.y << std::endl;
					// If there was a selected piece, try to place it
					else if (pieceSelected) {
						if (board.handleMoveInput(selectedSquare, clickedSquare)) {
							pieceSelected = false;
						}
					}
				}
			}
			else if (Keyboard::isKeyPressed(Keyboard::Key::Left)) {
				if (board.undoLastMove()) {
					board.swapCurrentPlayer();
					board.generateMoves();
				}
			}
			else if (Keyboard::isKeyPressed(Keyboard::Key::Right)) {
				if (board.redoLastMove()) {
					board.swapCurrentPlayer();
					board.generateMoves();
				}
			}
			else if (Keyboard::isKeyPressed(Keyboard::Key::T)) {
				std::cout << "Enter move generation test depth: ";
				int depth;
				std::cin >> depth;
				board.accumulatedGenerationTime = 0.0f;
				Timer timer("Board::testMoveGeneration(" + std::to_string(depth) + ')', NULL);
				int positions = board.testMoveGeneration(depth, true);
				std::cout << "After " << depth << " moves there are " << positions << " positions.\nAccumulated move generation time: "
					<< board.accumulatedGenerationTime << '\n';
			}
		}

		window.clear();
		window.draw(boardSprite);

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				short p = board.getPiece(i, j);
				if (p != Piece::NONE) {
					bool dragging = false;
					if (pieceSelected) {
						if (selectedSquare[0] == i && selectedSquare[1] == j) {
							float squareWidth = window.getSize().x / 8.0f;
							// Highlight selected square
							RectangleShape highlightSquare = getHighlightSquare(i, j);//(Vector2f(squareWidth, squareWidth));
							window.draw(highlightSquare);

							// Drag piece if mouse is held down
							dragging = Mouse::isButtonPressed(Mouse::Button::Left);
						}
					}

					if (dragging)
						setPiecePosition(p, mousePos.x, mousePos.y);
					else
						setPieceSquare(p, i, j);

					window.draw(getPieceSprite(p));
				}
			}
		}

		if (board.wantsToPromote) {
			// White transparency over board
			RectangleShape highlightSquare = getBoardOverlay();

			window.draw(highlightSquare);

			// Display possible promotion pieces
			setPieceSquare(Piece::QUEEN | board.currentPlayer, 3, 4);
			window.draw(getPieceSprite(Piece::QUEEN | board.currentPlayer));

			setPieceSquare(Piece::ROOK | board.currentPlayer, 4, 4);
			window.draw(getPieceSprite(Piece::ROOK | board.currentPlayer));

			setPieceSquare(Piece::BISHOP | board.currentPlayer, 3, 3);
			window.draw(getPieceSprite(Piece::BISHOP | board.currentPlayer));

			setPieceSquare(Piece::KNIGHT | board.currentPlayer, 4, 3);
			window.draw(getPieceSprite(Piece::KNIGHT | board.currentPlayer));
		}

		window.display();
	}
}

Sprite& ChessGraphics::getPieceSprite(short p)
{

	// White sprites first
	short color = (Piece::getColor(p) == Piece::WHITE) ? 0 : 1;
	short type = 0;

	if (Piece::getType(p) == Piece::KING) {
		type = 0;
	}
	else if (Piece::getType(p) == Piece::QUEEN) {
		type = 1;
	}
	else if (Piece::getType(p) == Piece::BISHOP) {
		type = 2;
	}
	else if (Piece::getType(p) == Piece::KNIGHT) {
		type = 3;
	}
	else if (Piece::getType(p) == Piece::ROOK) {
		type = 4;
	}
	else if (Piece::getType(p) == Piece::PAWN) {
		type = 5;
	}
	//std::cout << "Color = " << color << ", Type = " << type << std::endl;
	return pieceSprites[color * 6 + type];
}

void ChessGraphics::setPieceSquare(short piece, unsigned int column, unsigned int row)
{
	if (column > 7 || row > 7) return;
	float x = column * squareRes;
	float y = (7 - row) * squareRes;
	getPieceSprite(piece).setPosition(x, y);
	//std::cout << "Setting " << Piece::name(piece) << " on square [" << column << "][" << row << "], position: " << x << ", " << y << std::endl;
}

void ChessGraphics::setPiecePosition(short piece, float x, float y)
{
	float scale = boardTexture.getSize().x / (float)windowRes;
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
	RectangleShape r = RectangleShape((Vector2f)boardTexture.getSize() / 8.0f);
	r.setFillColor(Color(255, 204, 65, 100));
	float scale = boardTexture.getSize().x / (float)windowRes;
	float x = column * squareRes;
	float y = (7 - row) * squareRes;
	r.setPosition(x, y);
	return r;
}

RectangleShape ChessGraphics::getBoardOverlay()
{
	RectangleShape r = RectangleShape((Vector2f)boardTexture.getSize());
	r.setFillColor(Color(255, 255, 255, 100));
	r.setPosition(0, 0);
	return r;
}

// Called when resize event fired
void ChessGraphics::keepAspectRatio() {
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
	windowRes = window.getSize().x;
}
