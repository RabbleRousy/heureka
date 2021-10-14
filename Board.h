#pragma once
#include "Piece.h"
#include "Move.h"
#include "Bitboard.h"
#include <vector>
#include <stack>



// Central class that handles everything that happens on the board.
class Board
{
private:
	static const std::string squareNames[64];

	Bitboard bb;
	short squares[64];
	short whiteKingPos;
	short blackKingPos;
	AttackData attackData;

	const int pawnValue = 100;
	const int knightValue = 300;
	const int bishopValue = 300;
	const int rookValue = 500;
	const int queenValue = 900;

public:

	struct SearchResults {
		unsigned int positionsSearched;
		Move bestMove;
		int evaluation;

		SearchResults() : positionsSearched(0), evaluation(0) {}
	};

	bool aiPlayer;
	unsigned int searchDepth;
	
	// Castle rights as bits: O-O, O-O-O, o-o, o-o-o
	static short castleRights;

	static unsigned short enPassantSquare;

	/// <summary>
	/// Constructor for the Board.
	/// </summary>
	Board();

	// Whose turn it is, either Piece::WHITE or Piece::BLACK
	short currentPlayer;

	bool checkMate, staleMate;

	SearchResults currentSearch;

	// Indicates to the GUI wether the player needs to input a promotion choice
	bool wantsToPromote;

	// If set, detailed infos on move generation will be logged into the console
	bool debugLogs;

	std::vector<Move> possibleMoves;

	std::stack<Move> moveHistory;

	std::stack<Move> futureMovesBuffer;

	// Stores the pawn move while waiting for input on the promotion choice
	Move promoMoveBuffer;

	// Sets all squares to Piece::NONE
	void clearBoard();

	void reset();

	/// <summary>
	/// 
	/// </summary>
	/// <param name="fen"></param>
	/// <returns></returns>
	bool readPosFromFEN(std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

	/// <summary>
	/// Constructs a FEN string from the current game state.
	/// </summary>
	/// <returns>the FEN string representing the current game state.</returns>
	std::string getFENfromPos();

	/// <param name="column">from 0 to 7 (a to h).</param>
	/// <param name="row">from 0 to 7 (1 to 8).</param>
	/// <returns>the piece on the desired square, can be Piece::NONE if square was empty or invalid.</returns>
	short getPiece(unsigned short column, unsigned short row);
	/// <param name="index">from 0 to 63 (a1 to h8)</param>
	/// <returns>the piece on the desired square, can be Piece::NONE if square was empty or invalid.</returns>
	short getPiece(unsigned short index);

	/// <summary>
	/// Places a piece on the desired square.
	/// </summary>
	/// <param name="column">from 0 to 7 (a to h).</param>
	/// <param name="row">from 0 to 7 (1 to 8).</param>
	/// <param name="piece"> to be placed, use Piece::PieceType | Piece::PieceColor</param>
	void setPiece(unsigned short column, unsigned short row, short piece);
	/// <summary>
	/// Places a piece on the desired square.
	/// </summary>
	/// <param name="index">from 0 to 63 (a1 to h8)</param>
	/// <param name="piece">to be placed, use Piece::PieceType | Piece::PieceColor</param>
	void setPiece(unsigned short index, short piece);

	/// <summary>
	/// Removes the piece at the desired position and sets that square to Piece::NONE
	/// </summary>
	/// <param name="column">from 0 to 7 (a to h).</param>
	/// <param name="row">from 0 to 7 (1 to 8).</param>
	void removePiece(unsigned short column, unsigned short row);
	/// <summary>
	/// Removes the piece at the desired position and sets that square to Piece::NONE
	/// </summary>
	/// /// <param name="index">from 0 to 63 (a1 to h8)</param>
	void removePiece(unsigned short index);

	/// <summary>
	/// Performs a move on the board, adds it to the moveHistory.
	/// Does NOT swap player and regenerate moves!
	/// </summary>
	/// <param name="move"> to be made.</param>
	void doMove(const Move* move);

	void doMove(std::string move);

	/// <summary>
	/// Undos the move given move and moves it to the futureMovesBuffer.
	/// Does NOT swap player and regenerate moves!
	/// </summary>
	/// <param name="move">to be undone.</param>
	void undoMove(const Move* move);

	bool undoLastMove();

	/// <summary>
	/// Redos the move at the top of the futureMovesBuffer by calling doMove().
	/// </summary>
	/// <returns>wether there was a move to be redone.</returns>
	bool redoLastMove();

	bool inCheckAfter(const Move* move);

	/// <summary>
	/// Tries to call doMove() with the given input and update the board if a corresponding move is found in the possibleMoves vector.
	/// </summary>
	/// <param name="from">stores the position where the move starts.</param>
	/// <param name="to">stores the position where the move ends.</param>
	/// <param name="promotionChoice">is only used when not 0 and wantsToPromote is true.</param>
	/// <returns>wether the move was contained in the possibleMoves vector and could be made. If true, the game is now updated and ready for the next move.</returns>
	bool handleMoveInput(const unsigned short from[2], const unsigned short to[2], short promotionChoice = 0);
	
	void makePlayerMove(const Move* move);

	void makeAiMove();

	/// <summary>
	/// Clears the possibleMoves vector and regenerates it.
	/// For each piece on the board of the current player's color, every step of each direction it can go is calculated.
	/// tryAddMove() then tries to construct the corresponding move and if legal, add it to the vector.
	/// </summary>
	void generateMoves();

	/// <summary>
	/// Generates pseudo legal pawn moves and adds them to possible moves vector.
	/// </summary>
	void generatePawnMoves();

	void generateKingMoves();

	void generateKnightMoves();

	void generateRookMoves();

	void generateBishopMoves();

	void generateQueenMoves();

	int staticEvaluation();

	int evaluateMaterial();

	int negaMax(unsigned int depth, int alpha, int beta, bool firstCall);

	void searchBestMove(unsigned int depth);

	void iterativeSearch(float time);

	/// <summary>
	/// Converts a step to a x and y direction by bitshifting.
	/// </summary>
	/// <param name="steps">holds up to 8 steps, each step is a 4 bit code (first is right).</param>
	/// <param name="dir">points to where the result is stored.</param>
	static void stepsToDirection(int steps, short dir[2]);

	void swapCurrentPlayer();

	/// <summary>
	/// Converts square coordinates to the name of that square on the board.
	/// </summary>
	/// <param name="column">from 0 to 7 (a to h).</param>
	/// <param name="row">from 0 to 7 (1 to 8).</param>
	/// <returns>a string containing the name of the square, e.g. "d4".
	/// empty string if parameters were invalid.</returns>
	static std::string getSquareName(unsigned short index);

	int testMoveGeneration(unsigned int depth, bool divide);
};
