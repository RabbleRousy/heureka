#include "Board.h"
#include "Piece.h"
#include <iostream>
#include <string>
#include "Timer.h"

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
		squares[i] = Piece::NONE;
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

void Board::doMove(const Move* move)
{
	enPassantSquare = 64;
	const unsigned short from = move->startSquare;
	const unsigned short to = move->targetSquare;

	short pieceFrom = move->piece;
	short pieceTo = move->capturedPiece;
	short promoResult = move->getPromotionResult();
	setPiece(to, promoResult);
	removePiece(from);

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

void Board::undoMove(const Move* move)
{
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
	if (debugLogs) std::cout << "\nGenerating possible moves ...\n";
	possibleMoves.clear();
	generateKnightMoves();
	generateKingMoves();
	generatePawnMoves();
	generateRookMoves();
	generateBishopMoves();
	generateQueenMoves();
}

void Board::generatePawnMoves() {
	//---------- Moves one step ahead -----------------
	bitboard moves = bb.getSinglePawnSteps(currentPlayer);
	// Pawns may only step on empty fields
	moves &= bb.getEmpty();
	unsigned short targetIndex = 0;
	while (moves) {
		targetIndex += bb.pop(&moves);
		unsigned short originIndex = targetIndex + ((currentPlayer == Piece::WHITE) ? -8 : 8);
		short promotionFlag = ((currentPlayer == Piece::WHITE) && (targetIndex > 54)) ||
							 ((currentPlayer == Piece::BLACK) && (targetIndex < 8));
		
		Move move(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex, promotionFlag);
		possibleMoves.push_back(move);

		if (promotionFlag) {
			// Add all other possible promotions
			for (int i = 2; i < 5; i++) {
				Move promoMove(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex, i);
				possibleMoves.push_back(move);
			}
		}

		targetIndex++;
	}

	//---------- Moves two steps ahead ----------------
	moves = bb.getDoublePawnSteps(currentPlayer);
	// Target field must be empty
	moves &= bb.getEmpty();
	// Previous field must also be empty
	moves &= (currentPlayer == Piece::WHITE) ? (bb.getEmpty() << 8) : (bb.getEmpty() >> 8);
	targetIndex = 0;
	while (moves) {
		targetIndex += bb.pop(&moves);
		unsigned short originIndex = targetIndex + ((currentPlayer == Piece::WHITE) ? -16 : 16);
		Move move(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex);
		possibleMoves.push_back(move);
		targetIndex++;
	}

	//---------- Captures left ------------------------
	moves = bb.getPawnAttacks(true, currentPlayer);
	// Capture field has to be occupied by enemy or marked as ep square
	moves &= bb.getBitboard(Piece::getOppositeColor(currentPlayer)) | (1ULL << enPassantSquare);
	targetIndex = 0;
	while (moves) {
		targetIndex += bb.pop(&moves);
		unsigned short originIndex = targetIndex + ((currentPlayer == Piece::WHITE) ? -7 : 9);
		short promotionFlag = ((currentPlayer == Piece::WHITE) && (1ULL << targetIndex & ~bb.notEightRank)) ||
			((currentPlayer == Piece::BLACK) && (1ULL << targetIndex & ~bb.notFirstRank));
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

		targetIndex++;
	}

	//---------- Captures right -----------------------
	moves = bb.getPawnAttacks(false, currentPlayer);
	// Capture field has to be occupied by enemy or marked as ep square
	moves &= bb.getBitboard(Piece::getOppositeColor(currentPlayer)) | (1ULL << enPassantSquare);
	targetIndex = 0;
	while (moves) {
		targetIndex += bb.pop(&moves);
		unsigned short originIndex = targetIndex + ((currentPlayer == Piece::WHITE) ? -9 : 7);
		short promotionFlag = ((currentPlayer == Piece::WHITE) && (1ULL << targetIndex & ~bb.notEightRank)) ||
			((currentPlayer == Piece::BLACK) && (1ULL << targetIndex & ~bb.notFirstRank));
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

		targetIndex++;
	}
}

void Board::generateKingMoves() {
	unsigned short kingPos = (currentPlayer == Piece::WHITE) ? whiteKingPos : blackKingPos;
	bitboard kingMoves = bb.getKingAttacks(kingPos);
	// King can only move to squares that are not attacked by enemy pieces
	kingMoves &= ~bb.getAllAttacks(Piece::getOppositeColor(currentPlayer));
	// Only move to empty or enemy squares
	kingMoves &= (bb.getEmpty() | bb.getBitboard(Piece::getOppositeColor(currentPlayer)));

	// Index of the current move
	unsigned short targetIndex = 0;
	while (kingMoves) {
		// Increase index
		targetIndex += bb.pop(&kingMoves);

		bool castleFailed = false;
		if (targetIndex - kingPos == 2) {
			// Short castle
			castleFailed = !(getPiece(targetIndex + 1) == (Piece::ROOK | currentPlayer));
			if (currentPlayer == Piece::WHITE) {
				// Check white's short castle
				castleFailed |= !(castleRights & 0b1000);
			}
			else {
				// Check black's short castle
				castleFailed |= !(castleRights & 0b0010);
			}
		}
		else if (targetIndex - kingPos == -2) {
			// Long castle
			castleFailed = !(getPiece(targetIndex - 2) == (Piece::ROOK | currentPlayer));
			if (currentPlayer == Piece::WHITE) {
				// Check white's long castle
				castleFailed |= !(castleRights & 0b0100);
			}
			else {
				// Check black's long castle
				castleFailed |= !(castleRights & 0b0001);
			}
		}
		
		if (!castleFailed) {
			Move move(Piece::KING | currentPlayer, getPiece(targetIndex), kingPos, targetIndex);
			possibleMoves.push_back(move);
		}

		// Skip current 1
		targetIndex++;
	}
}

void Board::generateKnightMoves() {
	bitboard knights = bb.getBitboard(Piece::KNIGHT | currentPlayer);
	unsigned short knightPos = 0;
	while (knights) {
		// Get the index of next knight
		knightPos += bb.pop(&knights);

		bitboard knightMoves = bb.getKnightAttacks(knightPos);
		// Possible Knight moves either go to an empty square or capture an opponent's piece
		knightMoves &= (bb.getEmpty() | bb.getBitboard(Piece::getOppositeColor(currentPlayer)));

		// Index of the current move
		unsigned short targetIndex = 0;
		while (knightMoves) {
			// Increase index
			targetIndex += bb.pop(&knightMoves);

			Move move(Piece::KNIGHT | currentPlayer, getPiece(targetIndex), knightPos, targetIndex);
			possibleMoves.push_back(move);

			// Skip current 1
			targetIndex++;
		}
		knightPos++;
	}
}

void Board::generateRookMoves() {
	bitboard rooks = bb.getBitboard(Piece::ROOK | currentPlayer);
	unsigned short rookPos = 0;
	while (rooks) {
		rookPos += bb.pop(&rooks);

		if (debugLogs) std::cout << "\nGenerating Moves for Rook on " << getSquareName(rookPos) << "...\n";

		bitboard rookAttacks = bb.getRookAttacks(rookPos);
		// Remove squares that are blocked by friendly pieces
		rookAttacks &= ~bb.getBitboard(currentPlayer);

		if (debugLogs) std::cout << "\nRook Attacks Bitboard:\n" << bb.toString(rookAttacks);

		unsigned short targetIndex = 0;
		while (rookAttacks) {
			targetIndex += bb.pop(&rookAttacks);

			Move move(Piece::ROOK | currentPlayer, getPiece(targetIndex), rookPos, targetIndex);
			possibleMoves.push_back(move);

			targetIndex++;
		}
		rookPos++;
	}
}

void Board::generateBishopMoves() {
	bitboard bishops = bb.getBitboard(Piece::BISHOP | currentPlayer);
	unsigned short bishopPos = 0;
	while (bishops) {
		bishopPos += bb.pop(&bishops);

		if (debugLogs) std::cout << "\nGenerating Moves for Bishop on " << getSquareName(bishopPos) << "...\n";

		bitboard bishopAttacks = bb.getBishopAttacks(bishopPos);
		// Remove squares that are blocked by friendly pieces
		bishopAttacks &= ~bb.getBitboard(currentPlayer);

		if (debugLogs) std::cout << "\Bishop Attacks Bitboard:\n" << bb.toString(bishopAttacks);

		unsigned short targetIndex = 0;
		while (bishopAttacks) {
			targetIndex += bb.pop(&bishopAttacks);

			Move move(Piece::BISHOP | currentPlayer, getPiece(targetIndex), bishopPos, targetIndex);
			possibleMoves.push_back(move);

			targetIndex++;
		}
		bishopPos++;
	}
}

void Board::generateQueenMoves() {
	bitboard queens = bb.getBitboard(Piece::QUEEN | currentPlayer);
	unsigned short queenPos = 0;
	while (queens) {
		queenPos += bb.pop(&queens);

		if (debugLogs) std::cout << "\nGenerating Moves for Queen on " << getSquareName(queenPos) << "...\n";

		bitboard queenAttacks = bb.getRookAttacks(queenPos) | bb.getBishopAttacks(queenPos);
		// Remove squares that are blocked by friendly pieces
		queenAttacks &= ~bb.getBitboard(currentPlayer);

		if (debugLogs) std::cout << "\Queen Attacks Bitboard:\n" << bb.toString(queenAttacks);

		unsigned short targetIndex = 0;
		while (queenAttacks) {
			targetIndex += bb.pop(&queenAttacks);

			Move move(Piece::QUEEN | currentPlayer, getPiece(targetIndex), queenPos, targetIndex);
			possibleMoves.push_back(move);

			targetIndex++;
		}
		queenPos++;
	}
}

bool Board::tryAddMove(const unsigned short x, const unsigned short y, int steps, bool canCapture, unsigned short target[2], bool* illegalBecauseCheck)
{
	short dir[2];
	stepsToDirection(steps, dir);
	short moveColor = Piece::getColor(getPiece(x, y));

	bool returnTarget = !(target == NULL);

	if (!returnTarget) {
		target = new unsigned short[2];
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
					enPassant = (enPassantSquare == target[0] + target[1] * 8);
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

	Move move(getPiece(x, y), capture, x + y * 8, target[0] + target[1] * 8, flags);

	// If king tries to move more than one square, check for castling
	if (Piece::getType(getPiece(x, y)) == Piece::KING && steps > 8) {
		move.targetSquare = x + y * 8 + int(dir[0] / 2);
		if (kingInCheckAfter(&move))
			// Castling is interrupted on the first step
			return false;
		move.targetSquare = target[0] + target[1] * 8;
	}

	if (!kingInCheckAfter(&move)) {

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
	short kingPos[2];
	kingPos[0] = (color == Piece::WHITE) ? whiteKingPos % 8 : blackKingPos % 8;
	kingPos[1] = (color == Piece::WHITE) ? whiteKingPos / 8 : blackKingPos / 8;
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

bool Board::kingInCheckAfter(const Move* move)
{
	if (debugLogs) std::cout << "Is king in check after " << Move::toString(*move) << "? ";
	// Do the move
	doMove(move);

	// Is king in check now?
	bool check = kingIsInCheck(currentPlayer);
	
	// Undo move
	undoMove(move);

	if (debugLogs) std::cout << (check ? "Yes." : "No.") << "\n";
	return check;
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

int Board::testMoveGeneration(unsigned int depth, bool divide)
{
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
		}
	}
	futureMovesBuffer = std::stack<Move>();
	return positionCount;
}
