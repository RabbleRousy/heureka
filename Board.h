#pragma once
#include "Piece.h"
#include "Move.h"
#include <vector>
#include <stack>

#define UP 0b0001
#define DOWN 0b0100
#define RIGHT 0b0010
#define LEFT 0b1000



class Board
{
private:
	short squares[8][8];
	short whiteKingPos[2];
	short blackKingPos[2];
	// Castle rights as bits: O-O, O-O-O, o-o, o-o-o
	short castleRights = 0b1111;
public:
	Board(bool m, std::string fen);
	short currentPlayer;
	bool wantsToPromote;
	bool debugPossibleMoves;
	std::vector<Move> possibleMoves;
	std::stack<Move> moveHistory;
	Move promoMoveBuffer;
	void clearBoard();
	bool readPosFromFEN(std::string fen);
	std::string getFENfromPos();
	short getPiece(unsigned short column, unsigned short row);
	void setPiece(unsigned short column, unsigned short row, short piece);
	void removePiece(unsigned short column, unsigned short row);
	bool tryMakeMove(const unsigned short from[2], const unsigned short to[2], short promotionChoice = 0);
	void generateMoves();
	bool tryAddMove(unsigned short x, unsigned short y, int steps, bool canCapture, short target[2] = NULL, bool* illegalBecauseCheck = NULL);
	bool kingIsInCheck(const short color);
	bool kingInCheckAfter(const Move move);
	static void stepsToDirection(int steps, short dir[2]);
	void swapCurrentPlayer();
	static std::string squareName(unsigned short column, unsigned short row);
};

