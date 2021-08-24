#include "Board.h"
#include "Piece.h"
#include <iostream>
#include <string>

Board::Board(bool m, std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR")
	: currentPlayer(Piece::WHITE), possibleMoves(), moveHistory(), futureMovesBuffer(), debugLogs(m), wantsToPromote(false)
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

	const bool DEBUG = debugLogs;

	if (DEBUG) std::cout << "Trying to parse FEN: " << fen << std::endl;
	clearBoard();

	// FEN starts at the top left corner of the board
	unsigned short column = 0;
	unsigned short row = 7;
	int i = 0;
	char c = fen[0];
	while (i < fen.size() && fen[i] != ' ') {
		// Stop when at the end of the board
		if (row == 0 && column == 8)
			break;

		if (DEBUG) std::cout << "Parsing " << fen[i] << " ... \n";

		// Return false for invalid fens that reach out of bounds
		if (row < 0 || (column > 7 && fen[i] != '/')) {
			std::cerr << "FEN parsing failed." << std::endl;
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
			whiteKingPos[0] = column;
			whiteKingPos[1] = row;
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
			blackKingPos[0] = column;
			blackKingPos[1] = row;
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
		i++;
	}

	// Return false if FEN parsing didn't reach end of board
	if (!(row == 0 && column == 8))return false;
	// No additional infos
	if (i >= fen.size()-1) {
		if (DEBUG) std::cout << "End of FEN reached.\n";
		return true;
	}

	// Player to move
	switch (fen[++i]) {
	case 'b':
		currentPlayer = Piece::BLACK;
		break;
	default:
		currentPlayer = Piece::WHITE;
		break;
	}
	if (DEBUG)
		std::cout << "Current player read from FEN: " << fen[i] << '\n';

	// Castling rights
	castleRights = 0;
	int j = i+2;
	for (j; j < (i + 6); j++) {
		if (DEBUG) std::cout << "j = " << j << ", i = " << i << '\n';
		if (j == fen.size()) break;

		switch (fen[j]) {
			if (DEBUG) std::cout << "Parsing " << fen[j] << " ...\n";
		case '-':
			return true;
		case 'K':
			castleRights |= 0b1000;
			break;
		case 'Q':
			castleRights |= 0b0100;
			break;
		case 'k':
			castleRights |= 0b0010;
			break;
		case 'q':
			castleRights |= 0b0001;
			break;
		default:
			castleRights = 0b1111;
			return true;
		}
		if (DEBUG) std::cout << "Castle right input: " << fen[j] << '\n';
	}
	// Something went wrong while parsing castlerights, set all as default
	if (castleRights == 0)
		castleRights = 0b1111;

	// No ep capture left to read
	if (!(j < fen.size() - 2)) {
		return true;
	}

	column = 8;
	row = 8;
	for (i = (j + 1); i < (j + 3); i++) {
		switch (fen[i]) {
		case 'a':
			column = 0;
			break;
		case 'b':
			column = 1;
			break;
		case 'c':
			column = 2;
			break;
		case 'd':
			column = 3;
			break;
		case 'e':
			column = 4;
			break;
		case 'f':
			column = 5;
			break;
		case 'g':
			column = 6;
			break;
		case 'h':
			column = 7;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			row = std::atoi(&fen[i]) - 1;
			break;
		default:
			return true;
		}
		if (DEBUG) std::cout << "EP Capture input: " << fen[i] << '\n';
	}

	// Parsing failed
	if (column == 8 || !(row == 2 || row == 5)) {
		std::cerr << "EP capture parsing failed\n";
		return true;
	}

	// Remove the pawn and make the move manually
	swapCurrentPlayer();

	unsigned short from[2] = { column, (int)row + ((currentPlayer == Piece::WHITE) ? -1 : 1) };
	unsigned short to[2] = { column, (int)row + ((currentPlayer == Piece::WHITE) ? 1 : -1) };

	// Undo the move
	removePiece(to[0], to[1]);
	setPiece(from[0], from[1], Piece::PAWN | currentPlayer);

	Move epMove = Move(Piece::PAWN | currentPlayer, Piece::NONE, from[0], from[1], (currentPlayer == Piece::WHITE) ? ((UP << 4) | UP) : ((DOWN << 4) | DOWN));
	
	doMove(epMove);

	swapCurrentPlayer();

	return true;
}

std::string Board::getFENfromPos() {
	std::string fen = "";
	// Parse over the board from a8 to h1
	for (int i = 7; i >= 0; i--) {
		int gap = 0;
		for (int j = 0; j < 8; j++) {
			short p = getPiece(j, i);
			// if there is no piece at the current position, count gaps
			if (Piece::getType(p) == Piece::NONE) {
				gap++;
			}
			else {
				// if there was a gap, add it before adding the next piece and reset counter
				if (gap != 0) {
					fen += std::to_string(gap);
					gap = 0;
				}
				// add the piece to the fen
				fen += Piece::toChar(p);
			}
		}
		// end of row, check for remaining gap counter and add linebreak
		if (gap != 0) {
			fen += std::to_string(gap);
		}
		if (i != 0) {
			fen += '/';
		}
	}

	// who's turn to move
	fen += (currentPlayer == Piece::WHITE) ? " w" : " b";

	// Castling rights
	if (castleRights == 0) {
		fen += " -";
	}
	else {
		fen += ' ';
		if ((castleRights & 0b1000) != 0) fen += 'K';
		if ((castleRights & 0b0100) != 0) fen += 'Q';
		if ((castleRights & 0b0010) != 0) fen += 'k';
		if ((castleRights & 0b0001) != 0) fen += 'q';
	}
	
	// En passant captures
	if (moveHistory.empty())
		return fen;
	Move lastMove = moveHistory.top();
	if (Piece::getType(lastMove.piece) != Piece::PAWN)
		return fen;

	// If pawn did more than one step
	if (lastMove.steps > 0b1111) {
		fen += ' ';
		short dir[2];
		stepsToDirection(lastMove.steps, dir);
		fen += squareName(lastMove.startSquare[0], lastMove.startSquare[1] + dir[1] / 2);
	}

	return fen;
}

short Board::getPiece(unsigned short column, unsigned short row)
{
	if (row > 7 || column > 7) {
		return 0;
	}
	return squares[column][row];
}

void Board::setPiece(unsigned short column, unsigned short row, short p)
{
	if (row > 7 || column > 7) {
		std::cerr << "ERROR: setPiece out of bounds at [" << column << "][" << row << "]\n";
		return;
	}
	squares[column][row] = p;
}

void Board::removePiece(unsigned short column, unsigned short row) {
	if (row > 7 || column > 7) {
		std::cerr << "ERROR: removePiece out of bounds at [" << column << "][" << row << "]\n";
		return;
	}
	squares[column][row] = Piece::NONE;
}

bool Board::handleMoveInput(const unsigned short from[2], const unsigned short to[2], short promotionChoice) {
	if (from[0] == to[0] && from[1] == to[1])
		return false;
	if (from[0] > 7 || from[1] > 7 || to[0] > 7 || to[1] > 7) {
		std::cerr << "Tried illegal move (" << squareName(from[0], from[1]) << " to " << squareName(to[0], to[1]) << ")\n";
		return false;
	}
	
	if (debugLogs) std::cout << "Handling move input from " << squareName(from[0], from[1]) << " to " << squareName(to[0], to[1]) << " ...\n";

	if (wantsToPromote && promotionChoice != 0) {
		// Promotion choice was made, we already stored the correct move in promoMoveBuffer
		wantsToPromote = false;
		// Clear promoflags and set the correct one
		promoMoveBuffer.flags &= 0b1000;
		switch (promotionChoice)
		{
		case Piece::QUEEN:
			promoMoveBuffer.flags |= Move::Promotion::ToQueen;
			break;
		case Piece::ROOK:
			promoMoveBuffer.flags |= Move::Promotion::ToRook;
			break;
		case Piece::BISHOP:
			promoMoveBuffer.flags |= Move::Promotion::ToBishop;
			break;
		case Piece::KNIGHT:
			promoMoveBuffer.flags |= Move::Promotion::ToKnight;
			break;
		default:
			break;
		}

		doMove(promoMoveBuffer);

		swapCurrentPlayer();
		generateMoves();

		futureMovesBuffer = std::stack<Move>();

		if (debugLogs) std::cout << "Promotion to " << Piece::name(promotionChoice | currentPlayer) << " performed.\n";
		if (debugLogs) std::cout << "New FEN: " << getFENfromPos() << '\n';

		return true;
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

		if (debugLogs) std::cout << "Move found in possibleMoves list. Checking if it's promotion ...";

		// Promotion
		if (possibleMoves[i].isPromotion()) {
			if (debugLogs) std::cout << " YES. Setting wantsToPromote flag.\n";
			// Promotion needs to be completed by player
			wantsToPromote = true;
			promoMoveBuffer = possibleMoves[i];
			// Correct promotion move will be added afterwards
			return false;
		}
		if (debugLogs) std::cout << " NO. Doing move: " << Move::toString(possibleMoves[i]) << '\n';

		doMove(possibleMoves[i]);
		swapCurrentPlayer();
		generateMoves();

		if (debugLogs) std::cout << "New FEN: " << getFENfromPos() << '\n';

		futureMovesBuffer = std::stack<Move>();

		return true;
	}
	return false;
}

void Board::doMove(const Move move)
{
	const unsigned short* from = move.startSquare;
	unsigned short to[2];
	short dir[2];
	stepsToDirection(move.steps, dir);
	unsigned short result[2];
	to[0] = from[0] + dir[0];
	to[1] = from[1] + dir[1];

	short pieceFrom = move.piece;
	short pieceTo = move.capturedPiece;
	short promoResult = move.getPromotionResult();
	setPiece(to[0], to[1], promoResult);
	removePiece(from[0], from[1]);

	// En passant 
	if (move.isEnPassant()) {
		removePiece(from[0] + dir[0], from[1]);
	}

	if (Piece::getType(pieceFrom) == Piece::KING && (dir[0] == 2 || dir[0] == -2)) {
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

	//----------- UPDATE CASTLE RIGHTS ---------------------
	if (castleRights != 0) {
		// If king moved, delete that player's castle rights
		if (Piece::getType(pieceFrom) == Piece::KING) {
			castleRights &= (currentPlayer == Piece::WHITE) ? 0b0011 : 0b1100;
		}
		// If rook moved
		else if (Piece::getType(pieceFrom) == Piece::ROOK) {
			if (currentPlayer == Piece::WHITE && from[1] == 0) {
				if (from[0] == 0) {
					// Remove right for white's long castle
					castleRights &= 0b1011;
				}
				else if (from[0] == 7) {
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
		// If rook got captured
		else if (Piece::getType(pieceTo) == Piece::ROOK) {
			if (currentPlayer == Piece::BLACK && to[1] == 0) {
				if (to[0] == 0) {
					// Remove right for white's long castle
					castleRights &= 0b1011;
				}
				else if (to[0] == 7) {
					// Remove right for white's short castle
					castleRights &= 0b0111;
				}
			}
			else if (currentPlayer == Piece::WHITE && to[1] == 7) {
				if (to[0] == 0) {
					// Remove right for black's long castle
					castleRights &= 0b1110;
				}
				else if (to[0] == 7) {
					// Remove right for black's short castle
					castleRights &= 0b1101;
				}
			}
		}
	}

	//----------- UPDATE KING POS --------------------------
	if (Piece::getType(pieceFrom) == Piece::KING) {
		if (currentPlayer == Piece::WHITE) {
			whiteKingPos[0] = to[0];
			whiteKingPos[1] = to[1];
		}
		else {
			blackKingPos[0] = to[0];
			blackKingPos[1] = to[1];
		}
	}

	moveHistory.push(move);
}

void Board::undoLastMove()
{
	if (moveHistory.empty()) return;

	Move lastMove = moveHistory.top();
	moveHistory.pop();
	futureMovesBuffer.push(lastMove);

	if (debugLogs) std::cout << "Trying to undo Move >>" << Move::toString(lastMove) << "<<\n";

	short target[2];
	short dir[2];
	stepsToDirection(lastMove.steps, dir);
	target[0] = lastMove.startSquare[0] + dir[0];
	target[1] = lastMove.startSquare[1] + dir[1];

	// Place captured piece / clear target square
	if (lastMove.isEnPassant()) {
		removePiece(target[0], target[1]);
		setPiece(lastMove.startSquare[0] + dir[0], lastMove.startSquare[1], lastMove.capturedPiece);
	}
	else {
		setPiece(target[0], target[1], lastMove.capturedPiece);
	}

	// Place piece back at startsquare
	setPiece(lastMove.startSquare[0], lastMove.startSquare[1], lastMove.piece);

	// Check if the king moved
	if (Piece::getType(lastMove.piece) == Piece::KING) {
		if (Piece::getColor(lastMove.piece) == Piece::WHITE) {
			whiteKingPos[0] = lastMove.startSquare[0];
			whiteKingPos[1] = lastMove.startSquare[1];
		}
		else {
			blackKingPos[0] = lastMove.startSquare[0];
			blackKingPos[1] = lastMove.startSquare[1];
		}
		// If king castled
		if (dir[0] == 2 || dir[0] == -2) {
			//Rook has to be moved
			switch (dir[0])
			{
			case 2:
				setPiece(7, lastMove.startSquare[1], Piece::ROOK | Piece::getColor(lastMove.piece));
				removePiece(5, lastMove.startSquare[1]);
				break;
			case -2:
				setPiece(0, lastMove.startSquare[1], Piece::ROOK | Piece::getColor(lastMove.piece));
				removePiece(3, lastMove.startSquare[1]);
				break;
			default:
				break;
			}
		}
	}
}

void Board::redoLastMove() {
	if (futureMovesBuffer.empty()) return;
	doMove(futureMovesBuffer.top());
	futureMovesBuffer.pop();
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
				
				bool check = false;
				if (tryAddMove(i, j, step, false, NULL, &check) || check) {
					//-------- ANOTHER STEP AHEAD ------------------------
					// If pawn is still on its start square
					if ((pieceColor == Piece::WHITE && j == 1) || (pieceColor == Piece::BLACK) && j == 6) {
						step <<= 4;
						step |= (pieceColor == Piece::WHITE) ? UP : DOWN;
						tryAddMove(i, j, step, false);
					}
				}
				short targetPiece;
				// Clear step from double step
				step &= 0b00001111;
				step <<= 4;
				//-------- CAPTURE LEFT ----------------------
				if (i != 0) {
					step |= LEFT;
					tryAddMove(i, j, step, true);
				}
				
				//--------- CAPTURE RIGHT ---------------------
				if (i != 7) {
					// Clear step from capture left
					step &= 0b11110000;
					step |= RIGHT;
					tryAddMove(i, j, step, true);
				}
				
			}
			//----------- ALL OTHER MOVES ----------------------
			else {
				short* directions = Move::rookDirections;
				// Switch to diagonal moves (first) for bishop (and queen and king)
				if (pieceType == Piece::BISHOP || pieceType == Piece::QUEEN || pieceType == Piece::KING) {
					directions = Move::bishopDirections;
				}
				bool* check = new bool{};

				for (int dirIndex = 0; dirIndex < 4; dirIndex++) {
					// Go into one of the directions as long as possible and save possible moves along the way
					int steps = 0;
					int stepCounter = 0;
					while (stepCounter < 7) {
						// Go one step into the direction
						steps |= directions[dirIndex];
						short target[2];
						if (tryAddMove(i, j, steps, true, target, check) || *check) 
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
				delete check;
			}
			//----------- CASTLING MOVES -----------------------
			if (pieceType == Piece::KING) {
				short steps = 0;
				if (currentPlayer == Piece::WHITE) {
					if ((castleRights & 0b1000) == 0b1000) {
						//std::cout << "White's short castle \n";
						// White's short castle
						if (getPiece(i + 1, j) == Piece::NONE && getPiece(i + 2, j) == Piece::NONE) {
							steps = RIGHT | (RIGHT << 4);
							tryAddMove(i, j, steps, false);
						}
					}
					if ((castleRights & 0b0100) == 0b0100) {
						// White's long castle
						if (getPiece(i - 1, j) == Piece::NONE && getPiece(i - 2, j) == Piece::NONE && getPiece(i - 3, j) == Piece::NONE) {
							steps = (LEFT << 4) | LEFT;
							tryAddMove(i, j, steps, false);
						}
					}
				}
				else {
					if ((castleRights & 0b0010) == 0b0010) {
						// Black's short castle
						if (getPiece(i + 1, j) == Piece::NONE && getPiece(i + 2, j) == Piece::NONE) {
							steps = (RIGHT << 4) | RIGHT;
							tryAddMove(i, j, steps, false);
						}
					}
					if ((castleRights & 0b0001) == 0b0001) {
						// Black's long castle
						if (getPiece(i - 1, j) == Piece::NONE && getPiece(i - 2, j) == Piece::NONE && getPiece(i - 3, j) == Piece::NONE) {
							steps = (LEFT << 4) | LEFT;
							tryAddMove(i, j, steps, false);
						}
					}
				}
			}
		}
	}
}

bool Board::tryAddMove(const unsigned short x, const unsigned short y, int steps, bool canCapture, short target[2], bool* illegalBecauseCheck)
{
	short dir[2];
	stepsToDirection(steps, dir);
	short moveColor = Piece::getColor(getPiece(x, y));

	bool returnTarget = !(target == NULL);

	if (!returnTarget) {
		target = new short[2];
	}

	target[0] = x + dir[0];
	target[1] = y + dir[1];

	// If move goes out of bounds, discard
	if (target[0] > 7 || target[0] < 0 || target[1] > 7 || target[1] < 0) {
		if (!returnTarget) {
			delete[] target;
		}
		return false;
	}
		
	// If move would capture friendly piece, discard
	if (Piece::getColor(getPiece(target[0], target[1])) == currentPlayer) {
		if (!returnTarget) {
			delete[] target;
		}
		return false;
	}
		

	// Get target piece
	short capture = getPiece(target[0], target[1]);

	// If move would capture, but it's not allowed (pawn forward), discard
	if (!canCapture && Piece::getType(capture) != Piece::NONE) {
		if (!returnTarget) {
			delete[] target;
		}
		return false;
	}


	// Handle pawn move (promotion check)
	bool promote = false;
	short flags = 0;
	if (Piece::getType(getPiece(x, y)) == Piece::PAWN) {
		promote = (target[1] == 7 && currentPlayer == Piece::WHITE) || (target[1] == 0 && currentPlayer == Piece::BLACK);
		// Handle pawn captures (enpassant check)
		if (canCapture) {
			bool regularCapture = Piece::getType(capture) != Piece::NONE && Piece::getColor(capture) != currentPlayer;
			bool enPassant = false;
			if (!regularCapture) {
				// Check for en passant
				// White on 5th rank or black on 4th rank
				if ((moveColor == Piece::WHITE && y == 4) || (moveColor == Piece::BLACK && y == 3)) {
					// Check if enemy pawn is next to you
					short pieceOnEnpassantSquare = getPiece(x + dir[0], y);
					if (Piece::getType(pieceOnEnpassantSquare) == Piece::PAWN && Piece::getColor(capture) != currentPlayer) {
						// Check wether it moved 2 steps last turn
						enPassant = (moveHistory.top().startSquare[0] == x + dir[0]) && (moveHistory.top().startSquare[1] == y + 2 * dir[1]);
					}
				}
			}
			if (!(regularCapture || enPassant))
				return false;
			// If ep flag is set
			if (enPassant) {
				// Update the captured piece
				capture = Piece::PAWN | Piece::getOppositeColor(currentPlayer);
				// Save flag
				flags = enPassant * 8;
			}
		}
	}

	Move move(getPiece(x, y), capture, x, y, steps, flags);

	// If king tries to move more than one square, check for castling
	if (Piece::getType(getPiece(x, y)) == Piece::KING && steps > 8) {
		move.steps = steps & 0b1111;
		if (kingInCheckAfter(move))
			// Castling is interrupted on the first step
			return false;
		move.steps = steps;
	}

	if (!kingInCheckAfter(move)) {

		// Move accepted
		if (!promote) {
			possibleMoves.push_back(move);
		}
		else {
			// If it's a promotion, add one move for each type of promotion and set corresponding flag
			move.flags |= Move::Promotion::ToBishop;
			possibleMoves.push_back(move);
			move.flags |= Move::Promotion::ToKnight;
			possibleMoves.push_back(move);
			move.flags |= Move::Promotion::ToQueen;
			possibleMoves.push_back(move);
			move.flags |= Move::Promotion::ToRook;
			possibleMoves.push_back(move);
		}

		if (debugLogs) {
			std::cout << "Move " << Move::toString(move) << " accepted.\n";
		}
	}
	else {
		if (illegalBecauseCheck != NULL) {
			*illegalBecauseCheck = true;
		}
		if (!returnTarget) {
			delete[] target;
		}
		return false;
	}

	if (!returnTarget) {
		delete[] target;
	}

	return true;
}

bool Board::kingIsInCheck(const short color)
{
	short* kingPos = (color == Piece::WHITE) ? whiteKingPos : blackKingPos;
	short* directions = Move::bishopDirections;

	for (int dirIndex = 0; dirIndex < 4; dirIndex++) {
		// Go into one of the directions as long as possible and look for enemy pieces
		short dir[2];
		stepsToDirection(directions[dirIndex], dir);
		
		for (int i = 1; i < 8; i++) {
			short pieceOnTargetSquare = getPiece(kingPos[0] + i * dir[0], kingPos[1] + i * dir[1]);

			short pieceType = Piece::getType(pieceOnTargetSquare);

			if (pieceType == Piece::NONE) continue;
			if (Piece::getColor(pieceOnTargetSquare) == color) break;

			if (directions == Move::bishopDirections) {
				// "Short range" checks
				if (i == 1) {
					// Pawn checks
					if (pieceType == Piece::PAWN) {
						// White king get's checked by pawns above him
						if (color == Piece::WHITE && dirIndex < 2) {
							return true;
						}
						// Black king get's checked by pawns below him
						else if (color == Piece::BLACK && dirIndex >= 2) {
							return true;
						}
					}
					// Diagonal King checks
					else if (pieceType == Piece::KING){
						return true;
					}
				}
				// Diagonal checks
				if (pieceType == Piece::BISHOP || pieceType == Piece::QUEEN)
					return true;
				else
					break;
			}
			// Horizontal/vertical checks
			else {
				if (pieceType == Piece::ROOK || pieceType == Piece::QUEEN || (pieceType == Piece::KING && i == 1))
					return true;
				else
					break;
			}
		}

		// Do bishop moves and then rook moves
		if (dirIndex == 3 && directions == Move::bishopDirections) {
			dirIndex = -1;
			directions = Move::rookDirections;
		}
	}

	//---------- KNIGHT CHECKS -----------------
	directions = Move::knightMoves;
	for (int i = 0; i < 8; i++) {
		short dir[2];
		stepsToDirection(directions[i], dir);
		short pieceOnTargetSquare = getPiece(kingPos[0] + dir[0], kingPos[1] + dir[1]);
		// Check if there is an enemy knight on one of the squares where it's targeting the king
		if (pieceOnTargetSquare == (Piece::KNIGHT | Piece::getOppositeColor(color))) {
			return true;
		}
	}

	return false;
}

bool Board::kingInCheckAfter(const Move move)
{
	if (debugLogs) std::cout << "Is king in check after " << Move::toString(move) << "? ";
	// Do the move
	doMove(move);

	// Is king in check now?
	bool check = kingIsInCheck(currentPlayer);
	
	// Undo move
	undoLastMove();

	if (debugLogs) std::cout << (check ? "Yes." : "No.") << "\n";
	return check;
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

int Board::testMoveGeneration(unsigned int depth, bool divide)
{
	if (depth == 1) return possibleMoves.size();
	int positionCount = 0;
	std::vector<Move> moves = possibleMoves;

	for (int i = 0; i < possibleMoves.size(); i++) {
		doMove(possibleMoves[i]);
		swapCurrentPlayer();
		generateMoves();
		int positionsAfterMove = testMoveGeneration(depth - 1, false);
		positionCount += positionsAfterMove;
		undoLastMove();
		swapCurrentPlayer();
		possibleMoves = moves;
		if (divide) {
			std::cout << Move::toString(possibleMoves[i]) << ": " << std::to_string(positionsAfterMove) << '\n';
		}
	}
	futureMovesBuffer = std::stack<Move>();
	return positionCount;
}
