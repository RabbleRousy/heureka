#include "Board.h"
#include "Piece.h"
#include <iostream>
#include <string>
#include "Profiling.h"

const std::string Board::squareNames[] = {
		"a1","b1","c1","d1","e1","f1","g1","h1",
		"a2","b2","c2","d2","e2","f2","g2","h2",
		"a3","b3","c3","d3","e3","f3","g3","h3",
		"a4","b4","c4","d4","e4","f4","g4","h4",
		"a5","b5","c5","d5","e5","f5","g5","h5",
		"a6","b6","c6","d6","e6","f6","g6","h6",
		"a7","b7","c7","d7","e7","f7","g7","h7",
		"a8","b8","c8","d8","e8","f8","g8","h8"
};

short Board::castleRights = 0b1111;
unsigned short Board::enPassantSquare = 64;

Board::Board() : possibleMoves(), moveHistory(), futureMovesBuffer(), debugLogs(false), wantsToPromote(false), currentPlayer(Piece::WHITE)
{ }


void Board::clearBoard() {
	for (int i = 0; i < 64; i++) {
		removePiece(i);
	}
}

bool Board::readPosFromFEN(std::string fen) {

	const bool DEBUG = debugLogs;

	if (DEBUG) std::cout << "Trying to parse FEN: " << fen << std::endl;
	clearBoard();
	currentPlayer = Piece::WHITE;
	castleRights = 0b1111;

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
			setPiece(column, row, Piece::KING | Piece::WHITE);
			whiteKingPos = column + row * 8;
			column++;
			break;
		case 'Q':
			setPiece(column, row, Piece::QUEEN | Piece::WHITE);
			column++;
			break;
		case 'R':
			setPiece(column, row, Piece::ROOK | Piece::WHITE);
			column++;
			break;
		case 'N':
			setPiece(column, row, Piece::KNIGHT | Piece::WHITE);
			column++;
			break;
		case 'B':
			setPiece(column, row, Piece::BISHOP | Piece::WHITE);
			column++;
			break;
		case 'P':
			setPiece(column, row, Piece::PAWN | Piece::WHITE);
			column++;
			break;
		case 'k':
			setPiece(column, row, Piece::KING | Piece::BLACK);
			blackKingPos = column + row * 8;
			column++;
			break;
		case 'q':
			setPiece(column, row, Piece::QUEEN | Piece::BLACK);
			column++;
			break;
		case 'r':
			setPiece(column, row, Piece::ROOK | Piece::BLACK);
			column++;
			break;
		case 'n':
			setPiece(column, row, Piece::KNIGHT | Piece::BLACK);
			column++;
			break;
		case 'b':
			setPiece(column, row, Piece::BISHOP | Piece::BLACK);
			column++;
			break;
		case 'p':
			setPiece(column, row, Piece::PAWN | Piece::BLACK);
			column++;
			break;
		default:
			std::cout << "FEN parsing failed!\n";
			return false;
		}
		i++;
	}

	// Return false if FEN parsing didn't reach end of board
	if (!(row == 0 && column == 8))return false;
	// No additional infos
	if (i >= fen.size()-1) {
		if (DEBUG) std::cout << "End of FEN reached.\n";

		std::cout << "\nRooks bitboard after start:\n" << bb.toString(bb.getBitboard(Piece::ROOK | currentPlayer) | bb.getBitboard(Piece::ROOK | Piece::getOppositeColor(currentPlayer)));
		std::cout << "\nKnights bitboard after start:\n" << bb.toString(bb.getBitboard(Piece::KNIGHT | currentPlayer) | bb.getBitboard(Piece::KNIGHT | Piece::getOppositeColor(currentPlayer)));
		std::cout << "\nBishops bitboard after start:\n" << bb.toString(bb.getBitboard(Piece::BISHOP | currentPlayer) | bb.getBitboard(Piece::BISHOP | Piece::getOppositeColor(currentPlayer)));
		std::cout << "\nPawns bitboard after start:\n" << bb.toString(bb.getBitboard(Piece::PAWN | currentPlayer) | bb.getBitboard(Piece::PAWN | Piece::getOppositeColor(currentPlayer)));

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
	if (DEBUG) std::cout << "Castle rights set to: " << std::to_string(castleRights) << '\n';

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

	unsigned short from = column + 8 * (int)row + ((currentPlayer == Piece::WHITE) ? -1 : 1);
	unsigned short to = column + 8 * (int)row + ((currentPlayer == Piece::WHITE) ? 1 : -1);

	// Undo the move
	removePiece(to);
	setPiece(from, Piece::PAWN | currentPlayer);

	Move epMove = Move(Piece::PAWN | currentPlayer, Piece::NONE, from, (currentPlayer == Piece::WHITE) ? from + 16 : from - 16);
	
	doMove(&epMove);

	swapCurrentPlayer();

	/*

	std::cout << "\nRooks bitboard after start:\n" << bb.toString(bb.getBitboard(Piece::ROOK | currentPlayer) | bb.getBitboard(Piece::ROOK | Piece::getOppositeColor(currentPlayer)));
	std::cout << "\nKnights bitboard after start:\n" << bb.toString(bb.getBitboard(Piece::KNIGHT | currentPlayer) | bb.getBitboard(Piece::KNIGHT | Piece::getOppositeColor(currentPlayer)));
	std::cout << "\nBishops bitboard after start:\n" << bb.toString(bb.getBitboard(Piece::BISHOP | currentPlayer) | bb.getBitboard(Piece::BISHOP | Piece::getOppositeColor(currentPlayer)));
	std::cout << "\nPawns bitboard after start:\n" << bb.toString(bb.getBitboard(Piece::PAWN | currentPlayer) | bb.getBitboard(Piece::PAWN | Piece::getOppositeColor(currentPlayer)));
	*/

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
	if (enPassantSquare != 64) {
		fen += ' ' + getSquareName(enPassantSquare);
	}

	return fen;
}

