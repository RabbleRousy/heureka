#include "Board.h"
#include "Piece.h"
#include <iostream>
#include <string>

Board::Board(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR")
	: currentPlayer(Piece::WHITE), possibleMoves(), moveHistory()
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
		std::cerr << "Tried illegal move (" << squareName(from[0], from[1]) << " to " << squareName(to[0], to[1]) << ")\n";
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
		short* dir = stepsToDirection(possibleMoves[i].steps);
		unsigned short result[2];
		result[0] = from[0] + dir[0];
		result[1] = from[1] + dir[1];
		if (result[0] != to[0] || result[1] != to[1])
			continue;

		setPiece(to[0], to[1], pieceFrom);
		removePiece(from[0], from[1]);
		std::cout << "Made move: " << squareName(from[0], from[1]) << " to " << squareName(to[0], to[1]) << "\n";
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
			if (pieceType == Piece::NONE || Piece::getColor(squares[i][j]) != currentPlayer)
				continue;

			std::cout << "Computing moves for " << Piece::name(squares[i][j]) << " on " << squareName(i, j) << " ...\n";
			// Knight Moves
			if (pieceType == Piece::KNIGHT) {
				for (int moveIndex = 0; moveIndex < 8; moveIndex++) {
					int steps = Move::knightMoves[moveIndex];
					short* dir = stepsToDirection(steps);
					int newX = i + dir[0];
					int newY = j + dir[1];
					// If move goes out of bounds, discard
					if (newX > 7 || newX < 0 || newY > 7 || newY < 0)
						continue;
					// If move would capture friendly piece, discard
					if (Piece::getColor(getPiece(newX, newY)) == currentPlayer)
						continue;

					// Move accepted
					std::cout << "Move to " << squareName(newX, newY) << " accepted. (steps = " << steps << ", dir = ["
						<< dir[0] << "][" << dir[1] << "])\n";
					Move move(i, j, steps);
					possibleMoves.push_back(move);
				}
			}
		}
	}
}

short* Board::stepsToDirection(int steps) {
	//std::cout << "Converting steps " << steps << " to direction... ";
	short* dir = new short[2];
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
	return dir;
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

Move::Move(unsigned short startX, unsigned short startY, int s)
{
	startSquare[0] = startX;
	startSquare[1] = startY;
	steps = s;
}

const int Move::knightMoves[8] = {
		(UP << 4) | UP | RIGHT ,
		(UP << 4) | UP | LEFT,
		(RIGHT << 4) | UP | RIGHT,
		(RIGHT << 4) | DOWN | RIGHT,
		(DOWN << 4) | DOWN | RIGHT,
		(DOWN << 4) | DOWN | LEFT,
		(LEFT << 4) | UP | LEFT,
		(LEFT << 4) | DOWN | LEFT
};
