#include "Bitboard.h"

Bitboard::Bitboard() : knightAttacks(), kingAttacks()
{
    for (int i = 0; i < 64; i++) {
        bishopAttacks[i] = new bitboard[512];
        rookAttacks[i] = new bitboard[4096];
    }

    initKnightAttacks();
    initKingAttacks();
    initBishopMasks();
    initRookMasks();

    // Initialize random ULL generator
    std::random_device rd;
    randomBitboardGenerator = std::mt19937_64(rd());

    initMagicNumbers();
}

Bitboard::~Bitboard() {
    for (int i = 0; i < 64; i++) {
        delete[] bishopAttacks[i];
        delete[] rookAttacks[i];
    }
}

void Bitboard::initKnightAttacks()
{
    for (short i = 0; i < 64; i++) {
        // Bitboard representation of one of the 64 positions
        bitboard pos = bitboard(1) << i;

        bitboard* currentMask = knightAttacks + i;
        // NorthNorthEast
        *currentMask |= (pos << 17) & notAfile;
        // NorthEastEast
        *currentMask |= (pos << 10) & notAfile & notBfile;
        // SouthEastEast
        *currentMask |= (pos >> 6) & notAfile & notBfile;
        // SouthSouthEast
        *currentMask |= (pos >> 15) & notAfile;
        // NorthNorthWest
        *currentMask |= (pos << 15) & notHfile;
        // NorthWestWest
        *currentMask |= (pos << 6) & notHfile & notGfile;
        // SouthWestWest
        *currentMask |= (pos >> 10) & notHfile & notGfile;
        // SouthSouthWest
        *currentMask |= (pos >> 17) & notHfile;
    }
}

void Bitboard::initKingAttacks() {
    for (short i = 0; i < 64; i++) {
        // Bitboard representation of one of the 64 positions
        bitboard pos = (bitboard)1 << i;

        bitboard* currentMask = kingAttacks + i;
        // North
        *currentMask |= (pos << 8) & notFirstRank;
        // NorthEast
        *currentMask |= (pos << 9) & notFirstRank & notAfile;
        // East
        *currentMask |= (pos << 1) & notAfile;
        // SouthEast
        *currentMask |= (pos >> 7) & notAfile & notEightRank;
        // South
        *currentMask |= (pos >> 8) & notEightRank;
        // SouthWest
        *currentMask |= (pos >> 9) & notEightRank & notHfile;
        // West
        *currentMask |= (pos >> 1) & notHfile;
        // NorthWest
        *currentMask |= (pos << 7) & notFirstRank & notHfile;

        //std::cout << '\n' << toString(*currentMask);
    }
}

void Bitboard::initBishopMasks() {
    for (short i = 0; i < 64; i++) {
        bitboard* currentMask = bishopMasks + i;
        unsigned short bitCount = 0;

        int startColumn = i % 8;
        int startRow = i / 8;
        unsigned short bit;

        for (int column = startColumn + 1, row = startRow + 1; column < 7 && row < 7; column++, row++) {
            bit = (row * 8 + column);
            set(currentMask, bit);
            bitCount++;
        }
        for (int column = startColumn + 1, row = startRow - 1; column < 7 && row > 0; column++, row--) {
            bit = (row * 8 + column);
            set(currentMask, bit);
            bitCount++;
        }
        for (int column = startColumn - 1, row = startRow + 1; column > 0 && row < 7; column--, row++) {
            bit = (row * 8 + column);
            set(currentMask, bit);
            bitCount++;
        }
        for (int column = startColumn - 1, row = startRow - 1; column > 0 && row > 0; column--, row--) {
            bit = (row * 8 + column);
            set(currentMask, bit);
            bitCount++;
        }

        bitsInBishopMask[i] = bitCount;
        //std::cout << '\n' << toString(*currentMask) << "Bits in mask: " << bitCount;
    }
}

