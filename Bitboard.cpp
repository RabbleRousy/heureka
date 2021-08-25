#include "Bitboard.h"

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
