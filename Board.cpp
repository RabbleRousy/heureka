#include "Board.h"
#include "Piece.h"
#include <iostream>
#include <string>

Board::Board(bool m, std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR")
	: currentPlayer(Piece::WHITE), possibleMoves(), moveHistory(), debugPossibleMoves(m)
{
	if (!readPosFromFEN(fen)) {
		std::cout << "Loading default position..." << std::endl;
		readPosFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
	}
	generateMoves();
}

void Board::clearBoard() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			squares[i][j] = Piece::NONE;
		}
	}
}

bool Board::readPosFromFEN(std::string fen) {
	std::cout << "Trying to parse FEN: " << fen << std::endl;
	clearBoard();

	// FEN starts at the top left corner of the board
	int column = 0;
	int row = 7;
	for (unsigned int i = 0; i < fen.length(); i++) {
		// Stop when at the end of the board
		if (row == 0 && column == 8)
			break;

		// Return false for invalid fens that reach out of bounds
		if (row < 0 || (column > 7 && fen[i] != '/')) {
			std::cout << "FEN parsing failed." << std::endl;
			return false;
		}

		// Read the next char
		switch (fen[i])
		{
		case '/':
			// Go to next row
			row--;
			column = 0;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			// Skip next squares
			column += std::atoi(&fen[i]);
			break;
		case 'K':
			squares[column][row] = Piece::KING | Piece::WHITE;
			column++;
			break;
		case 'Q':
			squares[column][row] = Piece::QUEEN | Piece::WHITE;
			column++;
			break;
		case 'R':
			squares[column][row] = Piece::ROOK | Piece::WHITE;
			column++;
			break;
		case 'N':
			squares[column][row] = Piece::KNIGHT | Piece::WHITE;
			column++;
			break;
		case 'B':
			squares[column][row] = Piece::BISHOP | Piece::WHITE;
			column++;
			break;
		case 'P':
			squares[column][row] = Piece::PAWN | Piece::WHITE;
			column++;
			break;
		case 'k':
			squares[column][row] = Piece::KING | Piece::BLACK;
			column++;
			break;
		case 'q':
			squares[column][row] = Piece::QUEEN | Piece::BLACK;
			column++;
			break;
		case 'r':
			squares[column][row] = Piece::ROOK | Piece::BLACK;
			column++;
			break;
		case 'n':
			squares[column][row] = Piece::KNIGHT | Piece::BLACK;
			column++;
			break;
		case 'b':
			squares[column][row] = Piece::BISHOP | Piece::BLACK;
			column++;
			break;
		case 'p':
			squares[column][row] = Piece::PAWN | Piece::BLACK;
			column++;
			break;
		default:
			break;
		}
	}
	// Return true if FEN parsing reached end of board
	return (row == 0 && column == 8);
}

std::string Board::getFENfromPos() {
	return "";
}

// Returns the piece at the desired position (can be NONE)
// Row = bottom to top
// Column = left to right
short Board::getPiece(unsigned short column, unsigned short row)
{
	if (row > 7 || column > 7) return 0;
	return squares[column][row];
}

// Sets the piece at the desired position
// Row = bottom to top
// Column = left to right
void Board::setPiece(unsigned short column, unsigned short row, short p)
{
	if (row > 7 || column > 7) return;
	squares[column][row] = p;
}

void Board::removePiece(unsigned short column, unsigned short row) {
	if (row > 7 || column > 7) return;
	squares[column][row] = Piece::NONE;
}

