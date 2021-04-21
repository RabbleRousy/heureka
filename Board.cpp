#include "Board.h"
#include "Piece.h"
#include <iostream>
#include <string>

Board::Board(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR")
	: PIECE()
{
	if (!readPosFromFEN(fen)) {
		std::cout << "Loading default position..." << std::endl;
		readPosFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
	}
}

void Board::clearBoard() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			squares[i][j] = PIECE.NONE;
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
			squares[column][row] = PIECE.KING | PIECE.WHITE;
			column++;
			break;
		case 'Q':
			squares[column][row] = PIECE.QUEEN | PIECE.WHITE;
			column++;
			break;
		case 'R':
			squares[column][row] = PIECE.ROOK | PIECE.WHITE;
			column++;
			break;
		case 'N':
			squares[column][row] = PIECE.KNIGHT | PIECE.WHITE;
			column++;
			break;
		case 'B':
			squares[column][row] = PIECE.BISHOP | PIECE.WHITE;
			column++;
			break;
		case 'P':
			squares[column][row] = PIECE.PAWN | PIECE.WHITE;
			column++;
			break;
		case 'k':
			squares[column][row] = PIECE.KING | PIECE.BLACK;
			column++;
			break;
		case 'q':
			squares[column][row] = PIECE.QUEEN | PIECE.BLACK;
			column++;
			break;
		case 'r':
			squares[column][row] = PIECE.ROOK | PIECE.BLACK;
			column++;
			break;
		case 'n':
			squares[column][row] = PIECE.KNIGHT | PIECE.BLACK;
			column++;
			break;
		case 'b':
			squares[column][row] = PIECE.BISHOP | PIECE.BLACK;
			column++;
			break;
		case 'p':
			squares[column][row] = PIECE.PAWN | PIECE.BLACK;
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
	squares[column][row] = PIECE.NONE;
}

bool Board::tryMakeMove(const unsigned short from[2], const unsigned short to[2]) {
	if (from[0] > 7 || from[1] > 7 || to[0] > 7 || to[1] > 7
		|| (from[0] == to[0] && from[1] == to[1])) {
		// ERROR WARNING
		return false;
	}
	short pieceFrom = getPiece(from[0], from[1]);
	short pieceTo = getPiece(to[0], to[1]);
	if (PIECE.getType(pieceFrom) == PIECE.NONE) {
		return false;
	}

	setPiece(to[0], to[1], pieceFrom);
	removePiece(from[0], from[1]);
	return true;
}
