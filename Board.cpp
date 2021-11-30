#include "Board.h"
#include "Piece.h"
#include <iostream>
#include <string>
#include "Profiling.h"
#include "Zobrist.h"
#include "TranspositionTable.h"

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
unsigned short Board::fullMoveCount = 1;
unsigned short Board::halfMoveCount = 0;
Bitboard Board::bb = Bitboard();

Board::Board() : possibleMoves(), moveHistory(), futureMovesBuffer(), wantsToPromote(false), currentPlayer(Piece::WHITE) { 
	Zobrist::initializeHashes();
	TranspositionTable::setSize(128);
}


void Board::clearBoard() {
	for (int i = 0; i < 64; i++) {
		removePiece(i);
	}
}

void Board::reset() {
	possibleMoves = std::vector<Move>();
	moveHistory = std::stack<Move>();
	futureMovesBuffer = std::stack<Move>();
	halfMoveCount = 0;
	fullMoveCount = 1;
	wantsToPromote = false;
	checkMate = false;
	remis = false;
	TranspositionTable::clear();
	readPosFromFEN();
	generateMoves();
}

void Board::init(std::string fen) {
	if (!readPosFromFEN(fen)) {
		readPosFromFEN();
	}
	currentZobristKey = Zobrist::getZobristKey(&bb, castleRights, enPassantSquare, currentPlayer == Piece::WHITE);
	DEBUG_COUT("Zobrist key for this position: " + std::to_string(currentZobristKey) + '\n');
	generateMoves();
}

bool Board::readPosFromFEN(std::string fen) {

	DEBUG_COUT("Trying to parse FEN: " + fen + '\n');
	clearBoard();
	// Default values
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

		DEBUG_COUT("Parsing " + std::to_string(fen[i]) + " ... \n");

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
		DEBUG_COUT("End of FEN reached.\n");
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
	DEBUG_COUT("Current player read from FEN: " + fen[i] + '\n');

	// Castling rights
	castleRights = 0;
	int j = i+2;
	for (j; j < (i + 6); j++) {
		if (j == fen.size()) break;

		switch (fen[j]) {
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
	}

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
		DEBUG_COUT("EP Capture input: " + fen[i] + '\n');
	}

	// Parsing failed
	if (column == 8 || !(row == 2 || row == 5)) {
		DEBUG_COUT("EP capture parsing failed\n");
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
	
	fen += ' ';

	// En passant captures
	if (enPassantSquare != 64) {
		fen += getSquareName(enPassantSquare);
	}
	else {
		fen +=  '-';
	}

	fen += ' ' + std::to_string(halfMoveCount) + ' ' + std::to_string(fullMoveCount);

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
	
	DEBUG_COUT("Handling move input from " + getSquareName(start) + " to " + getSquareName(target) + " ...\n");

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

		makePlayerMove(&promoMoveBuffer);

		DEBUG_COUT("Promotion to " + Piece::name(promotionChoice | currentPlayer) + " performed.\n");
		DEBUG_COUT("New FEN: " + getFENfromPos() + '\n');

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

		DEBUG_COUT("Move found in possibleMoves list. Checking if it's promotion ...");

		// Promotion
		if (possibleMoves[i].isPromotion()) {
			DEBUG_COUT(" YES. Setting wantsToPromote flag.\n");
			// Promotion needs to be completed by player
			wantsToPromote = true;
			promoMoveBuffer = possibleMoves[i];
			// Correct promotion move will be added afterwards
			return false;
		}
		DEBUG_COUT(" NO. Doing move: " + Move::toString(possibleMoves[i]) + '\n');

		makePlayerMove(&possibleMoves[i]);

		DEBUG_COUT("New FEN: " + getFENfromPos() + '\n');

		return true;
	}
	return false;
}

bool Board::checkForMateOrRemis() {
	if (checkForRepetition()) {
		remis = true;
		return true;
	}

	generateMoves();
	if (possibleMoves.size() == 0) {
		if (attackData.checkExists) {
			checkMate = true;
		}
		else {
			remis = true;
		}
		return true;
	}

	if (halfMoveCount >= 100) {
		remis = true;
	}

	return remis | checkMate;
}

bool Board::checkForRepetition() {
	if (halfMoveCount > 7) {
		unsigned int halfMovesPlayed = positionHistory.size();
		unsigned short repetitions = 0;

		for (int i = halfMovesPlayed - 4; i >= int(halfMovesPlayed - halfMoveCount); i -= 4) {
			repetitions += (positionHistory[i] == currentZobristKey);
		}
		if (repetitions >= 2) {
			return true;
		}
	}
	return false;
}

void Board::makePlayerMove(const Move* move) {
	doMove(move);
	
	moveHistory.push(*move);
	futureMovesBuffer = std::stack<Move>();

	checkForMateOrRemis();
}

void Board::makeAiMove() {
	currentSearch = iterativeSearch(searchTime);
	doMove(&currentSearch.bestMove);

	moveHistory.push(currentSearch.bestMove);
	futureMovesBuffer = std::stack<Move>();

	checkForMateOrRemis();
}