bool Board::tryMakeMove(const unsigned short from[2], const unsigned short to[2]) {
	if (from[0] == to[0] && from[1] == to[1])
		return false;
	if (from[0] > 7 || from[1] > 7 || to[0] > 7 || to[1] > 7) {
		//std::cerr << "Tried illegal move (" << squareName(from[0], from[1]) << " to " << squareName(to[0], to[1]) << ")\n";
		return false;
	}
	short pieceFrom = getPiece(from[0], from[1]);
	short pieceTo = getPiece(to[0], to[1]);
	if (Piece::getType(pieceFrom) == Piece::NONE) {
		return false;
	}
	for (int i = 0; i < possibleMoves.size(); i++)
	{
		if (possibleMoves[i].startSquare[0] != from[0] || possibleMoves[i].startSquare[1] != from[1])
			continue;
		short dir[2];
		stepsToDirection(possibleMoves[i].steps, dir);
		unsigned short result[2];
		result[0] = from[0] + dir[0];
		result[1] = from[1] + dir[1];
		if (result[0] != to[0] || result[1] != to[1])
			continue;

		//--------- MOVE FOUND ----------------------

		setPiece(to[0], to[1], pieceFrom);
		removePiece(from[0], from[1]);

		if (possibleMoves[i].enpassant) {
			removePiece(from[0] + dir[0], from[1]);
		}

		if (Piece::getType(pieceFrom) == Piece::KING && (dir[0] == 2 || dir[0] == -2) ) {
			// Castle detected, Rook has to be moved
			switch (dir[0])
			{
			case 2: 
				removePiece(7, from[1]);
				setPiece(5, from[1], Piece::ROOK | currentPlayer);
				break;
			case -2:
				removePiece(0, from[1]);
				setPiece(3, from[1], Piece::ROOK | currentPlayer);
				break;
			default:
				break;
			}
		}

		std::cout << ((currentPlayer == Piece::WHITE) ? "White" : "Black") << " played " << Move::toString(possibleMoves[i]) << "\n";

		//----------- UPDATE CASTLE RIGHTS ---------------------
		if (castleRights != 0) {
			if (Piece::getType(pieceFrom) == Piece::KING) {
				castleRights &= (currentPlayer == Piece::WHITE) ? 0b0011 : 0b1100;
			}
			else if (Piece::getType(pieceFrom) == Piece::ROOK) {
				if (currentPlayer == Piece::WHITE && from[1] == 0) {
					if (from[0] == 0) {
						// Remove right for white's long castle
						castleRights &= 0b1011;
					} else if (from[0] == 7) {
						// Remove right for white's short castle
						castleRights &= 0b0111;
					}
				}
				else if (currentPlayer == Piece::BLACK && from[1] == 7) {
					if (from[0] == 0) {
						// Remove right for black's long castle
						castleRights &= 0b1110;
					}
					else if (from[0] == 7) {
						// Remove right for black's short castle
						castleRights &= 0b1101;
					}
				}
			}
		}
		

		moveHistory.push(possibleMoves[i]);
		swapCurrentPlayer();
		generateMoves();
		return true;
	}
	return false;
}