short Board::getPiece(unsigned short column, unsigned short row)
{
	if (column > 7 || row > 7) return 0;
	return squares[row * 8 + column];
}

short Board::getPiece(unsigned short index)
{
	if (index > 63) {
		//std::cerr << "ERROR: getPiece out of bounds at [" << column << "][" << row << "]\n";
		return 0;
	}
	return squares[index];
}

void Board::setPiece(unsigned short column, unsigned short row, short p)
{

	if (column > 7 || row > 7) return;
	unsigned short index = row * 8 + column;
	setPiece(index, p);
}

void Board::setPiece(unsigned short index, short piece)
{
	if (index > 63) {
		std::cerr << "ERROR: setPiece out of bounds at [" << index << "]\n";
		return;
	}
	if (squares[index] != Piece::NONE) bb.removePiece(squares[index], index);
	bb.setPiece(piece, index);
	squares[index] = piece;
}

void Board::removePiece(unsigned short column, unsigned short row) {
	if (column > 7 || row > 7) return;
	unsigned short index = row * 8 + column;
	removePiece(index);
}

void Board::removePiece(unsigned short index)
{
	if (index > 63) {
		std::cerr << "ERROR: removePiece out of bounds at [" << index << "]\n";
		return;
	}
	bb.removePiece(squares[index], index);
	squares[index] = Piece::NONE;
}

bool Board::handleMoveInput(const unsigned short from[2], const unsigned short to[2], short promotionChoice) {
	unsigned short start = from[1] * 8 + from[0];
	unsigned short target = to[1] * 8 + to[0];
	if (start == target)
		return false;
	if (start > 63 || target > 63) {
		std::cerr << "Tried illegal move (" << getSquareName(start) << " to " << getSquareName(target) << ")\n";
		return false;
	}
	
	if (debugLogs) std::cout << "Handling move input from " << getSquareName(start) << " to " << getSquareName(target) << " ...\n";

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

		doMove(&promoMoveBuffer);
		moveHistory.push(promoMoveBuffer);

		swapCurrentPlayer();
		generateMoves();

		futureMovesBuffer = std::stack<Move>();

		if (debugLogs) {
			std::cout << "Promotion to " << Piece::name(promotionChoice | currentPlayer) << " performed.\n";
			std::cout << "New FEN: " << getFENfromPos() << '\n';
		}

		return true;
	}

	short pieceFrom = getPiece(from[0], from[1]);
	short pieceTo = getPiece(to[0], to[1]);
	if (Piece::getType(pieceFrom) == Piece::NONE) {
		return false;
	}
	for (int i = 0; i < possibleMoves.size(); i++)
	{
		if (possibleMoves[i].startSquare != start)
			continue;

		unsigned short result = possibleMoves[i].targetSquare;
		if (result != target)
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

		doMove(&possibleMoves[i]);
		moveHistory.push(possibleMoves[i]);
		swapCurrentPlayer();
		generateMoves();

		if (debugLogs) {
			std::cout << "New FEN: " << getFENfromPos() << '\n';
		}

		futureMovesBuffer = std::stack<Move>();

		return true;
	}
	return false;
}

