#pragma once
#include <stdint.h>

#define INFINITY (INT_MAX - 52)
#define UNKNOWN (INT_MAX)

#pragma region Squares

enum
{
	a8,
	b8,
	c8,
	d8,
	e8,
	f8,
	g8,
	h8,
	a7,
	b7,
	c7,
	d7,
	e7,
	f7,
	g7,
	h7,
	a6,
	b6,
	c6,
	d6,
	e6,
	f6,
	g6,
	h6,
	a5,
	b5,
	c5,
	d5,
	e5,
	f5,
	g5,
	h5,
	a4,
	b4,
	c4,
	d4,
	e4,
	f4,
	g4,
	h4,
	a3,
	b3,
	c3,
	d3,
	e3,
	f3,
	g3,
	h3,
	a2,
	b2,
	c2,
	d2,
	e2,
	f2,
	g2,
	h2,
	a1,
	b1,
	c1,
	d1,
	e1,
	f1,
	g1,
	h1,
};


static const wchar_t* squareNames[]
{
	L"a8", L"b8", L"c8", L"d8", L"e8", L"f8", L"g8", L"h8",
	L"a7", L"b7", L"c7", L"d7", L"e7", L"f7", L"g7", L"h7",
	L"a6", L"b6", L"c6", L"d6", L"e6", L"f6", L"g6", L"h6",
	L"a5", L"b5", L"c5", L"d5", L"e5", L"f5", L"g5", L"h5",
	L"a4", L"b4", L"c4", L"d4", L"e4", L"f4", L"g4", L"h4",
	L"a3", L"b3", L"c3", L"d3", L"e3", L"f3", L"g3", L"h3",
	L"a2", L"b2", L"c2", L"d2", L"e2", L"f2", L"g2", L"h2",
	L"a1", L"b1", L"c1", L"d1", L"e1", L"f1", L"g1", L"h1",
};

constexpr uint64_t notAFile = 0xfefefefefefefefeULL;
constexpr uint64_t notHFile = 0x7f7f7f7f7f7f7f7fULL;
constexpr uint64_t not8Rank = 0xffffffffffffff00ULL;
constexpr uint64_t not1Rank = 0x00ffffffffffffffULL;

//for knight move:
constexpr uint64_t notABFile = 0xfcfcfcfcfcfcfcfcULL;
constexpr uint64_t notHGFile = 0x3f3f3f3f3f3f3f3fULL;
constexpr uint64_t not87Rank = 0xffffffffffff0000ULL;
constexpr uint64_t not12Rank = 0x0000ffffffffffffULL;

//for advanced pawn:
constexpr uint64_t rank6 = 0x0000000000ff0000ULL;
constexpr uint64_t rank3 = 0x0000ff0000000000ULL;

//for castle danger(king cannot castle through check)
constexpr uint64_t wkCastleAttacked = 0x6000000000000000ULL;
constexpr uint64_t wqCastleAttacked = 0x0c00000000000000ULL;
constexpr uint64_t bkCastleAttacked = 0x0000000000000060ULL;
constexpr uint64_t bqCastleAttacked = 0x000000000000000cULL;


#pragma endregion
#pragma region Pieces

//ASCII pieces
static constexpr char ASCIIPieces[]{' ', 'p', 'N', 'B', 'R', 'Q', '@'};

//UNICODE pieces:
static const wchar_t* const unicodePieces[]{L" ", L"♙", L"♘", L"♗", L"♖", L"♕", L"♔"};

static constexpr int knightAttackCountBySquare[]
{
	2, 3, 4, 4, 4, 4, 3, 2,
	3, 4, 6, 6, 6, 6, 4, 3,
	4, 6, 8, 8, 8, 8, 6, 4,
	4, 6, 8, 8, 8, 8, 6, 4,
	4, 6, 8, 8, 8, 8, 6, 4,
	4, 6, 8, 8, 8, 8, 6, 4,
	3, 4, 6, 6, 6, 6, 4, 3,
	2, 3, 4, 4, 4, 4, 3, 2,
};

//sliding piece Caching.
static constexpr int bishopAttackCountBySquare[]
{
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6,
};

static constexpr int rookAttackCountBySquare[]
{
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12,
};