void Board::generateMoves()
{
	possibleMoves.clear();
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			short pieceType = Piece::getType(squares[i][j]);
			short pieceColor = Piece::getColor(squares[i][j]);
			if (pieceType == Piece::NONE || pieceColor != currentPlayer)
				continue;

			//std::cout << "Computing moves for " << Piece::name(squares[i][j]) << " on " << squareName(i, j) << " ...\n";
			//-------------- KNIGHT MOVES -----------------------
			if (pieceType == Piece::KNIGHT) {
				for (int moveIndex = 0; moveIndex < 8; moveIndex++) {
					int steps = Move::knightMoves[moveIndex];
					tryAddMove(i, j, steps, true);
				}
			}
			//-------------- PAWN MOVES -------------------------
			else if (pieceType == Piece::PAWN) {

				//-------- ONE STEP AHEAD ---------------------------
				int step = (pieceColor == Piece::WHITE) ? UP : DOWN;
				
				if (tryAddMove(i, j, step, false)) {
					//-------- ANOTHER STEP AHEAD ------------------------
					// If pawn is still on its start square
					if ((pieceColor == Piece::WHITE && j == 1) || (pieceColor == Piece::BLACK) && j == 6) {
						step <<= 4;
						step |= (pieceColor == Piece::WHITE) ? UP : DOWN;
						tryAddMove(i, j, step, false);
					}
				}
				short dir[2];
				short targetPiece;
				// Clear step from double step
				step &= 0b00001111;
				step <<= 4;
				//-------- CAPTURE LEFT ----------------------
				if (i != 0) {
					step |= LEFT;
					stepsToDirection(step, dir);
					targetPiece = getPiece(i + dir[0], j + dir[1]);
					bool regularCapture = Piece::getType(targetPiece) != Piece::NONE && Piece::getColor(targetPiece) != currentPlayer;
					bool enPassant = false;
					if (!regularCapture) {
						// Check for en passant
						// White on 5th rank or black on 4th rank
						if ((pieceColor == Piece::WHITE && j == 4) || (pieceColor == Piece::BLACK && j == 3)) {
							// Check if enemy pawn is to your left
							short pieceOnEnpassantSquare = getPiece(i - 1, j);
							if (Piece::getType(pieceOnEnpassantSquare) == Piece::PAWN && Piece::getColor(targetPiece) != currentPlayer) {
								// Check wether it moved 2 steps last turn
								enPassant = (moveHistory.top().startSquare[0] == i - 1) && (moveHistory.top().startSquare[1] == j + 2 * dir[1]);
							}
						}
					}
					// Target square has to be occupied by enemy piece
					if (regularCapture || enPassant) {
						// Move accepted
						Move move(Piece::PAWN | currentPlayer, Piece::PAWN | ((currentPlayer == Piece::WHITE) ? Piece::BLACK : Piece::WHITE), i, j, step, enPassant);
						possibleMoves.push_back(move);
						if (debugPossibleMoves) {
							std::cout << "Possible move " << Move::toString(move) << " accepted.\n";
						}
					}
				}
				
				//--------- CAPTURE RIGHT ---------------------
				if (i != 7) {
					// Clear step from capture left
					step &= 0b11110000;
					step |= RIGHT;
					stepsToDirection(step, dir);
					targetPiece = getPiece(i + dir[0], j + dir[1]);
					bool regularCapture = Piece::getType(targetPiece) != Piece::NONE && Piece::getColor(targetPiece) != currentPlayer;
					bool enPassant = false;
					if (!regularCapture) {
						// Check for en passant
						// White on 5th rank or black on 4th rank
						if ((pieceColor == Piece::WHITE && j == 4) || (pieceColor == Piece::BLACK && j == 3)) {
							// Check if enemy pawn is to your right
							// TODO: check that he moved last turn
							short pieceOnEnpassantSquare = getPiece(i + 1, j);
							if (Piece::getType(pieceOnEnpassantSquare) == Piece::PAWN && Piece::getColor(targetPiece) != currentPlayer) {
								// Check wether it moved 2 steps last turn
								enPassant = (moveHistory.top().startSquare[0] == i + 1) && (moveHistory.top().startSquare[1] == j + 2 * dir[1]);
							}
						}
					}
					// Target square has to be occupied by enemy piece
					if (regularCapture || enPassant) {
						// Move accepted
						Move move(Piece::PAWN | currentPlayer, Piece::PAWN | ((currentPlayer == Piece::WHITE) ? Piece::BLACK : Piece::WHITE), i, j, step, enPassant);
						possibleMoves.push_back(move);
						if (debugPossibleMoves) {
							std::cout << "Possible move " << Move::toString(move) << " accepted.\n";
						}
					}
				}
				
			}
			//----------- ALL OTHER MOVES ----------------------
			else {
				short* directions = Move::rookDirections;
				// Switch to diagonal moves (first) for bishop (and queen and king)
				if (pieceType == Piece::BISHOP || pieceType == Piece::QUEEN || pieceType == Piece::KING) {
					directions = Move::bishopDirections;
				}

				for (int dirIndex = 0; dirIndex < 4; dirIndex++) {
					// Go into one of the directions as long as possible and save possible moves along the way
					int steps = 0;
					int stepCounter = 0;
					while (stepCounter < 7) {
						// Go one step into the direction
						steps |= directions[dirIndex];
						short target[2];
						if (tryAddMove(i, j, steps, true, target))
						{
							stepCounter++;
							steps <<= 4;
							// If capturing piece, still stop after accepting
							if (Piece::getType(getPiece(target[0], target[1])) != Piece::NONE)
								break;
							// If king, also stop after accepting
							if (pieceType == Piece::KING) {
								break;
							}
						}
						else {
							break;
						}

					}
					// For queen and king, do bishop moves and then rook moves
					if (dirIndex == 3 && (pieceType == Piece::QUEEN || pieceType == Piece::KING) && directions == Move::bishopDirections) {
						dirIndex = -1;
						directions = Move::rookDirections;
					}
				}
			}
			//----------- CASTLING MOVES -----------------------
			if (pieceType == Piece::KING) {
				int steps = 0;
				if (currentPlayer == Piece::WHITE) {
					if ((castleRights & 0b1000) == 0b1000) {
						//std::cout << "White's short castle \n";
						// White's short castle
						if (getPiece(i + 1, j) == Piece::NONE && getPiece(i + 2, j) == Piece::NONE) {
							steps = (RIGHT << 4) | RIGHT;
							if (debugPossibleMoves) {
								std::cout << "White's short castle accepted.\n";
							}
						}
					}
					if ((castleRights & 0b0100) == 0b0100) {
						// White's long castle
						if (getPiece(i - 1, j) == Piece::NONE && getPiece(i - 2, j) == Piece::NONE && getPiece(i - 3, j) == Piece::NONE) {
							steps = (LEFT << 4) | LEFT;
							if (debugPossibleMoves) {
								std::cout << "White's long castle accepted.\n";
							}
						}
					}
				}
				else {
					if ((castleRights & 0b0010) == 0b0010) {
						// Black's short castle
						if (getPiece(i + 1, j) == Piece::NONE && getPiece(i + 2, j) == Piece::NONE) {
							steps = (RIGHT << 4) | RIGHT;
							if (debugPossibleMoves) {
								std::cout << "Black's short castle accepted.\n";
							}
						}
					}
					if ((castleRights & 0b0001) == 0b0001) {
						// Black's long castle
						if (getPiece(i - 1, j) == Piece::NONE && getPiece(i - 2, j) == Piece::NONE && getPiece(i - 3, j) == Piece::NONE) {
							int steps = (LEFT << 4) | LEFT;
							if (debugPossibleMoves) {
								std::cout << "Black's long castle accepted.\n";
							}
						}
					}
				}

				if (steps != 0) {
					Move move(Piece::KING | currentPlayer, Piece::NONE, i, j, steps);
					possibleMoves.push_back(move);
				}
			}
		}
	}
}