void Board::doMove(const Move* move) {
	PROFILE_FUNCTION();
	enPassantSquare = 64;
	const unsigned short from = move->startSquare;
	const unsigned short to = move->targetSquare;

	short pieceFrom = move->piece;
	short pieceTo = move->capturedPiece;
	short promoResult = move->getPromotionResult();
	setPiece(to, promoResult);
	removePiece(from);
	
	//std::cout << "\nBishops bitboard after " << Move::toString(*move) << ":\n" << bb.toString(bb.getBitboard(Piece::BISHOP | currentPlayer) | bb.getBitboard(Piece::BISHOP | Piece::getOppositeColor(currentPlayer)));

	
	// En passant 
	if (move->isEnPassant()) {
		removePiece(currentPlayer == Piece::WHITE ? to - 8 : to + 8);
	}
	else if (Piece::getType(pieceFrom) == Piece::PAWN && (abs(to - from) == 16)) {
		// Double pawn step
		enPassantSquare = from + ((to - from) / 2);
	}

	if ((Piece::getType(pieceFrom) == Piece::KING) && (abs(to-from) == 2)) {
		unsigned short rookFrom = to + (to - from) / ((to - from == 2) ? 2 : 1);
		unsigned short rookTo = from + (to - from) / 2;
		std::cout << "\n------------CASTLED!------------\n";
		// Castle detected, Rook has to be moved
		removePiece(rookFrom);
		setPiece(rookTo, Piece::ROOK | currentPlayer);
	}

	//----------- UPDATE CASTLE RIGHTS ---------------------
	if (castleRights != 0) {
		// If king moved, delete that player's castle rights
		if (Piece::getType(pieceFrom) == Piece::KING) {
			castleRights &= (currentPlayer == Piece::WHITE) ? 0b0011 : 0b1100;
		}
		// If rook moved
		else if (Piece::getType(pieceFrom) == Piece::ROOK) {
			if (currentPlayer == Piece::WHITE && bb.containsSquare(~bb.notFirstRank, from)) {
				if (from == 0) {
					// Remove right for white's long castle
					castleRights &= 0b1011;
				}
				else if (from == 7) {
					// Remove right for white's short castle
					castleRights &= 0b0111;
				}
			}
			else if (currentPlayer == Piece::BLACK && bb.containsSquare(~bb.notEightRank, from)) {
				if (from == 56) {
					// Remove right for black's long castle
					castleRights &= 0b1110;
				}
				else if (from == 63) {
					// Remove right for black's short castle
					castleRights &= 0b1101;
				}
			}
		}
		// If rook got captured
		else if (Piece::getType(pieceTo) == Piece::ROOK) {
			if (currentPlayer == Piece::BLACK && bb.containsSquare(~bb.notFirstRank, to)) {
				if (to == 0) {
					// Remove right for white's long castle
					castleRights &= 0b1011;
				}
				else if (to == 7) {
					// Remove right for white's short castle
					castleRights &= 0b0111;
				}
			}
			else if (currentPlayer == Piece::WHITE && bb.containsSquare(~bb.notEightRank, to)) {
				if (to == 56) {
					// Remove right for black's long castle
					castleRights &= 0b1110;
				}
				else if (to == 63) {
					// Remove right for black's short castle
					castleRights &= 0b1101;
				}
			}
		}
	}

	//----------- UPDATE KING POS --------------------------
	if (Piece::getType(pieceFrom) == Piece::KING) {
		if (currentPlayer == Piece::WHITE) {
			whiteKingPos = to;
		}
		else {
			blackKingPos = to;
		}
	}
}