void Bitboard::initRookMasks() {
    for (short i = 0; i < 64; i++) {
        bitboard* currentMask = rookMasks + i;
        unsigned short bitCount = 0;

        int startColumn = i % 8;
        int startRow = i / 8;
        unsigned short bit;

        for (int column = startColumn + 1; column < 7; column++) {
            bit = (startRow * 8 + column);
            set(currentMask, bit);
            bitCount++;
        }
        for (int column = startColumn - 1; column > 0; column--) {
            bit = (startRow * 8 + column);
            set(currentMask, bit);
            bitCount++;
        }
        for (int row = startRow + 1; row < 7; row++) {
            bit = (row * 8 + startColumn);
            set(currentMask, bit);
            bitCount++;
        }
        for (int row = startRow - 1; row > 0; row--) {
            bit = (row * 8 + startColumn);
            set(currentMask, bit);
            bitCount++;
        }

        bitsInRookMask[i] = bitCount;
        //std::cout << '\n' << toString(*currentMask) << "\nBitcount: " << bitCount << '\n';
    }
}

bitboard Bitboard::getOccupancy(int index, bitboard blockerMask)
{
    bitboard occupany = bitboard(0);

    int bitCount = count(blockerMask);
    short square = -1;
    
    for (int i = 0; i < bitCount; i++) {
        // Read the next least significant bit
        square += pop(&blockerMask) + 1;
        // Set the i'th bit if the i'th bit in the index is also set
        // --> guarantees all possible occupancy variations with indexes from 0 - 2^bitCount
        if (index & (1 << i)) {
            set(&occupany, square);
        }
    }
    return occupany;
}

bitboard Bitboard::scanRookDirections(unsigned short pos, bitboard blockers) {
    bitboard result(0);
    int startColumn = pos % 8;
    int startRow = pos / 8;
    unsigned short bit;

    for (int column = startColumn + 1; column < 8; column++) {
        bit = (startRow * 8 + column);
        set(&result, bit);
        if ((bitboard(1) << bit) & blockers) break;
    }
    for (int column = startColumn - 1; column >= 0; column--) {
        bit = (startRow * 8 + column);
        set(&result, bit);
        if ((bitboard(1) << bit) & blockers) break;
    }
    for (int row = startRow + 1; row < 8; row++) {
        bit = (row * 8 + startColumn);
        set(&result, bit);
        if ((bitboard(1) << bit) & blockers) break;
    }
    for (int row = startRow - 1; row >= 0; row--) {
        bit = (row * 8 + startColumn);
        set(&result, bit);
        if ((bitboard(1) << bit) & blockers) break;
    }
    return result;
}

bitboard Bitboard::scanBishopDirections(unsigned short pos, bitboard blockers) {
    bitboard result(0);
    int startColumn = pos % 8;
    int startRow = pos / 8;
    unsigned short bit;

    for (int column = startColumn + 1, row = startRow + 1; column < 8 && row < 8; column++, row++) {
        bit = (row * 8 + column);
        set(&result, bit);
        if ((bitboard(1) << bit) & blockers) break;
    }
    for (int column = startColumn + 1, row = startRow - 1; column < 8 && row >= 0; column++, row--) {
        bit = (row * 8 + column);
        set(&result, bit);
        if ((bitboard(1) << bit) & blockers) break;
    }
    for (int column = startColumn - 1, row = startRow + 1; column >= 0 && row < 8; column--, row++) {
        bit = (row * 8 + column);
        set(&result, bit);
        if ((bitboard(1) << bit) & blockers) break;
    }
    for (int column = startColumn - 1, row = startRow - 1; column >= 0 && row >= 0; column--, row--) {
        bit = (row * 8 + column);
        set(&result, bit);
        if ((bitboard(1) << bit) & blockers) break;
    }
    return result;
}

unsigned long long Bitboard::getMagicNumberCandidate()
{
    return randomBitboardGenerator() & randomBitboardGenerator() & randomBitboardGenerator();
}