bool Board::tryAddMove(const unsigned short x, const unsigned short y, int steps, bool canCapture, short target[2])
{
	short dir[2];
	stepsToDirection(steps, dir);

	bool returnTarget = !(target == NULL);

	if (!returnTarget) {
		target = new short[2];
	}

	target[0] = x + dir[0];
	target[1] = y + dir[1];

	// If move goes out of bounds, discard
	if (target[0] > 7 || target[0] < 0 || target[1] > 7 || target[1] < 0)
		return false;
	// If move would capture friendly piece, discard
	if (Piece::getColor(getPiece(target[0], target[1])) == currentPlayer)
		return false;

	// Get target piece
	short capture = getPiece(target[0], target[1]);

	// If move would capture, but it's not allowed (pawn forward), discard
	if (!canCapture && Piece::getType(capture) != Piece::NONE)
		return false;

	// Move accepted
	Move move(getPiece(x,y), capture, x, y, steps);
	possibleMoves.push_back(move);

	if (debugPossibleMoves) {
		std::cout << "Move " << Move::toString(move) << " accepted.\n";
	}

	if (!returnTarget) {
		delete[] target;
	}

	return true;
}

// Converts an integer (step) to a short[2] x and y direction
void Board::stepsToDirection(int steps, short dir[2]) {
	//std::cout << "Converting steps " << steps << " to direction... ";
	dir[0] = 0;
	dir[1] = 0;
	int i = 0;
	while (steps != 0) {
		if (i % 2 == 0) {
			// y direction
			dir[1] += ((i % 4 == 0) ? 1 : -1) * (steps & 1);
		}
		else {
			// x direction
			dir[0] += (((i+1) % 4 == 0) ? -1 : 1) * (steps & 1);
		}
		steps >>= 1;
		i++;
		/*switch (steps & 0b0101) {
		case 0:
		case 0b0101:
			dir[0] += 0;
			break;
		case 0b0100:
			dir[0] += -1;
			break;
		case 0b0001:
			dir[0] += 1;
			break;
		default:
			break;
		}
		switch (steps & 0b1010) {
		case 0:
		case 0b1010:
			dir[1] += 0;
			break;
		case 0b1000:
			dir[1] += -1;
			break;
		case 0b0010:
			dir[1] += 1;
			break;
		default:
			break;
		}

		steps = steps >> 4;
		//std::cout << "Shifted. Steps is now " << steps << ".\n";*/
	}
	//std::cout << "[" << dir[0] << "][" << dir[1] << "]\n";
}

void Board::swapCurrentPlayer() {
	if (currentPlayer == Piece::WHITE)
		currentPlayer = Piece::BLACK;
	else
		currentPlayer = Piece::WHITE;
}

std::string Board::squareName(unsigned short column, unsigned short row) {
	if (row > 7 || column > 7) return "";
	std::string name;
	switch (column) {
	case 0:
		name = "a";
		break;
	case 1:
		name = "b";
		break;
	case 2:
		name = "c";
		break;
	case 3:
		name = "d";
		break;
	case 4:
		name = "e";
		break;
	case 5:
		name = "f";
		break;
	case 6:
		name = "g";
		break;
	case 7:
		name = "h";
		break;
	default:
		return "";
	}
	switch (row) {
	case 0:
		name += "1";
		break;
	case 1:
		name += "2";
		break;
	case 2:
		name += "3";
		break;
	case 3:
		name += "4";
		break;
	case 4:
		name += "5";
		break;
	case 5:
		name += "6";
		break;
	case 6:
		name += "7";
		break;
	case 7:
		name += "8";
		break;
	default:
		return "";
	}
	return name;
}
