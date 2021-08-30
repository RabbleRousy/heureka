#include "Bitboard.h"

Bitboard::Bitboard() : knightAttacks(), kingAttacks()
{
    initKnightAttacks();
    initKingAttacks();
    initBishopAttacks();
    initRookAttacks();
}

void Bitboard::initKnightAttacks()
{
    for (short i = 0; i < 64; i++) {
        // Bitboard representation of one of the 64 positions
        bitboard pos = bitboard(1) << i;

        bitboard* currentAttacks = knightAttacks + i;
        // NorthNorthEast
        *currentAttacks |= (pos << 17) & notAfile;
        // NorthEastEast
        *currentAttacks |= (pos << 10) & notAfile & notBfile;
        // SouthEastEast
        *currentAttacks |= (pos >> 6) & notAfile & notBfile;
        // SouthSouthEast
        *currentAttacks |= (pos >> 15) & notAfile;
        // NorthNorthWest
        *currentAttacks |= (pos << 15) & notHfile;
        // NorthWestWest
        *currentAttacks |= (pos << 6) & notHfile & notGfile;
        // SouthWestWest
        *currentAttacks |= (pos >> 10) & notHfile & notGfile;
        // SouthSouthWest
        *currentAttacks |= (pos >> 17) & notHfile;
    }
}

void Bitboard::initKingAttacks() {
    for (short i = 0; i < 64; i++) {
        // Bitboard representation of one of the 64 positions
        bitboard pos = (bitboard)1 << i;

        bitboard* currentAttacks = kingAttacks + i;
        // North
        *currentAttacks |= (pos << 8) & notFirstRank;
        // NorthEast
        *currentAttacks |= (pos << 9) & notFirstRank & notAfile;
        // East
        *currentAttacks |= (pos << 1) & notAfile;
        // SouthEast
        *currentAttacks |= (pos >> 7) & notAfile & notEightRank;
        // South
        *currentAttacks |= (pos >> 8) & notEightRank;
        // SouthWest
        *currentAttacks |= (pos >> 9) & notEightRank & notHfile;
        // West
        *currentAttacks |= (pos >> 1) & notHfile;
        // NorthWest
        *currentAttacks |= (pos << 7) & notFirstRank & notHfile;

        //std::cout << '\n' << toString(*currentAttacks);
    }
}

void Bitboard::initBishopAttacks() {
    for (short i = 0; i < 64; i++) {
        bitboard* currentAttacks = bishopAttacks + i;

        int startColumn = i % 8;
        int startRow = i / 8;

        for (int column = startColumn + 1, row = startRow + 1; column < 7 && row < 7; column++, row++) {
            *currentAttacks |= bitboard(1) << (row * 8 + column);
        }
        for (int column = startColumn + 1, row = startRow - 1; column < 7 && row > 0; column++, row--) {
            *currentAttacks |= bitboard(1) << (row * 8 + column);
        }
        for (int column = startColumn - 1, row = startRow + 1; column > 0 && row < 7; column--, row++) {
            *currentAttacks |= bitboard(1) << (row * 8 + column);
        }
        for (int column = startColumn - 1, row = startRow - 1; column > 0 && row > 0; column--, row--) {
            *currentAttacks |= bitboard(1) << (row * 8 + column);
        }

        //std::cout << '\n' << toString(*currentAttacks);
    }
}

void Bitboard::initRookAttacks() {
    for (short i = 0; i < 64; i++) {
        bitboard* currentAttacks = rookAttacks + i;

        int startColumn = i % 8;
        int startRow = i / 8;

        for (int column = startColumn + 1; column < 7; column++) {
            *currentAttacks |= bitboard(1) << (startRow * 8 + column);
        }
        for (int column = startColumn - 1; column > 0; column--) {
            *currentAttacks |= bitboard(1) << (startRow * 8 + column);
        }
        for (int row = startRow + 1; row < 7; row++) {
            *currentAttacks |= bitboard(1) << (row * 8 + startColumn);
        }
        for (int row = startRow - 1; row > 0; row--) {
            *currentAttacks |= bitboard(1) << (row * 8 + startColumn);
        }

        std::cout << '\n' << toString(*currentAttacks);
    }
}

bitboard Bitboard::getBitboard(short p)
{
    return allPieces[p];
}

void Bitboard::setPiece(short p, unsigned short index)
{
    bitboard mask = (bitboard)1 << (index);
    allPieces[p] |= mask;
    allPieces[Piece::getColor(p)] |= mask;
}

void Bitboard::removePiece(short p, unsigned short index)
{
    bitboard mask = (bitboard)1 << (index);
    allPieces[p] &= ~mask;
    allPieces[Piece::getColor(p)] &= ~mask;
}

bitboard Bitboard::getOccupied()
{
    return allPieces[Piece::WHITE] | allPieces[Piece::BLACK];
}

bitboard Bitboard::getEmpty()
{
    return ~(allPieces[Piece::WHITE] | allPieces[Piece::BLACK]);
}

bitboard Bitboard::getAllAttacks(short color)
{
    return bitboard();
}

bitboard Bitboard::getKnightAttacks(unsigned short pos)
{
    return knightAttacks[pos];
}

bitboard Bitboard::getKingAttacks(unsigned short pos)
{
    return kingAttacks[pos];
}

bool Bitboard::containsSquare(bitboard b, unsigned short square)
{
    return (b >> square) & 1;
}

unsigned short Bitboard::pop(bitboard* b)
{
    unsigned long scanIndex;
    _BitScanForward64(&scanIndex, *b);
    *b >>= scanIndex + 1;
    return scanIndex;
}

std::string Bitboard::toString(bitboard b)
{
    std::string s;
    int count = 0;

    for (int i = 7; i >= 0 ; i--) {
        for (int j = 0; j < 8; j++) {
            int index = i * 8 + j;
            s += std::to_string((b >> index) & 1) + ' ';
        }
        s += '\n';
    }
    return s;
}
