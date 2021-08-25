#include "Bitboard.h"

Bitboard::Bitboard() : knightAttacks()
{
    initKnightAttacks();
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

        std::cout << '\n' << toString(*currentAttacks);
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

std::string Bitboard::toString(bitboard b)
{
    std::string s;
    int count = 0;

    for (int i = 0; i < 64; i++) {
        s += std::to_string(b & 1) + ' ';
        b >>= 1;
        if ((i+1) % 8 == 0) {
            s += '\n';
        }
    }
    return s;
}