void Board::doMove(const Move* move) {
	PROFILE_FUNCTION();
	// Save the old position
	positionHistory.push_back(currentZobristKey);

	unsigned short oldEpSquare = enPassantSquare;
	enPassantSquare = 64;
	const unsigned short from = move->startSquare;
	const unsigned short to = move->targetSquare;

	short pieceFrom = move->piece;
	short pieceTo = move->capturedPiece;
	short promoResult = move->getPromotionResult();

	if (currentPlayer == Piece::BLACK)
		fullMoveCount++;

	halfMoveCount++;
	if (Piece::getType(pieceTo) != Piece::NONE ||
		Piece::getType(pieceFrom) == Piece::PAWN) {
		// If there was a capture or pawn move, halfmove clock is resetted
		halfMoveCount = 0;
	}

	setPiece(to, promoResult);
	if ((Piece::getType(pieceTo) != Piece::NONE) && !move->isEnPassant()) {
		// Remove captured piece from hash
		Zobrist::updatePieceHash(currentZobristKey, pieceTo, to);
	}
	// Add piece to hash on target square
	Zobrist::updatePieceHash(currentZobristKey, promoResult, to);
	
	removePiece(from);

	// Remove piece from hash at origin position
	Zobrist::updatePieceHash(currentZobristKey, pieceFrom, from);
	
	//std::cout << "\nBishops bitboard after " << Move::toString(*move) << ":\n" << bb.toString(bb.getBitboard(Piece::BISHOP | currentPlayer) | bb.getBitboard(Piece::BISHOP | Piece::getOppositeColor(currentPlayer)));

	
	// En passant 
	if (move->isEnPassant()) {
		unsigned short square = currentPlayer == Piece::WHITE ? to - 8 : to + 8;
		removePiece(square);
		// Remove captured pawn from hash
		Zobrist::updatePieceHash(currentZobristKey, pieceTo, square);
	}
	else if (Piece::getType(pieceFrom) == Piece::PAWN && (abs(to - from) == 16)) {
		// Double pawn step
		enPassantSquare = from + ((to - from) / 2);
	}
	// Update Zobrist Key w.r.t. ep Square (might have changed or dissapeared)
	Zobrist::updateZobristKey(currentZobristKey, oldEpSquare, enPassantSquare);

	if ((Piece::getType(pieceFrom) == Piece::KING) && (abs(to-from) == 2)) {
		unsigned short rookFrom = to + (to - from) / ((to - from == 2) ? 2 : 1);
		unsigned short rookTo = from + (to - from) / 2;
		// Castle detected, Rook has to be moved
		removePiece(rookFrom);
		Zobrist::updatePieceHash(currentZobristKey, Piece::ROOK | currentPlayer, rookFrom);
		setPiece(rookTo, Piece::ROOK | currentPlayer);
		Zobrist::updatePieceHash(currentZobristKey, Piece::ROOK | currentPlayer, rookTo);
	}

	//----------- UPDATE CASTLE RIGHTS ---------------------
	if (castleRights != 0) {
		short oldCastleRights = castleRights;
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
		Zobrist::updateZobristKey(currentZobristKey, oldCastleRights, castleRights);
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

	swapCurrentPlayer();
	//printPositionHistory();
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
	//if (debugLogs) std::cout << "Trying to undo Move >>" << Move::toString(*move) << "<<\n";

	unsigned short target = move->targetSquare;

	// Place captured piece / clear target square
	if (move->isEnPassant()) {
		removePiece(target);
		Zobrist::updatePieceHash(currentZobristKey, move->piece, target);
		unsigned short capturedPawnSquare = Piece::getColor(move->piece) == Piece::WHITE ? target - 8 : target + 8;
		setPiece(capturedPawnSquare, move->capturedPiece);
		Zobrist::updatePieceHash(currentZobristKey, move->capturedPiece, capturedPawnSquare);
	}
	else {
		setPiece(target, move->capturedPiece);
		if (Piece::getType(move->capturedPiece) != Piece::NONE) {
			// Add captured piece back to the hash
			Zobrist::updatePieceHash(currentZobristKey, move->capturedPiece, target);
		}
		// Remove moved piece from the hash at target pos
		Zobrist::updatePieceHash(currentZobristKey, move->getPromotionResult(), target);
	}

	// Place piece back at startsquare
	setPiece(move->startSquare, move->piece);
	Zobrist::updatePieceHash(currentZobristKey, move->piece, move->startSquare);

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
			short rook = Piece::ROOK | Piece::getColor(move->piece);
			// Castle detected, Rook has to be moved
			setPiece(to, rook);
			Zobrist::updatePieceHash(currentZobristKey, rook, to);
			removePiece(from);
			Zobrist::updatePieceHash(currentZobristKey, rook, from);
		}
	}

	// Restore the castlerights and epsquare from before that move
	Zobrist::updateZobristKey(currentZobristKey, castleRights, move->previousCastlerights);
	Zobrist::updateZobristKey(currentZobristKey, enPassantSquare, move->previousEPsquare);
	castleRights = move->previousCastlerights;
	enPassantSquare = move->previousEPsquare;
	halfMoveCount = move->previousHalfMoves;

	if (currentPlayer == Piece::WHITE) {
		// Black's move was undone
		fullMoveCount--;
	}

	swapCurrentPlayer();

	positionHistory.pop_back();
	//printPositionHistory();
}

bool Board::undoLastMove()
{
	if (moveHistory.empty()) return false;

	Move* lastMove = &moveHistory.top();
	moveHistory.pop();
	undoMove(lastMove);
	futureMovesBuffer.push(*lastMove);

	return true;
}

