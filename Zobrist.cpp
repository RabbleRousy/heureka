#include "Zobrist.h"

unsigned long long Zobrist::castleHashes[16];
unsigned long long Zobrist::epHashes[65];

void Zobrist::initializeHashes() {
    std::random_device rd;
    std::mt19937_64 rng(rd());

    for (int i = 0; i < 16; i++) {
        castleHashes[i] = rng();
    }
    for (int i = 0; i < 65; i++) {
        epHashes[i] = rng();
    }
}

unsigned long long Zobrist::getZobristKey(const Bitboard* bitboard, short castleRights, unsigned short epSquare) {
    unsigned long long castleHash = castleHashes[castleRights];
    unsigned long long epHash = epHashes[epSquare];

    unsigned long long hash = castleHash ^ epHash;
    short color = Piece::WHITE;
    for (short pieceType = 1; pieceType < 7; pieceType++) {
        hash ^= bitboard->getBitboard(color | pieceType);
    }
    color = Piece::BLACK;
    for (short pieceType = 1; pieceType < 7; pieceType++) {
        hash ^= bitboard->getBitboard(color | pieceType);
    }
    return hash;
}

void Zobrist::updateZobristKey(unsigned long long &oldHash, bitboard oldBB, bitboard newBB) {
    // Remove old bitboard from hash
    oldHash ^= oldBB;
    // Add new bitboard to hash
    oldHash ^= newBB;
}

void Zobrist::updateZobristKey(unsigned long long &oldHash, short oldCastle, short newCastle) {
    // Remove old castle from hash
    oldHash ^= castleHashes[oldCastle];
    // Add new castle to hash
    oldHash ^= castleHashes[newCastle];
}

void Zobrist::updateZobristKey(unsigned long long &oldHash, unsigned short oldEP, unsigned short newEP) {
    // Remove old ep square from hash
    oldHash ^= epHashes[oldEP];
    // Add new ep square to hash
    oldHash ^= epHashes[newEP];
}