void Board::doMove(std::string move) {
	unsigned short from, to;
	for (int i = 0; i < 63; i++) {
		if (move.substr(0, 2) == squareNames[i]) from = i;
		if (move.substr(2, 2) == squareNames[i]) to = i;
	}

	Move::Promotion promo = Move::Promotion::None;

	if (move.size() > 4) {
		switch (move[4]) {
		case 'q':
			promo = Move::Promotion::ToQueen;
			break;
		case 'r':
			promo = Move::Promotion::ToRook;
			break;
		case 'n':
			promo = Move::Promotion::ToKnight;
			break;
		case 'b':
			promo = Move::Promotion::ToBishop;
			break;
		}
	}

	short ep = 0;
	if ((Piece::getType(getPiece(from) == Piece::PAWN)) && (abs(to - from) == 7 || abs(to - from) == 9)) {
		ep = 0b1000;
	}

	Move m(getPiece(from), getPiece(to), from, to, promo | ep);
	doMove(&m);
}

void Board::undoMove(const Move* move) {
	PROFILE_FUNCTION();
	if (debugLogs) std::cout << "Trying to undo Move >>" << Move::toString(*move) << "<<\n";

	unsigned short target = move->targetSquare;

	// Place captured piece / clear target square
	if (move->isEnPassant()) {
		removePiece(target);
		setPiece(Piece::getColor(move->piece) == Piece::WHITE ? target - 8 : target + 8, move->capturedPiece);
	}
	else {
		setPiece(target, move->capturedPiece);
	}

	// Place piece back at startsquare
	setPiece(move->startSquare, move->piece);

	// Check if the king moved
	if (Piece::getType(move->piece) == Piece::KING) {
		if (Piece::getColor(move->piece) == Piece::WHITE) {
			whiteKingPos = move->startSquare;
		}
		else {
			blackKingPos = move->startSquare;
		}
		// If king castled
		if (abs(move->startSquare - target) == 2) {
			unsigned short from = move->startSquare + (target - move->startSquare) / 2;
			unsigned short to = target + (target - move->startSquare) / ((target - move->startSquare == 2) ? 2 : 1);
			// Castle detected, Rook has to be moved
			setPiece(to, Piece::ROOK | Piece::getColor(move->piece));
			removePiece(from);
		}
	}
	// Restore the castlerights and epsquare from before that move
	castleRights = move->previousCastlerights;
	enPassantSquare = move->previousEPsquare;
}

bool Board::undoLastMove()
{
	if (moveHistory.empty()) return false;

	Move* lastMove = &moveHistory.top();
	moveHistory.pop();
	undoMove(lastMove);
	futureMovesBuffer.push(*lastMove);
}

bool Board::redoLastMove() {
	if (futureMovesBuffer.empty()) return false;
	doMove(&futureMovesBuffer.top());
	moveHistory.push(futureMovesBuffer.top());
	futureMovesBuffer.pop();
	return true;
}

void Board::generateMoves()
{
	PROFILE_FUNCTION();
	if (debugLogs) std::cout << "\nGenerating possible moves ...\n";
	//Instrumentor::Get().BeginSession("Generate Moves Profiling", "moves.json");
	possibleMoves.clear();
	generateKnightMoves();
	generateKingMoves();
	generatePawnMoves();
	generateRookMoves();
	generateBishopMoves();
	generateQueenMoves();

	//pseudoLegalToLegalMoves();
	//Instrumentor::Get().EndSession();
}

