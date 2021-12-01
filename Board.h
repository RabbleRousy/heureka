#pragma once
#include "Piece.h"
#include "Move.h"
#include "Bitboard.h"
#include <vector>
#include <stack>
#include <thread>
#include <future>

#ifdef _DEBUG
#define DEBUG_COUT(x) (std::cout << (x))
#define DEBUG_CERR(x) (std::cerr << (x))
#else
#define DEBUG_COUT(x)
#define DEBUG_CERR(x)
#endif

using namespace std::literals::chrono_literals;

// Central class that handles everything that happens on the board.
class Board
{
private:
	static const std::string squareNames[64];

	short squares[64];
	short whiteKingPos;
	short blackKingPos;
	AttackData attackData;

	unsigned long long currentZobristKey;

	bool timeOut;

	const int pawnValueMap[64] = {
	//  A1   B1   C1   D1   E1   F1   G1   H1
		0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  ,
		105, 110, 110,  80,  80, 110, 110, 105,
		105,  95, 90 , 100, 100, 90 ,  95, 105,
		100, 100, 100, 120, 120, 100, 100, 100,
		105, 105, 110, 125, 125, 110, 105, 105,
		110, 110, 120, 130, 130, 120, 110, 110,
		150, 150, 150, 150, 150, 150, 150, 150,
		0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  
	//  A8	 B8	  C8   D8	E8	 F8   G8   H8
	};

	const int kingValueMapEarlyMid[64] = {
	//  A1   B1   C1   D1   E1   F1   G1   H1
		20 , 30 , 10 , 0  , 0  , 10 , 30 , 20 ,
		20 , 20 , 0  ,-10 ,-10 , 0  , 20 , 20 ,
	   -10 ,-20 ,-20 ,-20 ,-20 ,-20 ,-20 ,-10 ,
	   -20 ,-30 ,-30 ,-40 ,-40 ,-30 ,-30 ,-20 ,
	   -30 ,-40 ,-40 ,-50 ,-50 ,-40 ,-40 ,-30 ,
	   -30 ,-40 ,-40 ,-50 ,-50 ,-40 ,-40 ,-30 ,
	   -30 ,-40 ,-40 ,-50 ,-50 ,-40 ,-40 ,-30 ,
	   -30 ,-40 ,-40 ,-50 ,-50 ,-40 ,-40 ,-30
	//  A8	 B8	  C8   D8	E8	 F8   G8   H8
	};

	const int kingValueMapEnd[64] = {
	//  A1   B1   C1   D1   E1   F1   G1   H1
	   -50 ,-30 ,-30 , 0  , 0  ,-30 ,-30 ,-50 ,
	   -30 ,-30 , 0  , 0  , 0  , 0  ,-30 ,-30 ,
	   -30 ,-10 , 20 , 30 , 30 , 20 ,-10 ,-30 ,
	   -30 ,-10 , 30 , 40 , 40 , 30 ,-10 ,-30 ,
	   -30 ,-10 , 30 , 40 , 40 , 30 ,-10 ,-30 ,
	   -30 ,-10 , 20 , 30 , 30 , 20 ,-10 ,-30 ,
	   -30 ,-20 ,-10 , 0  , 0  ,-10 ,-20 ,-30 ,
	   -50 ,-40 ,-30 ,-20 ,-20 ,-30 ,-40 ,-50
	//  A8	 B8	  C8   D8	E8	 F8   G8   H8
	};

	const int knightValueMap[64] = {
	//  A1   B1   C1   D1   E1   F1   G1   H1
		270, 280, 290, 290, 290, 290, 280, 270,
		280, 300, 320, 325, 325, 320, 300, 280,
		290, 325, 330, 335, 335, 330, 325, 290,
		290, 320, 335, 340, 340, 335, 320, 290,
		290, 325, 335, 340, 340, 335, 325, 290,
		290, 320, 330, 335, 335, 330, 320, 290,
		280, 300, 320, 320, 320, 320, 300, 280,
		270, 280, 290, 290, 290, 290, 280, 270
	//  A8	 B8	  C8   D8	E8	 F8   G8   H8
	};

	const int bishopValueMap[64] = {
	//  A1   B1   C1   D1   E1   F1   G1   H1
		310, 320, 320, 320, 320, 320, 320, 310,
		320, 335, 330, 330, 330, 330, 335, 320,
		320, 340, 340, 340, 340, 340, 340, 320,
		320, 330, 340, 340, 340, 340, 330, 320,
		320, 335, 335, 340, 340, 335, 335, 320,
		320, 330, 335, 340, 340, 335, 330, 320,
		320, 330, 330, 330, 330, 330, 330, 320,
		310, 320, 320, 320, 320, 320, 320, 310
	//  A8	 B8	  C8   D8	E8	 F8   G8   H8
	};

