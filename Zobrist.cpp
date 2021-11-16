#include "Zobrist.h"

unsigned long long Zobrist::pieceHashes[23][64];
unsigned long long Zobrist::castleHashes[16];
unsigned long long Zobrist::epHashes[65];
unsigned long long Zobrist::whiteToMoveHash;

void Zobrist::initializeHashes() {
    std::random_device rd;
    std::mt19937_64 rng(rd());

    for (int i = 0; i < 23; i++) {
        for (int j = 0; j < 64; j++) {
            pieceHashes[i][j] = rng();
        }
    }
    for (int i = 0; i < 16; i++) {
        castleHashes[i] = rng();
    }
    for (int i = 0; i < 65; i++) {
        epHashes[i] = rng();
    }
    whiteToMoveHash = rng();
}

unsigned long long Zobrist::getZobristKey(const Bitboard* bb, short castleRights, unsigned short epSquare, bool whiteToMove) {
    unsigned long long castleHash = castleHashes[castleRights];
    unsigned long long epHash = epHashes[epSquare];
    unsigned long long playerHash = whiteToMoveHash * whiteToMove;

    unsigned long long hash = castleHash ^ epHash ^ playerHash;
    short color = Piece::WHITE;
    for (short pieceType = 1; pieceType < 7; pieceType++) {
        bitboard pieces = bb->getBitboard(color | pieceType);
        unsigned short pos = 0;
        Bitloop(pieces) {
            pos = getSquare(pieces);
            hash ^= pieceHashes[color | pieceType][pos];
        }
    }
    color = Piece::BLACK;
    for (short pieceType = 1; pieceType < 7; pieceType++) {
        bitboard pieces = bb->getBitboard(color | pieceType);
        unsigned short pos = 0;
        Bitloop(pieces) {
            pos = getSquare(pieces);
            hash ^= pieceHashes[color | pieceType][pos];
        }
    }
    return hash;
}

void Zobrist::updatePieceHash(unsigned long long &oldHash, short piece, unsigned short pos) {
    oldHash ^= pieceHashes[piece][pos];
}

void Zobrist::swapPlayerHash(unsigned long long &oldHash) {
    // Add / Remove the white to move part from the hash
    oldHash ^= whiteToMoveHash;
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