bool Board::redoLastMove() {
	if (futureMovesBuffer.empty()) return false;
	doMove(&futureMovesBuffer.top());
	moveHistory.push(futureMovesBuffer.top());
	futureMovesBuffer.pop();

	return true;
}

bool Board::inCheckAfter(const Move* move) {
	doMove(move);
	bool check = bb.getAttackData(Piece::getOppositeColor(currentPlayer)).checkExists;
	undoMove(move);
	return check;
}

void Board::generateMoves(bool onlyCaptures)
{
	PROFILE_FUNCTION();
	//if (debugLogs) std::cout << "\nGenerating possible moves ...\n";
	//Instrumentor::Get().BeginSession("Generate Moves Profiling", "moves.json");
	possibleMoves.clear();

	attackData = bb.getAttackData(currentPlayer);

	generateKingMoves(onlyCaptures);

	if (attackData.doubleCheck) return;

	generatePawnMoves(onlyCaptures);
	generateKnightMoves(onlyCaptures);
	generateBishopMoves(onlyCaptures);
	generateRookMoves(onlyCaptures);
	generateQueenMoves(onlyCaptures);

	//pseudoLegalToLegalMoves();
	//Instrumentor::Get().EndSession();
}

void Board::generatePawnMoves(bool onlyCaptures) {
	PROFILE_FUNCTION();
	bool white = currentPlayer == Piece::WHITE;
	bitboard pawns = bb.getBitboard(Piece::PAWN | currentPlayer);
	// Pinned pawns can't move if there is a check
	pawns &= pawns ^ (attackData.checkExists * attackData.allPins);

	bitboard moves;
	bitboard empty = bb.getEmpty();
	unsigned short targetIndex = 0;

	if (onlyCaptures) goto captures;

	//---------- Moves one step ahead -----------------
	moves = bb.getSinglePawnSteps(pawns, currentPlayer);
	// Pawns may only step on empty fields
	moves &= empty;

	// If player is in check, pawns may only step inbetween the check ray
	moves &= attackData.allChecks;
	// Loop over all pawns that can move one step ahead
	Bitloop (moves) {
		targetIndex = getSquare(moves);
		unsigned short originIndex = targetIndex + (white ? -8 : 8);
		
		// If not moving along existing pinray, skip Move
		if (!bb.containsSquare(attackData.pins[originIndex], targetIndex)) continue;

		short promotionFlag = (white && (targetIndex > 55)) ||
							 (!white && (targetIndex < 8));
		
		Move move(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex, promotionFlag);
		possibleMoves.push_back(move);

		if (promotionFlag) {
			// Add all other possible promotions
			for (int i = 2; i < 5; i++) {
				Move promoMove(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex, i);
				possibleMoves.push_back(promoMove);
			}
		}
	}

	//---------- Moves two steps ahead ----------------
	moves = bb.getDoublePawnSteps(pawns, currentPlayer);
	// Target field must be empty
	moves &= empty;
	// Previous field must also be empty
	moves &= white ? (empty << 8) : (empty >> 8);

	// If player is in check, pawns may only step inbetween the check ray
	moves &= attackData.allChecks;

	// Loop over all pawns that can move two steps ahead
	Bitloop (moves) {
		targetIndex = getSquare(moves);
		unsigned short originIndex = targetIndex + (white ? -16 : 16);

		// Pinned piece can only move on pin ray
		if (!bb.containsSquare(attackData.pins[originIndex], targetIndex)) continue;

		Move move(Piece::PAWN | currentPlayer, Piece::NONE, originIndex, targetIndex);
		possibleMoves.push_back(move);
	}

	captures:
	//---------- Captures left ------------------------
	moves = bb.getPawnAttacks(pawns, true, currentPlayer);
	// Capture field has to be occupied by enemy or marked as ep square
	bitboard captures = bb.getBitboard(Piece::getOppositeColor(currentPlayer));
	// Or marked as en passant
	captures |= bitboard(enPassantSquare != 64) << enPassantSquare;
	moves &= captures;

	// If player is in check, pawns may only capture checking pieces
	bitboard checkRays = attackData.allChecks;
	if (enPassantSquare != 64) {
		if (bb.count(checkRays) == 1) {
			if (Piece::getType(getPiece(getSquare(checkRays))) == Piece::PAWN) {
				// Or on the enpassant square if it's the pawn giving check
				checkRays |= bitboard(1) << enPassantSquare;
			}
		}
	}
	moves &= checkRays;

	// Loop over all pawn captures to the left
	Bitloop (moves) {
		targetIndex = getSquare(moves);
		unsigned short originIndex = targetIndex + (white ? -7 : 9);

		// Pinned pawn can only capture the pinning piece
		if (!bb.containsSquare(attackData.pins[originIndex], targetIndex)) continue;


		short promotionFlag = (white && (1ULL << targetIndex & ~bb.notEightRank)) ||
			(!white && (1ULL << targetIndex & ~bb.notFirstRank));
		short epFlag = 0;
		if (targetIndex == enPassantSquare) {
			epFlag |= 0b1000;
		}
		short capture = (epFlag ? Piece::PAWN | Piece::getOppositeColor(currentPlayer) : getPiece(targetIndex));
		Move move(Piece::PAWN | currentPlayer, capture, originIndex, targetIndex, promotionFlag | epFlag);

		if (epFlag && inCheckAfter(&move)) continue;

		possibleMoves.push_back(move);

		if (promotionFlag) {
			// Add all other possible promotions
			for (int i = 2; i < 5; i++) {
				Move promoMove(Piece::PAWN | currentPlayer, capture, originIndex, targetIndex, i | epFlag);
				possibleMoves.push_back(promoMove);
			}
		}
	}

	//---------- Captures right -----------------------
	moves = bb.getPawnAttacks(pawns, false, currentPlayer);
	// Capture field has to be occupied by enemy
	captures = bb.getBitboard(Piece::getOppositeColor(currentPlayer));
	// Or marked as enpassant
	captures |= bitboard(enPassantSquare != 64) << enPassantSquare;
	moves &= captures;

	// If player is in check, pawns may only capture checking pieces
	checkRays = attackData.allChecks;
	if (enPassantSquare != 64) {
		if (bb.count(checkRays) == 1) {
			if (Piece::getType(getPiece(getSquare(checkRays))) == Piece::PAWN) {
				// Or on the enpassant square if it's the pawn giving check
				checkRays |= bitboard(1) << enPassantSquare;
			}
		}
	}
	moves &= checkRays;

	// Loop over all pawn captures to the right
	Bitloop (moves) {
		targetIndex = getSquare(moves);
		unsigned short originIndex = targetIndex + (white ? -9 : 7);

		// Pinned pawn can only capture the pinning piece
		if (!bb.containsSquare(attackData.pins[originIndex], targetIndex)) continue;

		short promotionFlag = (white && (1ULL << targetIndex & ~bb.notEightRank)) ||
			(!white && (1ULL << targetIndex & ~bb.notFirstRank));
		short epFlag = 0;
		if (targetIndex == enPassantSquare) {
			epFlag |= 0b1000;
		}
		short capture = (epFlag ? Piece::PAWN | Piece::getOppositeColor(currentPlayer) : getPiece(targetIndex));
		Move move(Piece::PAWN | currentPlayer, capture, originIndex, targetIndex, promotionFlag | epFlag);

		if (epFlag && inCheckAfter(&move)) continue;

		possibleMoves.push_back(move);

		if (promotionFlag) {
			// Add all other possible promotions
			for (int i = 2; i < 5; i++) {
				Move promoMove(Piece::PAWN | currentPlayer, capture, originIndex, targetIndex, i | epFlag);
				possibleMoves.push_back(promoMove);
			}
		}
	}
}