static constexpr int queenAttackCountBySquare[]
{
	18, 16, 16, 16, 16, 16, 16, 18,
	16, 15, 15, 15, 15, 15, 15, 16,
	16, 15, 17, 17, 17, 17, 15, 16,
	16, 15, 17, 19, 19, 17, 15, 16,
	16, 15, 17, 19, 19, 17, 15, 16,
	16, 15, 17, 17, 17, 17, 15, 16,
	16, 15, 15, 15, 15, 15, 15, 16,
	18, 16, 16, 16, 16, 16, 16, 18,
};


//For Evaluation:

static constexpr int pieceScores[6]{
	100, // pawn score
	300, // knight scrore
	350, // bishop score
	500, // rook score
	900, // queen score
	10000, // king score
};

static constexpr int pawnPositionalScore[64]
{
	90, 90, 90, 90, 90, 90, 90, 90,
	30, 30, 30, 40, 40, 30, 30, 30,
	20, 20, 20, 30, 30, 30, 20, 20,
	10, 10, 10, 20, 20, 10, 10, 10,
	5, 5, 10, 20, 22, 5, 5, 5,
	0, 0, 0, 5, 5, 0, 0, 0,
	0, 0, 0, -10, -10, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static constexpr int knightPositionalScore[64]
{
	-5, 0, 0, 0, 0, 0, 0, -5,
	-5, 0, 0, 10, 10, 0, 0, -5,
	-5, 5, 20, 20, 20, 20, 5, -5,
	-5, 10, 20, 30, 30, 20, 10, -5,
	-5, 10, 20, 30, 30, 20, 10, -5,
	-5, 5, 20, 10, 10, 20, 5, -5,
	-5, 0, 0, 0, 0, 0, 0, -5,
	-5, -10, 0, 0, 0, 0, -10, -5
};

// bishop positional score
static constexpr int bishopPositionalScore[64]
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 10, 10, 0, 0, 0,
	0, 5, 10, 20, 20, 10, 5, 0,
	0, 10, 10, 20, 20, 10, 10, 0,
	5, 10, 0, -10, -10, 0, 10, 5,
	0, 30, 0, 0, 0, 0, 30, 0,
	0, 0, -10, 0, 0, -10, 0, 0

};

// rook positional score
static constexpr int rookPositionalScore[64] =
{
	50, 50, 50, 50, 50, 50, 50, 50,
	50, 50, 50, 50, 50, 50, 50, 50,
	0, 0, 10, 20, 20, 10, 0, 0,
	0, 0, 10, 20, 20, 10, 0, 0,
	0, 0, 10, 20, 20, 10, 0, 0,
	0, 0, 10, 20, 20, 10, 0, 0,
	0, 0, 10, 20, 20, 10, 0, 0,
	0, 0, 0, 20, 20, 0, 0, 0
};

// king positional score
static constexpr int queenPositionalScore[64] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};


// king positional score
static constexpr int kingPositionalScoreMid[64] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 5, - 25, -50, -50, -25, 5, 0,
	0, 5, 35, -15, -15, -15, 40, 0,
};
// king positional score
static constexpr int kingPositionalScoreLate[64] =
{
	-5, 0, 0, 0, 0, 0, 0, -5,
	0, 0, 5, 5, 5, 5, 0, 0,
	0, 5, 5, 10, 10, 5, 5, 0,
	0, 5, 10, 20, 20, 10, 5, 0,
	0, 5, 10, 20, 20, 10, 5, 0,
	0, 0, 5, 10, 10, 5, 0, 0,
	-5, 0, 5, 5, 5, 5, 0, -5,
	-10, -5, 0, 0, 0, 0, -5, -10
};


static constexpr const int* PositionalScores[7]
{
	pawnPositionalScore, knightPositionalScore, bishopPositionalScore, rookPositionalScore, queenPositionalScore, kingPositionalScoreMid, kingPositionalScoreLate
};