void Board::generatePawnMoves() {
	PROFILE_FUNCTION();
	bool white = currentPlayer == Piece::WHITE;
	//---------- Moves one step ahead -----------------
	bitboard moves = bb.getSinglePawnSteps(currentPlayer);
	// Pawns may only step on empty fields
	bitboard empty = bb.getEmpty();
	moves &= empty;
	unsigned short targetIndex = 0;
	// Loop over all pawns that can move one step ahead
	while (moves) {
		targetIndex = bb.pop(&moves);
		unsigned short originIndex = targetIndex + (white ? -8 : 8);

		if (bb.isPinned(originIndex, currentPlayer)) continue;

		short promotionFlag = (white && (targetIndex > 54)) ||
							 (!white && (targetIndex < 8));
		
		Move move(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex, promotionFlag);
		possibleMoves.push_back(move);

		if (promotionFlag) {
			// Add all other possible promotions
			for (int i = 2; i < 5; i++) {
				Move promoMove(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex, i);
				possibleMoves.push_back(move);
			}
		}
	}

	//---------- Moves two steps ahead ----------------
	moves = bb.getDoublePawnSteps(currentPlayer);
	// Target field must be empty
	moves &= empty;
	// Previous field must also be empty
	moves &= white ? (empty << 8) : (empty >> 8);
	// Loop over all pawns that can move two steps ahead
	while (moves) {
		targetIndex = bb.pop(&moves);
		unsigned short originIndex = targetIndex + (white ? -16 : 16);

		if (bb.isPinned(originIndex, currentPlayer)) continue;

		Move move(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex);
		possibleMoves.push_back(move);
	}

	//---------- Captures left ------------------------
	moves = bb.getPawnAttacks(true, currentPlayer);
	// Capture field has to be occupied by enemy or marked as ep square
	moves &= bb.getBitboard(Piece::getOppositeColor(currentPlayer)) | (1ULL << enPassantSquare);
	// Loop over all pawn captures to the left
	while (moves) {
		targetIndex = bb.pop(&moves);
		unsigned short originIndex = targetIndex + (white ? -7 : 9);

		if (bb.isPinned(originIndex, currentPlayer)) continue;


		short promotionFlag = (white && (1ULL << targetIndex & ~bb.notEightRank)) ||
			(!white && (1ULL << targetIndex & ~bb.notFirstRank));
		short epFlag = 0;
		if (targetIndex == enPassantSquare) {
			epFlag |= 0b1000;
		}
		Move move(Piece::PAWN | currentPlayer, getPiece(targetIndex), originIndex, targetIndex, promotionFlag | epFlag);
		possibleMoves.push_back(move);

		if (promotionFlag) {
			// Add all other possible promotions
			for (int i = 2; i < 5; i++) {
				Move promoMove(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex, i | epFlag);
				possibleMoves.push_back(move);
			}
		}
	}

	//---------- Captures right -----------------------
	moves = bb.getPawnAttacks(false, currentPlayer);
	// Capture field has to be occupied by enemy or marked as ep square
	moves &= bb.getBitboard(Piece::getOppositeColor(currentPlayer)) | (1ULL << enPassantSquare);
	// Loop over all pawn captures to the right
	while (moves) {
		targetIndex = bb.pop(&moves);
		unsigned short originIndex = targetIndex + (white ? -9 : 7);

		if (bb.isPinned(originIndex, currentPlayer)) continue;


		short promotionFlag = (white && (1ULL << targetIndex & ~bb.notEightRank)) ||
			(!white && (1ULL << targetIndex & ~bb.notFirstRank));
		short epFlag = 0;
		if (targetIndex == enPassantSquare) {
			epFlag |= 0b1000;
		}
		Move move(Piece::PAWN | currentPlayer, getPiece(targetIndex), originIndex, targetIndex, promotionFlag | epFlag);
		possibleMoves.push_back(move);

		if (promotionFlag) {
			// Add all other possible promotions
			for (int i = 2; i < 5; i++) {
				Move promoMove(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex, i | epFlag);
				possibleMoves.push_back(move);
			}
		}
	}
}