void Board::generateKingMoves(bool onlyCaptures) {
	PROFILE_FUNCTION();
	unsigned short kingPos = (currentPlayer == Piece::WHITE) ? whiteKingPos : blackKingPos;
	bitboard kingMoves = bb.getKingAttacks(kingPos, true);
	// Don't move to squares occupied by your own color
	kingMoves &= ~bb.getBitboard(currentPlayer);
	// Don't move onto attacked squares
	kingMoves &= ~attackData.allAttacks;

	if (onlyCaptures)
		kingMoves &= bb.getBitboard(Piece::getOppositeColor(currentPlayer));

	// Index of the current move
	unsigned short targetIndex = 0;
	Bitloop (kingMoves) {
		// Increase index
		targetIndex = getSquare(kingMoves);
		// If not in check, look if this is a valid castle move
		if (abs(targetIndex - kingPos) == 2) {
			if (attackData.checkExists) continue;

			bool castleFailed = false;
			if (targetIndex > kingPos) {
				// Short castle
				if (currentPlayer == Piece::WHITE && (castleRights & 0b1000)) {
					// Check white's short castle
					// Two squares next to king have to be empty and not attacked
					castleFailed |= (bb.OO & (bb.getOccupied() | attackData.allAttacks));
				}
				else if (castleRights & 0b0010) {
					// Check black's short castle
					// Two squares next to king have to be empty and not attacked
					castleFailed |= (bb.oo & (bb.getOccupied() | attackData.allAttacks));
				}
				else {
					castleFailed = true;
				}

				if (!castleFailed) {
					// Rook has to be on the right square
					castleFailed |= !(getPiece(targetIndex + 1) == (Piece::ROOK | currentPlayer));
				}
			}
			else if (targetIndex < kingPos) {
				// Long castle
				if (currentPlayer == Piece::WHITE && (castleRights & 0b0100)) {
					// Check white's long castle
					// Three squares next to king have to be empty
					castleFailed |= (bb.OOO & bb.getOccupied());
					// Two squares next to king must not be attacked
					castleFailed |= (0x000000000000000C & attackData.allAttacks);
				}
				else if (castleRights & 0b0001) {
					// Check black's long castle
					// Three squares next to king have to be empty
					castleFailed |= (bb.ooo & bb.getOccupied());
					// Two squares next to king must not be attacked
					castleFailed |= (0x0C00000000000000 & attackData.allAttacks);
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
		else {
			Move move(Piece::KING | currentPlayer, getPiece(targetIndex), kingPos, targetIndex);
			possibleMoves.push_back(move);
		}
	}
}

void Board::generateKnightMoves(bool onlyCaptures) {
	PROFILE_FUNCTION();
	bitboard knights = bb.getBitboard(Piece::KNIGHT | currentPlayer);
	// Pinned knights can't move
	knights &= ~attackData.allPins;

	unsigned short knightPos = 0;
	Bitloop (knights) {
		// Get the index of next knight
		knightPos = getSquare(knights);

		bitboard knightMoves = bb.getKnightAttacks(knightPos);
		// Possible Knight moves can't go on squares occupied by own color
		knightMoves &= ~(bb.getBitboard(currentPlayer));

		// If in check, only try moves that move onto the checking ray
		knightMoves &= attackData.allChecks;

		if (onlyCaptures)
			knightMoves &= bb.getBitboard(Piece::getOppositeColor(currentPlayer));

		// Index of the current move
		unsigned short targetIndex = 0;
		Bitloop (knightMoves) {
			// Increase index
			targetIndex = getSquare(knightMoves);

			Move move(Piece::KNIGHT | currentPlayer, getPiece(targetIndex), knightPos, targetIndex);
			possibleMoves.push_back(move);
		}
	}
}

void Board::generateRookMoves(bool onlyCaptures) {
	PROFILE_FUNCTION();
	bitboard rooks = bb.getBitboard(Piece::ROOK | currentPlayer);
	// Pinned rooks can't move when in check
	rooks &= rooks ^ (attackData.checkExists * attackData.allPins);
	unsigned short rookPos = 0;
	Bitloop (rooks) {
		rookPos = getSquare(rooks);

		//if (debugLogs) std::cout << "\nGenerating Moves for Rook on " << getSquareName(rookPos) << "...\n";

		bitboard rookAttacks = bb.getRookAttacks(rookPos, bb.getOccupied());
		// Remove squares that are blocked by friendly pieces
		rookAttacks &= ~bb.getBitboard(currentPlayer);

		// If pinned, move along your pin ray
		rookAttacks &= attackData.pins[rookPos];

		// If in check, only move to blocking squares
		rookAttacks &= attackData.allChecks;

		if (onlyCaptures)
			rookAttacks &= bb.getBitboard(Piece::getOppositeColor(currentPlayer));

		//if (debugLogs) std::cout << "\nRook Attacks Bitboard:\n" << bb.toString(rookAttacks);

		unsigned short targetIndex = 0;
		Bitloop (rookAttacks) {
			targetIndex = getSquare(rookAttacks);

			Move move(Piece::ROOK | currentPlayer, getPiece(targetIndex), rookPos, targetIndex);
			possibleMoves.push_back(move);
		}
	}
}

void Board::generateBishopMoves(bool onlyCaptures) {
	PROFILE_FUNCTION();
	bitboard bishops = bb.getBitboard(Piece::BISHOP | currentPlayer);
	// Pinned bishops can't move when in check
	bishops &= bishops ^ (attackData.checkExists * attackData.allPins);

	unsigned short bishopPos = 0;
	Bitloop (bishops) {
		bishopPos = getSquare(bishops);

		//if (debugLogs) std::cout << "\nGenerating Moves for Bishop on " << getSquareName(bishopPos) << "...\n";

		bitboard bishopAttacks = bb.getBishopAttacks(bishopPos, bb.getOccupied());
		// Remove squares that are blocked by friendly pieces
		bishopAttacks &= ~bb.getBitboard(currentPlayer);

		// If pinned, move along your pin ray
		bishopAttacks &= attackData.pins[bishopPos];

		// If in check, only move to blocking squares
		bishopAttacks &= attackData.allChecks;

		if (onlyCaptures)
			bishopAttacks &= bb.getBitboard(Piece::getOppositeColor(currentPlayer));

		//if (debugLogs) std::cout << "\Bishop Attacks Bitboard:\n" << bb.toString(bishopAttacks);

		unsigned short targetIndex = 0;
		Bitloop (bishopAttacks) {
			targetIndex = getSquare(bishopAttacks);

			Move move(Piece::BISHOP | currentPlayer, getPiece(targetIndex), bishopPos, targetIndex);
			possibleMoves.push_back(move);
		}
	}
}

void Board::generateQueenMoves(bool onlyCaptures) {
	PROFILE_FUNCTION();
	bitboard queens = bb.getBitboard(Piece::QUEEN | currentPlayer);
	// Pinned queens can't move when in check
	queens &= queens ^ (attackData.checkExists * attackData.allPins);

	unsigned short queenPos = 0;
	Bitloop (queens) {
		queenPos = getSquare(queens);

		//if (debugLogs) std::cout << "\nGenerating Moves for Queen on " << getSquareName(queenPos) << "...\n";

		bitboard queenAttacks = bb.getRookAttacks(queenPos, bb.getOccupied()) | bb.getBishopAttacks(queenPos, bb.getOccupied());
		// Remove squares that are blocked by friendly pieces
		queenAttacks &= ~bb.getBitboard(currentPlayer);

		// If pinned, only move along your pin ray
		queenAttacks &= attackData.pins[queenPos];

		// If in check, only move to blocking squares
		queenAttacks &= attackData.allChecks;

		if (onlyCaptures)
			queenAttacks &= bb.getBitboard(Piece::getOppositeColor(currentPlayer));

		//if (debugLogs) std::cout << "\Queen Attacks Bitboard:\n" << bb.toString(queenAttacks);

		unsigned short targetIndex = 0;
		Bitloop (queenAttacks) {
			targetIndex = getSquare(queenAttacks);

			Move move(Piece::QUEEN | currentPlayer, getPiece(targetIndex), queenPos, targetIndex);
			possibleMoves.push_back(move);
		}
	}
}

void Board::orderMoves() {
	std::sort(possibleMoves.begin(), possibleMoves.end(),
		[](const Move &m1, const Move &m2) { return m1.score > m2.score; });
	/*std::cout << "New move order:\n";
	for (int i = 0; i < possibleMoves.size(); i++) {
		std::cout << Move::toString(possibleMoves[i]) 
		<< "(scoreGuess: " << possibleMoves[i].score << "),\n";
	}*/
}

int Board::staticEvaluation() {
	int perspective = (currentPlayer == Piece::WHITE) ? 1 : -1;

	int sum = evaluateMaterial();

	return sum * perspective;
}

int Board::evaluateMaterial() {
	int result = 0;
	result += evaluatePawns<Piece::WHITE>() - evaluatePawns<Piece::BLACK>();
	result += evaluateKing<Piece::WHITE>() - evaluateKing<Piece::BLACK>();
	result += evaluateBishops<Piece::WHITE>() - evaluateBishops<Piece::BLACK>();
	result += evaluateKnights<Piece::WHITE>() - evaluateKnights<Piece::BLACK>();
	result += evaluateRooks<Piece::WHITE>() - evaluateRooks<Piece::BLACK>();
	result += evaluateQueens<Piece::WHITE>() - evaluateQueens<Piece::BLACK>();
	return result;
}


float sigmoid(int x, int offset = 0, float stretch = 1.0f) {
	return 1.0f / (1.0f + expf(-stretch * (x - offset)));
}

template<short color>
int Board::countMaterial() {
	int sum = 0;
	sum += bb.count(bb.getBitboard(Piece::PAWN | color)) * Piece::getPieceValue(Piece::PAWN);
	sum += bb.count(bb.getBitboard(Piece::KNIGHT | color)) * Piece::getPieceValue(Piece::KNIGHT);
	sum += bb.count(bb.getBitboard(Piece::BISHOP | color)) * Piece::getPieceValue(Piece::BISHOP);
	sum += bb.count(bb.getBitboard(Piece::ROOK | color)) * Piece::getPieceValue(Piece::ROOK);
	sum += bb.count(bb.getBitboard(Piece::QUEEN | color)) * Piece::getPieceValue(Piece::QUEEN);
	return sum;
}

template<short color>
int Board::evaluatePawns() {
	bitboard pawns = bb.getBitboard(Piece::PAWN | color);
	int pawnsValue = 0;
	short offset = 0;
	// White and black can share the same map
	// Black's is just flipped upside down
	if constexpr (color == Piece::BLACK) {
		offset = 63;
	}
	Bitloop(pawns) {
		unsigned short position = getSquare(pawns);
		//std::cout << "Value of " << Piece::name(Piece::PAWN | color) << " on " << getSquareName(position);
		// 63 - pos if black, else pos
		position = abs(offset - position);
		int value = pawnValueMap[position];
		//std::cout << ": " << value << '\n';
		pawnsValue += value;
	}
	return pawnsValue;
}

template<short color>
int Board::evaluateKing() {
	unsigned short kingPos = whiteKingPos;
	if constexpr (color == Piece::BLACK) {
		kingPos = blackKingPos;
		// Flip vertically
		kingPos = 63 - kingPos;
	}
		
	int materialCount = countMaterial<Piece::WHITE>() + countMaterial<Piece::BLACK>();

	float earlyGameFactor = sigmoid(materialCount, 4000, 0.003f);
	float endGameFactor = 1.0f - earlyGameFactor;

	//if (debugLogs) std::cout << "Game Progress: " << endGameFactor << '\n';
	int value = int(earlyGameFactor * kingValueMapEarlyMid[kingPos] + endGameFactor * kingValueMapEnd[kingPos]);

	return value;
}

template<short color>
int Board::evaluateKnights() {
	bitboard knights = bb.getBitboard(Piece::KNIGHT | color);
	int knightsValue = 0;
	Bitloop(knights) {
		unsigned short position = getSquare(knights);
		//std::cout << "Value of " << Piece::name(Piece::PAWN | color) << " on " << getSquareName(position);
		int value = knightValueMap[position];
		//std::cout << ": " << value << '\n';
		knightsValue += value;
	}
	return knightsValue;
}

template<short color>
int Board::evaluateBishops() {
	bitboard bishops = bb.getBitboard(Piece::BISHOP | color);
	int bishopsValue = 0;
	Bitloop(bishops) {
		unsigned short position = getSquare(bishops);
		//std::cout << "Value of " << Piece::name(Piece::PAWN | color) << " on " << getSquareName(position);
		int value = bishopValueMap[position];
		//std::cout << ": " << value << '\n';
		bishopsValue += value;
	}
	return bishopsValue;
}

template<short color>
int Board::evaluateRooks() {
	bitboard rooks = bb.getBitboard(Piece::ROOK | color);
	int rooksValue = 0;
	Bitloop(rooks) {
		unsigned short position = getSquare(rooks);
		//std::cout << "Value of " << Piece::name(Piece::PAWN | color) << " on " << getSquareName(position);
		int value = rookValueMap[position];
		//std::cout << ": " << value << '\n';
		rooksValue += value;
	}
	return rooksValue;
}

template<short color>
int Board::evaluateQueens() {
	bitboard queens = bb.getBitboard(Piece::QUEEN | color);
	int queensValue = 0;
	Bitloop(queens) {
		unsigned short position = getSquare(queens);
		//std::cout << "Value of " << Piece::name(Piece::PAWN | color) << " on " << getSquareName(position);
		int value = queenValueMap[position];
		//std::cout << ": " << value << '\n';
		queensValue += value;
	}
	return queensValue;
}

int Board::negaMax(unsigned int depth, int alpha, int beta, SearchResults* results, bool firstCall = false, bool allowNull = true) {
	if (timeOut) return 0;
	//----------------------- TRANSPOSITION TABLE LOOKUP ---------------------------
	TableEntry* transposition = TranspositionTable::get(currentZobristKey);
	if (transposition) {
		//DEBUG_COUT("Transposition Hit ... ");
		if (transposition->depth >= depth) {
			switch (transposition->type) {
			case TableEntry::scoreType::EXACT:
				//DEBUG_COUT(" CAUSES CUTOFF!!\n");
				if (firstCall) {
					results->bestMove = transposition->bestMove;
					results->evaluation = transposition->evaluation;
				}
				return transposition->evaluation;
			case TableEntry::scoreType::LOWER_BOUND:
				//DEBUG_COUT(" sets new lower bound.\n");
				if (!firstCall && (transposition->evaluation >= beta))
					// Beta cutoff with lower bound value
					return beta;
				break;
			case TableEntry::scoreType::UPPER_BOUND:
				//DEBUG_COUT(" sets new upper bound.\n");
				// Great alpha value for current search
				if (!firstCall)
					alpha = transposition->evaluation;
				else if (transposition->bestMove != Move::NULLMOVE) {
					results->bestMove = transposition->bestMove;
					results->evaluation = transposition->evaluation;
					alpha = transposition->evaluation;
				}
				break;
			}
		}
	}
	// ----------------------------------------------------------------------------

	// Remis by repetition
	if (checkForRepetition()) {
		TranspositionTable::add(currentZobristKey, Move::NULLMOVE, 0, TableEntry::scoreType::EXACT, depth);
		return 0;
	}

	generateMoves();

	// Check- or stalemate
	if (possibleMoves.empty()) {
		int score = (attackData.checkExists ? -100000 - depth : 0);
		TranspositionTable::add(currentZobristKey, Move::NULLMOVE, score, TableEntry::scoreType::EXACT, depth);
		return score;
	}

	// Remis by 50 Move rule (Mate has precedence)
	if (halfMoveCount >= 100) {
		TranspositionTable::add(currentZobristKey, Move::NULLMOVE, 0, TableEntry::scoreType::EXACT, depth);
		return 0;
	}

	// If desired depth is reached, return result of a reduced quiet search (allow search depth to double at most)
	if (depth == 0) {
		int eval = negaMaxQuiescence(alpha, beta, results, results->depth);
		TranspositionTable::add(currentZobristKey, Move::NULLMOVE, eval, TableEntry::scoreType::UPPER_BOUND, 0);
		return eval;
	}
	
	// Order Moves before iterating to maximize pruning
	orderMoves();
	std::vector<Move> moves = possibleMoves;
	Move bestMove = Move::NULLMOVE;

	for (int i = 0; i < possibleMoves.size(); i++) {
		results->positionsSearched++;

		//----------------------- NULL MOVE PRUNING ----------------------------------------
		if (allowNull && !firstCall && !attackData.checkExists) {
			const int nullMoveReduction = 3;
			// Avoid situations where zugzwang is most likely
			if (possibleMoves.size() > 5 && depth > nullMoveReduction) {
				// Skip our move
				swapCurrentPlayer();
				// Do a reduced depth search
				int evaluation = -negaMax(depth - nullMoveReduction, -beta, -beta+1, results, false, false);
				// Undo stuff
				swapCurrentPlayer();
				possibleMoves = moves;

				// PRUNE
				if (evaluation >= beta) {
					TranspositionTable::add(currentZobristKey, Move::NULLMOVE, evaluation, TableEntry::scoreType::UPPER_BOUND, depth - nullMoveReduction);
					return beta;
				}
			}
		}
		//---------------------------------------------------------------------------------

		Move move = possibleMoves[i];
		doMove(&move);

		//----------------------- LATE MOVE REDUCTION ----------------------------------------
		const int reduction = (i < 10) ? 1 : 2;
		bool tryReduction = (i >= 5) && (depth > reduction);
		// Don't apply late move reduction when: in check; capturing; promoting; giving check;
		tryReduction &= (!attackData.checkExists && (move.capturedPiece == Piece::NONE) && !move.isPromotion() && !bb.getAttackData(currentPlayer).checkExists);
		if (tryReduction) {
			DEBUG_COUT("DEPTH: " + std::to_string(depth) + ", MOVE #" + std::to_string(i)
				+ ": " + Move::toString(move) + ", alpha: " + std::to_string(alpha) + ". Doing reduced depth search... ");
			int evaluation = -negaMax(depth - reduction, -beta, -alpha, results);
			// Evaluation was not better than best line yet, as expected. PRUNE!
			if (evaluation <= alpha) {
						DEBUG_COUT("--> Line can be discarded.\n");
				undoMove(&move);
				possibleMoves = moves;
				continue;
			} else 
				DEBUG_COUT("--> Evaluation was better than expected. Doing deeper search.\n");
		}
		//---------------------------------------------------------------------------------
		
		int evaluation = -negaMax(depth - 1, -beta, -alpha, results);

		if (firstCall) DEBUG_COUT("Move #" + std::to_string(i) + ' ' + Move::toString(move) + " has evaluation: " + std::to_string(evaluation) + '\n');
		undoMove(&move);
		possibleMoves = moves;

		if (evaluation > alpha) {
			if (firstCall) {
				results->bestMove = move;
				DEBUG_COUT("New best move: #" + std::to_string(i) + ' ' + Move::toString(results->bestMove) +
					" with eval=" + std::to_string(evaluation) + " (Alpha was " + std::to_string(alpha) + ")\n");
				results->evaluation = evaluation;
			}
			bestMove = move;
			alpha = evaluation;
		}
		if (!firstCall && (evaluation >= beta)) {
			// Prune branch
			TranspositionTable::add(currentZobristKey, move, evaluation, TableEntry::scoreType::LOWER_BOUND, depth);
			return beta;
		}
	}
	if (bestMove != Move::NULLMOVE) {
		TranspositionTable::add(currentZobristKey, bestMove, alpha, TableEntry::scoreType::EXACT, depth);
	}
	return alpha;
}

// Search until a quiet position (no check, no captures) is reached
// TODO: Consider stalemate
int Board::negaMaxQuiescence(int alpha, int beta, SearchResults* results, int depth) {
	//std::cout << "negaMax(" << depth << ',' << alpha << ',' << beta << ")\n";
	if (timeOut) return 0;
	int evaluation = staticEvaluation();

	if (depth == 0) return evaluation;

	if (!attackData.checkExists) {
		if (evaluation >= beta)
			return beta;
		alpha = std::max(alpha, evaluation);
	}
	// Check is not quiet
	else {
		generateMoves();
		if (possibleMoves.empty()) {
			//std::cout << "Moves list is empty... ";
			// Checkmate
			//std::cout << "Checkmate!\n";
			return -1000000;
		}
	}

	if (checkForRepetition()) {
		// Remis by repetition
		return 0;
	}
	if (halfMoveCount >= 100) {
		// Remis by 50 Move Rule
		return 0;
	}

	// Order Moves before iterating to maximize pruning
	orderMoves();
	std::vector<Move> captures = possibleMoves;

	for (int i = 0; i < possibleMoves.size(); i++) {
		results->positionsSearched++;
		Move move = possibleMoves[i];
		doMove(&move);
		generateMoves(true);
		evaluation = -negaMaxQuiescence(-beta, -alpha, results, depth-1);
		undoMove(&move);
		possibleMoves = captures;
		
		alpha = std::max(alpha, evaluation);
		if (evaluation >= beta) {
			// Prune branch
			return beta;
		}
	}
	return alpha;
}

Board::SearchResults Board::searchBestMove(unsigned int depth) {
	SearchResults searchResults;
	searchResults.depth = depth;
	negaMax(depth, -1000000, 1000000, &searchResults, true);
	return searchResults;
}

Board::SearchResults Board::iterativeSearch(float time) {
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	start = std::chrono::high_resolution_clock::now();
	end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> duration;

	processing = true;

	unsigned int depth = 0;
	SearchResults lastSearchResult;

	while (!stopDemanded && duration.count() * 1000.0f < time) {
		depth++;
		timeOut = false;
		std::future<SearchResults> future = std::async(&Board::searchBestMove, this, depth);

		while (!(future.wait_for(100ms) == std::future_status::ready)) {
			end = std::chrono::high_resolution_clock::now();
			duration = end - start;
			if (stopDemanded || duration.count() * 1000.0f >= time) {
				timeOut = true;
				goto logResults;
			}
		}
		// Future is ready
		lastSearchResult = future.get();
		currentSearch = lastSearchResult;

		logResults:

		DEBUG_COUT("Depth: " + std::to_string(lastSearchResult.depth) + "; Eval: " + std::to_string(lastSearchResult.evaluation)
				+ "; Move: " + Move::toString(lastSearchResult.bestMove) + "; Positions: "
				+ std::to_string(lastSearchResult.positionsSearched) + "; Time searched: "
				+ std::to_string(duration.count() * 1000.0f) + "ms\n");
	}

	processing = false;
	stopDemanded = false;
	timeOut = false;
	// Set the final search results
	currentSearch = lastSearchResult;
	return lastSearchResult;
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

	Zobrist::swapPlayerHash(currentZobristKey);
	//DEBUG_COUT("Incrementally Updated Zobrist Key: " + std::to_string(currentZobristKey) + '\n');
}

std::string Board::getSquareName(unsigned short index) {
	if (index > 63) return "";
	return squareNames[index];
}

void Board::printPositionHistory() {
	std::cout << "\nPosition History:\n";
	
	for (auto i = positionHistory.begin(); i != positionHistory.end(); i++) {
		std::cout << *i << '\n';
	}
	std::cout << '\n';
}

unsigned long long Board::testMoveGeneration(unsigned int depth, bool divide) {
	PROFILE_FUNCTION();
	if (depth == 1) return possibleMoves.size();
	unsigned long positionCount = 0;
	std::vector<Move> moves = possibleMoves;

	for (int i = 0; i < possibleMoves.size(); i++) {
		// COPY!!! IMPORTANT, because possibleMoves will change
		Move move = possibleMoves[i];
		doMove(&move);
		//float time;
		{
			//Timer timer("Board::generateMoves()", &time);
			generateMoves();
		}
		//accumulatedGenerationTime += time;
		unsigned long long positionsAfterMove = testMoveGeneration(depth - 1, false);
		positionCount += positionsAfterMove;
		undoMove(&move);
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

std::thread Board::launchSearchThread(float time) {
	processing = true;
	return std::thread(&Board::iterativeSearch, this, time);
}