// mirror positional score tables for opposite side
static constexpr int mirror[64] =
{
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

static constexpr uint64_t rookMagicNumbers[64]
{
	0x8a80104000800020ULL,
	0x140002000100040ULL,
	0x2801880a0017001ULL,
	0x100081001000420ULL,
	0x200020010080420ULL,
	0x3001c0002010008ULL,
	0x8480008002000100ULL,
	0x2080088004402900ULL,
	0x800098204000ULL,
	0x2024401000200040ULL,
	0x100802000801000ULL,
	0x120800800801000ULL,
	0x208808088000400ULL,
	0x2802200800400ULL,
	0x2200800100020080ULL,
	0x801000060821100ULL,
	0x80044006422000ULL,
	0x100808020004000ULL,
	0x12108a0010204200ULL,
	0x140848010000802ULL,
	0x481828014002800ULL,
	0x8094004002004100ULL,
	0x4010040010010802ULL,
	0x20008806104ULL,
	0x100400080208000ULL,
	0x2040002120081000ULL,
	0x21200680100081ULL,
	0x20100080080080ULL,
	0x2000a00200410ULL,
	0x20080800400ULL,
	0x80088400100102ULL,
	0x80004600042881ULL,
	0x4040008040800020ULL,
	0x440003000200801ULL,
	0x4200011004500ULL,
	0x188020010100100ULL,
	0x14800401802800ULL,
	0x2080040080800200ULL,
	0x124080204001001ULL,
	0x200046502000484ULL,
	0x480400080088020ULL,
	0x1000422010034000ULL,
	0x30200100110040ULL,
	0x100021010009ULL,
	0x2002080100110004ULL,
	0x202008004008002ULL,
	0x20020004010100ULL,
	0x2048440040820001ULL,
	0x101002200408200ULL,
	0x40802000401080ULL,
	0x4008142004410100ULL,
	0x2060820c0120200ULL,
	0x1001004080100ULL,
	0x20c020080040080ULL,
	0x2935610830022400ULL,
	0x44440041009200ULL,
	0x280001040802101ULL,
	0x2100190040002085ULL,
	0x80c0084100102001ULL,
	0x4024081001000421ULL,
	0x20030a0244872ULL,
	0x12001008414402ULL,
	0x2006104900a0804ULL,
	0x1004081002402ULL
};

static constexpr uint64_t bishopMagicNumbers[64]
{
	0x40040844404084ULL,
	0x2004208a004208ULL,
	0x10190041080202ULL,
	0x108060845042010ULL,
	0x581104180800210ULL,
	0x2112080446200010ULL,
	0x1080820820060210ULL,
	0x3c0808410220200ULL,
	0x4050404440404ULL,
	0x21001420088ULL,
	0x24d0080801082102ULL,
	0x1020a0a020400ULL,
	0x40308200402ULL,
	0x4011002100800ULL,
	0x401484104104005ULL,
	0x801010402020200ULL,
	0x400210c3880100ULL,
	0x404022024108200ULL,
	0x810018200204102ULL,
	0x4002801a02003ULL,
	0x85040820080400ULL,
	0x810102c808880400ULL,
	0xe900410884800ULL,
	0x8002020480840102ULL,
	0x220200865090201ULL,
	0x2010100a02021202ULL,
	0x152048408022401ULL,
	0x20080002081110ULL,
	0x4001001021004000ULL,
	0x800040400a011002ULL,
	0xe4004081011002ULL,
	0x1c004001012080ULL,
	0x8004200962a00220ULL,
	0x8422100208500202ULL,
	0x2000402200300c08ULL,
	0x8646020080080080ULL,
	0x80020a0200100808ULL,
	0x2010004880111000ULL,
	0x623000a080011400ULL,
	0x42008c0340209202ULL,
	0x209188240001000ULL,
	0x400408a884001800ULL,
	0x110400a6080400ULL,
	0x1840060a44020800ULL,
	0x90080104000041ULL,
	0x201011000808101ULL,
	0x1a2208080504f080ULL,
	0x8012020600211212ULL,
	0x500861011240000ULL,
	0x180806108200800ULL,
	0x4000020e01040044ULL,
	0x300000261044000aULL,
	0x802241102020002ULL,
	0x20906061210001ULL,
	0x5a84841004010310ULL,
	0x4010801011c04ULL,
	0xa010109502200ULL,
	0x4a02012000ULL,
	0x500201010098b028ULL,
	0x8040002811040900ULL,
	0x28000010020204ULL,
	0x6000020202d0240ULL,
	0x8918844842082200ULL,
	0x4010011029020020ULL
};

#pragma endregion