void Board::generateKingMoves() {
	PROFILE_FUNCTION();
	unsigned short kingPos = (currentPlayer == Piece::WHITE) ? whiteKingPos : blackKingPos;
	bitboard kingMoves = bb.getKingAttacks(kingPos, true);
	// Don't move to squares occupied by your own color
	kingMoves &= ~bb.getBitboard(currentPlayer);

	// Index of the current move
	unsigned short targetIndex = 0;
	while (kingMoves) {
		// Increase index
		targetIndex = bb.pop(&kingMoves);

		bool castleFailed = false;
		if (targetIndex - kingPos == 2) {
			// Short castle
			if (currentPlayer == Piece::WHITE && (castleRights & 0b1000)) {
				// Check white's short castle
				// Two squares next to king have to be empty and not attacked
				castleFailed |= (bb.OO & (bb.getOccupied() | bb.getAllAttacks(Piece::getOppositeColor(currentPlayer))));
			}
			else if (castleRights & 0b0010) {
				// Check black's short castle
				// Two squares next to king have to be empty and not attacked
				castleFailed |= (bb.oo & (bb.getOccupied() | bb.getAllAttacks(Piece::getOppositeColor(currentPlayer))));
			}
			else {
				castleFailed = true;
			}

			if (!castleFailed) {
				// Rook has to be on the right square
				castleFailed |= !(getPiece(targetIndex + 1) == (Piece::ROOK | currentPlayer));
			}
		}
		else if (targetIndex - kingPos == -2) {
			// Long castle
			if (currentPlayer == Piece::WHITE && (castleRights & 0b0100)) {
				// Check white's long castle
				// Three squares next to king have to be empty and not attacked
				castleFailed |= (bb.OOO & (bb.getOccupied() | bb.getAllAttacks(Piece::getOppositeColor(currentPlayer))));
			}
			else if (castleRights & 0b0001) {
				// Check black's long castle
				// Three squares next to king have to be empty and not attacked
				castleFailed |= (bb.ooo & (bb.getOccupied() | bb.getAllAttacks(Piece::getOppositeColor(currentPlayer))));
			}
			else castleFailed = true;

			if (!castleFailed) {
				// Rook has to be on the right square
				castleFailed = !(getPiece(targetIndex - 2) == (Piece::ROOK | currentPlayer));
			}
		}
		
		if (!castleFailed) {
			Move move(Piece::KING | currentPlayer, getPiece(targetIndex), kingPos, targetIndex);
			possibleMoves.push_back(move);
		}
	}
}

void Board::generateKnightMoves() {
	PROFILE_FUNCTION();
	bitboard knights = bb.getBitboard(Piece::KNIGHT | currentPlayer);
	unsigned short knightPos = 0;
	while (knights) {
		// Get the index of next knight
		knightPos = bb.pop(&knights);

		if (bb.isPinned(knightPos, currentPlayer)) continue;

		bitboard knightMoves = bb.getKnightAttacks(knightPos);
		// Possible Knight moves can't go on squares occupied by own color
		knightMoves &= ~(bb.getBitboard(currentPlayer));

		// Index of the current move
		unsigned short targetIndex = 0;
		while (knightMoves) {
			// Increase index
			targetIndex = bb.pop(&knightMoves);

			Move move(Piece::KNIGHT | currentPlayer, getPiece(targetIndex), knightPos, targetIndex);
			possibleMoves.push_back(move);
		}
	}
}

void Board::generateRookMoves() {
	PROFILE_FUNCTION();
	bitboard rooks = bb.getBitboard(Piece::ROOK | currentPlayer);
	unsigned short rookPos = 0;
	while (rooks) {
		rookPos = bb.pop(&rooks);

		if (debugLogs) std::cout << "\nGenerating Moves for Rook on " << getSquareName(rookPos) << "...\n";

		if (bb.isPinned(rookPos, currentPlayer)) continue;

		bitboard rookAttacks = bb.getRookAttacks(rookPos);
		// Remove squares that are blocked by friendly pieces
		rookAttacks &= ~bb.getBitboard(currentPlayer);

		if (debugLogs) std::cout << "\nRook Attacks Bitboard:\n" << bb.toString(rookAttacks);

		unsigned short targetIndex = 0;
		while (rookAttacks) {
			targetIndex = bb.pop(&rookAttacks);

			Move move(Piece::ROOK | currentPlayer, getPiece(targetIndex), rookPos, targetIndex);
			possibleMoves.push_back(move);
		}
	}
}