	const int rookValueMap[64] = {
		//  A1   B1   C1   D1   E1   F1   G1   H1
			500, 500, 500, 505, 505, 500, 500, 500,
			495, 500, 500, 500, 500, 500, 500, 495,
			495, 500, 500, 500, 500, 500, 500, 495,
			495, 500, 500, 500, 500, 500, 500, 495,
			495, 500, 500, 500, 500, 500, 500, 495,
			495, 500, 500, 500, 500, 500, 500, 495,
			505, 510, 510, 510, 510, 510, 510, 505,
			500, 500, 500, 500, 500, 500, 500, 500
		//  A8	 B8	  C8   D8	E8	 F8   G8   H8
	};

	const int queenValueMap[64] = {
		//  A1   B1   C1   D1   E1   F1   G1   H1
			880, 890, 890, 895, 895, 890, 890, 880,
			890, 900, 900, 900, 900, 900, 900, 890,
			890, 905, 905, 905, 905, 905, 905, 890,
			900, 900, 905, 905, 905, 905, 900, 900,
			895, 900, 905, 905, 905, 905, 900, 895,
			890, 900, 905, 905, 905, 905, 900, 890,
			890, 900, 900, 900, 900, 900, 900, 890,
			880, 890, 890, 895, 895, 890, 890, 880
		//  A8	 B8	  C8   D8	E8	 F8   G8   H8
	};

public:
	static Bitboard bb;

	struct SearchResults {
		unsigned int depth;
		unsigned int positionsSearched;
		Move bestMove;
		int evaluation;

		SearchResults() : positionsSearched(0), evaluation(0), depth(0), bestMove(Move::NULLMOVE) {}
	};
	float searchTime;
	bool processing;
	bool stopDemanded;
	
	// Castle rights as bits: O-O, O-O-O, o-o, o-o-o
	static short castleRights;

	static unsigned short enPassantSquare;

	static unsigned short fullMoveCount, halfMoveCount;

	/// <summary>
	/// Constructor for the Board.
	/// </summary>
	Board();

	// Whose turn it is, either Piece::WHITE or Piece::BLACK
	short currentPlayer;

	bool checkMate, remis;

	SearchResults currentSearch;

	// Indicates to the GUI wether the player needs to input a promotion choice
	bool wantsToPromote;

	std::vector<Move> possibleMoves;

	std::stack<Move> moveHistory;
	std::vector<unsigned long long> positionHistory;

	std::stack<Move> futureMovesBuffer;

	// Stores the pawn move while waiting for input on the promotion choice
	Move promoMoveBuffer;

	// Sets all squares to Piece::NONE
	void clearBoard();

	void reset();

	void init(std::string fen);

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
	
	bool checkForMateOrRemis();

	bool checkForRepetition();

	void makePlayerMove(const Move* move);

	void makeAiMove();

	/// <summary>
	/// Clears the possibleMoves vector and regenerates it.
	/// For each piece on the board of the current player's color, every step of each direction it can go is calculated.
	/// tryAddMove() then tries to construct the corresponding move and if legal, add it to the vector.
	/// </summary>
	void generateMoves(bool onlyCaptures = false);

	void generatePawnMoves(bool onlyCaptures);

	void generateKingMoves(bool onlyCaptures);

	void generateKnightMoves(bool onlyCaptures);

	void generateRookMoves(bool onlyCaptures);

	void generateBishopMoves(bool onlyCaptures);

	void generateQueenMoves(bool onlyCaptures);

	void orderMoves();

	int staticEvaluation();

	int evaluateMaterial();

	template <short color>
	int countMaterial();

	template <short color>
	int evaluatePawns();

	template <short color>
	int evaluateKing();

	template <short color>
	int evaluateKnights();

	template <short color>
	int evaluateBishops();

	template <short color>
	int evaluateRooks();

	template <short color>
	int evaluateQueens();

	int negaMax(unsigned int depth, int alpha, int beta, SearchResults* results, bool firstCall, bool allowNull);

	int negaMaxQuiescence(int alpha, int beta, SearchResults* results, int depth);

	SearchResults searchBestMove(unsigned int depth);

	SearchResults iterativeSearch(float time);

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

	void printPositionHistory();

	void print();

	unsigned long long testMoveGeneration(unsigned int depth, bool divide);

	std::thread launchSearchThread(float time);
};