unsigned long long Bitboard::findMagicNumber(unsigned short pos, bool forRook) {
    std::cout << "Looking for magic number #" << pos << " for " << (forRook ? "Rook" : "Bishop") << "...\n";

    // Relevant Blockermask for this position and piece
    bitboard blockerMask = forRook ? rookMasks[pos] : bishopMasks[pos];
    bitboard* attackTableToFill = forRook ? rookAttacks[pos] : bishopAttacks[pos];
    // Calculate relevant bits in the blockerMask of this position and the resulting table size needed
    unsigned short relevantBits = forRook ? bitsInRookMask[pos] : bitsInBishopMask[pos];
    int tableSize = 1 << relevantBits; // 2^relevantBits

    // Init all possible occupancy combinations and the resulting scanlines
    bitboard* occupancyCombinations = new bitboard[tableSize];
    bitboard* scanLines = new bitboard[tableSize];
    for (int i = 0; i < tableSize; i++) {
        occupancyCombinations[i] = getOccupancy(i, forRook ? rookMasks[pos] : bishopMasks[pos]);
        scanLines[i] = forRook ? scanRookDirections(pos, occupancyCombinations[i]) : scanBishopDirections(pos, occupancyCombinations[i]);
    }

    for (int i = 0; i < 100000000; i++) {
        unsigned long long magicCandidate = getMagicNumberCandidate();
        // Magic candidate doesn't have enough 1s
        if (count((blockerMask * magicCandidate) & 0xFF00000000000000) < 6) continue;
        // Reset Memory
        memset(attackTableToFill, 0ULL, sizeof(bitboard) * (forRook ? 4096 : 512));

        int index;
        bool collision;
        // Check this magic number candidate for all indeces
        for (index = 0, collision = false; (index < tableSize) && !collision; index++) {
            //std::cout << "\nOccupancy #" << index << ":\n" << toString(occupancyCombinations[index]);
            int magicIndex = shittyHash(occupancyCombinations[index], magicCandidate, relevantBits);
            // No collision
            if (attackTableToFill[magicIndex] == 0ULL) {
                // Magic index maps to the scanLine of this occupancy
                attackTableToFill[magicIndex] = scanLines[index];
                //std::cout << "\nAttack table filled with:\n" << toString(attackTableToFill[magicIndex]);
            }
            else if (attackTableToFill[magicIndex] != scanLines[index]) {
                // COLLISION!
                collision = true;
            }
        }

        if (!collision) {
            // If there was no collision for all the indeces, magic number works!

            // Free memory
            delete[] scanLines;
            delete[] occupancyCombinations;

            return magicCandidate;
        }
    }

    // Free memory
    delete[] scanLines;
    delete[] occupancyCombinations;

    std::cerr << "No magic number found for " << (forRook ? "Rook" : "Bishop") << " on " << pos << '\n';
    return 0;
}

void Bitboard::initMagicNumbers() {
    for (int pos = 0; pos < 64; pos++) {
        rookMagics[pos] = findMagicNumber(pos, true);
        bishopMagics[pos] = findMagicNumber(pos, false);
    }
}

int Bitboard::shittyHash(bitboard occupancy, unsigned long long magicNumber, unsigned short bitCount) {
    return int((occupancy * magicNumber) >> (64 - bitCount));
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

bitboard Bitboard::getRookAttacks(unsigned short pos)
{
    int magicIndex = shittyHash(getOccupied() & rookMasks[pos], rookMagics[pos], bitsInRookMask[pos]);
    return rookAttacks[pos][magicIndex];
}

bool Bitboard::containsSquare(bitboard b, unsigned short square)
{
    return (b >> square) & 1;
}

unsigned short Bitboard::pop(bitboard* b)
{
    unsigned long scanIndex;
    _BitScanForward64(&scanIndex, *b);
    if (scanIndex != 63) {
        *b >>= scanIndex + 1;
    }
    // Special case where we would >>64 (undef.)
    else {
        *b = 0;
    }
    return scanIndex;
}

unsigned short Bitboard::count(bitboard b)
{
    unsigned short count = 0;
    while (b) {
        b &= b - 1; // removes least significant bit
        count++;
    }
    return count;
}

void Bitboard::set(bitboard* b, unsigned short bit) {
    *b |= bitboard(1) << bit;
}

std::string Bitboard::toString(bitboard b)
{
    std::string s;
    int count = 0;

    for (int i = 7; i >= 0 ; i--) {
        for (int j = 0; j < 8; j++) {
            int index = i * 8 + j;
            s += ((b >> index) & 1) == 1 ? "1 " : ". ";
        }
        s += '\n';
    }
    return s;
}