void Board::generateBishopMoves() {
	PROFILE_FUNCTION();
	bitboard bishops = bb.getBitboard(Piece::BISHOP | currentPlayer);
	unsigned short bishopPos = 0;
	while (bishops) {
		bishopPos = bb.pop(&bishops);

		if (debugLogs) std::cout << "\nGenerating Moves for Bishop on " << getSquareName(bishopPos) << "...\n";

		if (bb.isPinned(bishopPos, currentPlayer)) continue;

		bitboard bishopAttacks = bb.getBishopAttacks(bishopPos);
		// Remove squares that are blocked by friendly pieces
		bishopAttacks &= ~bb.getBitboard(currentPlayer);

		if (debugLogs) std::cout << "\Bishop Attacks Bitboard:\n" << bb.toString(bishopAttacks);

		unsigned short targetIndex = 0;
		while (bishopAttacks) {
			targetIndex = bb.pop(&bishopAttacks);

			Move move(Piece::BISHOP | currentPlayer, getPiece(targetIndex), bishopPos, targetIndex);
			possibleMoves.push_back(move);
		}
	}
}

void Board::generateQueenMoves() {
	PROFILE_FUNCTION();
	bitboard queens = bb.getBitboard(Piece::QUEEN | currentPlayer);
	unsigned short queenPos = 0;
	while (queens) {
		queenPos = bb.pop(&queens);

		if (debugLogs) std::cout << "\nGenerating Moves for Queen on " << getSquareName(queenPos) << "...\n";

		if (bb.isPinned(queenPos, currentPlayer)) continue;

		bitboard queenAttacks = bb.getRookAttacks(queenPos) | bb.getBishopAttacks(queenPos);
		// Remove squares that are blocked by friendly pieces
		queenAttacks &= ~bb.getBitboard(currentPlayer);

		if (debugLogs) std::cout << "\Queen Attacks Bitboard:\n" << bb.toString(queenAttacks);

		unsigned short targetIndex = 0;
		while (queenAttacks) {
			targetIndex = bb.pop(&queenAttacks);

			Move move(Piece::QUEEN | currentPlayer, getPiece(targetIndex), queenPos, targetIndex);
			possibleMoves.push_back(move);
		}
	}
}

void Board::pseudoLegalToLegalMoves() {
	PROFILE_FUNCTION();
	std::vector<Move> legalMoves;
	for (int i = 0; i < possibleMoves.size(); i++) {
		Move* move = &possibleMoves[i];
		doMove(move);
		if (!kingIsInCheck(currentPlayer)) {
			legalMoves.push_back(*move);
		}
		undoMove(move);
	}
	possibleMoves = legalMoves;
}

bool Board::kingIsInCheck(const short color) {
	PROFILE_FUNCTION();
	bitboard king = bb.getBitboard(Piece::KING | color);
	return king & bb.getAllAttacks(Piece::getOppositeColor(color));
}

// Converts an integer (step) to a short[2] x and y direction
void Board::stepsToDirection(int steps, short dir[2]) {
	//std::cout << "Converting steps " << steps << " to direction... ";
	dir[0] = 0;
	dir[1] = 0;
	int i = 0;
	while (steps) {
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

std::string Board::getSquareName(unsigned short index) {
	if (index > 63) return "";
	return squareNames[index];
}

int Board::testMoveGeneration(unsigned int depth, bool divide) {
	PROFILE_FUNCTION();
	if (depth == 1) return possibleMoves.size();
	int positionCount = 0;
	std::vector<Move> moves = possibleMoves;

	for (int i = 0; i < possibleMoves.size(); i++) {
		// COPY!!! IMPORTANT, because possibleMoves will change
		Move move = possibleMoves[i];
		doMove(&move);
		swapCurrentPlayer();
		//float time;
		{
			//Timer timer("Board::generateMoves()", &time);
			generateMoves();
		}
		//accumulatedGenerationTime += time;
		int positionsAfterMove = testMoveGeneration(depth - 1, false);
		positionCount += positionsAfterMove;
		undoMove(&move);
		swapCurrentPlayer();
		possibleMoves = moves;
		if (divide) {
			std::cout << Move::toString(possibleMoves[i]) << ": " << std::to_string(positionsAfterMove) << '\n';
			std::string dump;
			//std::cin >> dump;
		}
	}
	futureMovesBuffer = std::stack<Move>();
	return positionCount;
}
