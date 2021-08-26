#include "Bitboard.h"

Bitboard::Bitboard() : knightAttacks(), kingAttacks()
{
    initKnightAttacks();
    initKingAttacks();
}

void Bitboard::initKnightAttacks()
{
    for (short i = 0; i < 64; i++) {
        // Bitboard representation of one of the 64 positions
        bitboard pos = (bitboard)1 << i;

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

bitboard Bitboard::getBitboard(short p)
{
    return allPieces[p];
}

void Bitboard::setPiece(short p, short column, short row)
{
    bitboard mask = (bitboard)1 << (row * 8 + column);
    allPieces[p] |= mask;
    allPieces[Piece::getColor(p)] |= mask;
}

void Bitboard::removePiece(short p, short column, short row)
{
    bitboard mask = (bitboard)1 << (row * 8 + column);
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

bitboard Bitboard::getKnightAttacks(short column, short row)
{
    return knightAttacks[row * 8 + column];
}

bitboard Bitboard::getKingAttacks(short column, short row)
{
    return kingAttacks[row * 8 + column];
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