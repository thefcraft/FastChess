#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h> // qsort


#define ulld unsigned long long int
#define ushort unsigned short
#define piece char
#define move ulld
#define one (ulld)0x0000000000000001
#define BOARD_SIZE 8
#define SET_BIT(bitboard, square) (bitboard |= (1ULL << (square)))
#define CLEAR_BIT(bitboard, square) (bitboard &= ~(1ULL << (square)))
#define WHITE true
#define BLACK false
// TODO LIST
// 1) add pinnig system Done ...
//    - use same color piece map and check single piece then restrict movement to checks squares... and many other things we have to do in this process
//    - or use temp_move is_check() method (i like this one as it is easier to do)
//    - # DONE is_valid_move on castling not implemented yet inside legal_move_king fn
// 2) add upgrade piece system Done ... 
//    - # DONE add this feature into legal_move_pawn
// 3) add enPassant system Done ...
// 4) is_game_over = len(legal_moves) == 0 Done ...
// 5) # DONE fen = '8/4Q1r1/2p3P1/3k1NP1/n5P1/4p3/5K2/B7 w - - 0 68' don't stop checks by killing something
// 6) # DONE fen = '8/8/1b4p1/3q1k1p/1Pp1ppP1/2P4P/4QP2/2B2K2 b - g3 0 45' enpassant killing during check not working
// MAY # BUG is_attackers_exclude_pos may not contain errors when casting or another thing happened as it not exclude other piece to exclude

// .) # AI MINMAX Done ... # BUG
// .) # AI ALPHABETA Done ... # TODO short moves to increase % of prunning # BUG
// TODO
// In chess, the halfmove clock is a counter that keeps track of the number of halfmoves since the last capture or pawn move. 
// A halfmove consists of a move by one player followed by a move by the other player. 
// The halfmove clock is used primarily in chess notation and in the fifty-move rule, which states that a player can claim a draw 
// if no capture or pawn move has occurred in the last fifty moves by each player.

const piece BLACK_KING = 'k';
const piece BLACK_QUEEN = 'q';
const piece BLACK_KNIGHT = 'n';
const piece BLACK_BISHOP = 'b';
const piece BLACK_ROOK = 'r';
const piece BLACK_PAWN = 'p';

const piece WHITE_KING = 'K';
const piece WHITE_QUEEN = 'Q';
const piece WHITE_KNIGHT = 'N';
const piece WHITE_BISHOP = 'B';
const piece WHITE_ROOK = 'R';
const piece WHITE_PAWN = 'P';

piece EMPTY_SQUARE = '.';

typedef struct State{
    ulld BLACK_KING;
    ulld BLACK_QUEEN;
    ulld BLACK_KNIGHT;
    ulld BLACK_ROOK;
    ulld BLACK_PAWN;
    ulld BLACK_BISHOP;

    ulld WHITE_KING;
    ulld WHITE_QUEEN;
    ulld WHITE_KNIGHT;
    ulld WHITE_ROOK;
    ulld WHITE_PAWN;
    ulld WHITE_BISHOP;

    bool whiteTurn;
    ushort castlingRights; // KQkq
    ulld enPassantSquare; 

    ushort halfmoveClock;
    ushort fullmoveCounter;
} State;

typedef struct Box64{
    bool squares[64];
} Box64;

typedef struct repr_str{
    char s[65];
} repr_str;

typedef struct char_4096{
    char s[4097];
} char_4096;


void temp_print_move(move result, int pos) {
    for (int i = 63; i >= 0; i--) {
        ulld k = result >> i;
        if (i == pos) putchar('@');
        else if (k & 1) putchar('+');
        else putchar('.');
        
        if (i!=0 & i%8==0) putchar('\n');
        else putchar(' ');
    }
    putchar('\n');
}

// DECLARE Functions
bool is_attackers(State *, int, bool);
bool is_attackers_exclude_pos(State *, int, bool, ulld);

void init_default(State *state){
    state->BLACK_KING =   0x0800000000000000;
    state->BLACK_QUEEN =  0x1000000000000000;
    state->BLACK_KNIGHT = 0x4200000000000000;
    state->BLACK_ROOK =   0x8100000000000000;
    state->BLACK_PAWN =   0x00ff000000000000;
    state->BLACK_BISHOP = 0x2400000000000000;

    state->WHITE_KING =   0x0000000000000008;
    state->WHITE_QUEEN =  0x0000000000000010;
    state->WHITE_KNIGHT = 0x0000000000000042;
    state->WHITE_ROOK =   0x0000000000000081;
    state->WHITE_PAWN =   0x000000000000ff00;
    state->WHITE_BISHOP = 0x0000000000000024;

    state->whiteTurn = true;
    state->castlingRights =  0xf;
    state->enPassantSquare = 0x0000000000000000;
    state->halfmoveClock =   0x0000000000000000;
    state->fullmoveCounter = 0x0000000000000001;
}
void init(State* state, char* fen, bool whiteTurn, ushort castlingRights, ulld enPassantSquare, ushort halfmoveClock, ushort fullmoveCounter){    
    state->BLACK_KING =   0x0000000000000000;
    state->BLACK_QUEEN =  0x0000000000000000;
    state->BLACK_KNIGHT = 0x0000000000000000;
    state->BLACK_ROOK =   0x0000000000000000;
    state->BLACK_PAWN =   0x0000000000000000;
    state->BLACK_BISHOP = 0x0000000000000000;
    state->WHITE_KING =   0x0000000000000000;
    state->WHITE_QUEEN =  0x0000000000000000;
    state->WHITE_KNIGHT = 0x0000000000000000;
    state->WHITE_ROOK =   0x0000000000000000;
    state->WHITE_PAWN =   0x0000000000000000;
    state->WHITE_BISHOP = 0x0000000000000000;
    
    for (ulld i = 0; i < 64; i++)
    {
        ulld n = 1;
        n = n << i;
        char pce = fen[63-i];
        
        if (pce == BLACK_KING) state->BLACK_KING|=n;
        else if (pce == BLACK_QUEEN) state->BLACK_QUEEN|=n;
        else if (pce == BLACK_KNIGHT) state->BLACK_KNIGHT|=n;
        else if (pce == BLACK_ROOK) state->BLACK_ROOK|=n;
        else if (pce == BLACK_PAWN) state->BLACK_PAWN|=n;
        else if (pce == BLACK_BISHOP) state->BLACK_BISHOP|=n;

        else if (pce == WHITE_KING) state->WHITE_KING|=n;
        else if (pce == WHITE_QUEEN) state->WHITE_QUEEN|=n;
        else if (pce == WHITE_KNIGHT) state->WHITE_KNIGHT|=n;
        else if (pce == WHITE_ROOK) state->WHITE_ROOK|=n;
        else if (pce == WHITE_PAWN) state->WHITE_PAWN|=n;
        else if (pce == WHITE_BISHOP) state->WHITE_BISHOP|=n;
    }
    
    state->whiteTurn = whiteTurn;
    state->castlingRights =  castlingRights;
    state->enPassantSquare = enPassantSquare;
    state->halfmoveClock =   halfmoveClock;
    state->fullmoveCounter = fullmoveCounter;
}

repr_str repr(State *state){
    repr_str s;
    s.s[64] = '\0';

    ulld n = state->BLACK_KING;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i; // right shift
        if (k & 1) s.s[63-i] = 'k';
        else s.s[63-i] = '.';
    }

    n = state->BLACK_QUEEN;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'q';
    }
    
    n = state->BLACK_KNIGHT;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'n';
    }
    n = state->BLACK_ROOK;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'r';
    }
    n = state->BLACK_PAWN;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'p';
    }
    n = state->BLACK_BISHOP;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'b';
    }

    n = state->WHITE_KING;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'K';
    }
    n = state->WHITE_QUEEN;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'Q';
    }
    n = state->WHITE_KNIGHT;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'N';
    }
    n = state->WHITE_ROOK;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'R';
    }
    n = state->WHITE_PAWN;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'P';
    }
    n = state->WHITE_BISHOP;
    for (int i = 63; i >= 0; i--) {
        ulld k = n >> i;
        if (k & 1) s.s[63-i] = 'B';
    }
    return s;
}

int pos_decoder(int pos){
    return 8*(pos/8) + (7-pos%8);
}

piece piece_at(State *state, int pos){    
    pos = pos_decoder(pos);

    if (((state->BLACK_KING) >> pos) & 1)   return BLACK_KING;
    if (((state->BLACK_QUEEN) >> pos) & 1)  return BLACK_QUEEN;
    if (((state->BLACK_KNIGHT) >> pos) & 1) return BLACK_KNIGHT;
    if (((state->BLACK_ROOK) >> pos) & 1)   return BLACK_ROOK;
    if (((state->BLACK_PAWN) >> pos) & 1)   return BLACK_PAWN;
    if (((state->BLACK_BISHOP) >> pos) & 1) return BLACK_BISHOP;

    if (((state->WHITE_KING) >> pos) & 1)   return WHITE_KING;
    if (((state->WHITE_QUEEN) >> pos) & 1)  return WHITE_QUEEN;
    if (((state->WHITE_KNIGHT) >> pos) & 1) return WHITE_KNIGHT;
    if (((state->WHITE_ROOK) >> pos) & 1)   return WHITE_ROOK;
    if (((state->WHITE_PAWN) >> pos) & 1)   return WHITE_PAWN;
    if (((state->WHITE_BISHOP) >> pos) & 1) return WHITE_BISHOP;

    return EMPTY_SQUARE;
}

ulld piece_map_all(State *state) {
    return state->WHITE_KING | state->WHITE_QUEEN | state->WHITE_KNIGHT | state->WHITE_ROOK | state->WHITE_PAWN | state->WHITE_BISHOP | 
           state->BLACK_KING | state->BLACK_QUEEN | state->BLACK_KNIGHT | state->BLACK_ROOK | state->BLACK_PAWN | state->BLACK_BISHOP;
}
ulld piece_map(State *state, bool whiteColor) {
    if (whiteColor)
        return state->WHITE_KING | state->WHITE_QUEEN | state->WHITE_KNIGHT | state->WHITE_ROOK | state->WHITE_PAWN | state->WHITE_BISHOP;
    else
        return state->BLACK_KING | state->BLACK_QUEEN | state->BLACK_KNIGHT | state->BLACK_ROOK | state->BLACK_PAWN | state->BLACK_BISHOP;
}
bool is_check_exclude_pos_ulld(State *state, bool whiteColor, ulld exclude_pos){
    ulld king = whiteColor?state->WHITE_KING:state->BLACK_KING;
    for (int pos = 0; pos < 64; pos++) if ((king>>pos) & one) return is_attackers_exclude_pos(state, pos, !whiteColor, exclude_pos);
    return false;
}
bool is_check_exclude_pos(State *state, bool whiteColor, int exclude_pos){
    ulld king = whiteColor?state->WHITE_KING:state->BLACK_KING;
    for (int pos = 0; pos < 64; pos++) if ((king>>pos) & one) return is_attackers_exclude_pos(state, pos, !whiteColor, (one << exclude_pos));
    return false;
}
bool is_check(State *state, bool whiteColor){
    ulld king = whiteColor?state->WHITE_KING:state->BLACK_KING;
    for (int pos = 0; pos < 64; pos++) if ((king>>pos) & one) return is_attackers(state, pos, !whiteColor);
    return false;
}

bool is_valid_square(int pos, ulld pieces) {
    return (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE) && (!((pieces >> pos) & 1));
}
// BUG is_valid_move not implemented for killing checking pieces it just stop checks by secrifying own
bool is_valid_move_enpassant(State *state, ulld *bit, int pos, bool whiteColor) {
    SET_BIT(*bit, pos);
    bool check = is_check_exclude_pos(state, whiteColor, whiteColor?pos-BOARD_SIZE:pos+BOARD_SIZE);
    CLEAR_BIT(*bit, pos);
    return !check;
}
bool is_valid_move(State *state, ulld *bit, int pos, bool whiteColor){
    // TODO: put a piece at pos position (remove a piece from_square pos before calling this func) and check king attackers if not then return true
    SET_BIT(*bit, pos);
    // BUG if any piece is already at pos then remove it temporarily... kudos temp_killer
    // BUG enpassant killing not implemented yet
    bool check = is_check_exclude_pos(state, whiteColor, pos);
    // printf(check?"CHECK\n":"NOT\n");
    // char *s_ptr = repr(state).s;
    // for(int i = 0; i < 64; i++){
        // printf("%c", *(s_ptr+i));
        // if ((i+1)%8==0)printf("\n");
    // }
    CLEAR_BIT(*bit, pos);
    return !check;
}

move legal_move_king(State *state, int pos, bool whiteColor){
    
    move result = 0x0000000000000000;
    ulld same_color_pieces = piece_map(state, whiteColor);
    ulld pieces = piece_map_all(state);
    
    // TODO: test
    if (whiteColor) CLEAR_BIT(state->WHITE_KING, pos);
    else CLEAR_BIT(state->BLACK_KING, pos);
    ulld *bit_ptr = whiteColor?&state->WHITE_KING:&state->BLACK_KING;

    // Left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && is_valid_square(pos + 1, same_color_pieces) && is_valid_move(state, bit_ptr, pos + 1, whiteColor))
        result |= (one << (pos + 1));
    
    // Right
    if (pos % BOARD_SIZE != 0 && is_valid_square(pos - 1, same_color_pieces) && is_valid_move(state, bit_ptr, pos - 1, whiteColor))
        result |= (one << (pos - 1));
    
    // Down
    if (pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE, whiteColor))
        result |= (one << (pos - BOARD_SIZE));
    
    // Up
    if (pos / BOARD_SIZE != BOARD_SIZE - 1 && is_valid_square(pos + BOARD_SIZE, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE, whiteColor))
        result |= (one << (pos + BOARD_SIZE));
    
    // Diagonals
    // bottom-right
    if (pos % BOARD_SIZE != 0 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE - 1, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE - 1, whiteColor))
        result |= (one << (pos - BOARD_SIZE - 1));
    // bottom-left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE + 1, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE + 1, whiteColor))
        result |= (one << (pos - BOARD_SIZE + 1));
    // top-right
    if (pos % BOARD_SIZE != 0 && pos / BOARD_SIZE != BOARD_SIZE - 1 && is_valid_square(pos + BOARD_SIZE - 1, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE - 1, whiteColor))
        result |= (one << (pos + BOARD_SIZE - 1));
    // top-left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != BOARD_SIZE - 1 && is_valid_square(pos + BOARD_SIZE + 1, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE + 1, whiteColor))
        result |= (one << (pos + BOARD_SIZE + 1));
    
    // TODO castling rights ... IT MAY generate bug on is_attackers fn ...
    // KQkq
    // TODO is_valid_move on castling complications are ther as we have to predict moves of rooks also...
    if (whiteColor)
    {
        if(is_valid_square(1, pieces) && is_valid_square(2, pieces) && ((state->castlingRights >> 3) & 1) && is_valid_move(state, bit_ptr, 1, whiteColor) && is_valid_move(state, bit_ptr, 2, whiteColor) && is_valid_move(state, bit_ptr, 3, whiteColor))
            result |= (one << 1);
        if(is_valid_square(4, pieces) && is_valid_square(5, pieces) && is_valid_square(6, pieces) && ((state->castlingRights >> 2) & 1) && is_valid_move(state, bit_ptr, 4, whiteColor) && is_valid_move(state, bit_ptr, 5, whiteColor) && is_valid_move(state, bit_ptr, 3, whiteColor))
            result |= (one << 5);
    }else{
        if(is_valid_square(57, pieces) && is_valid_square(58, pieces) && ((state->castlingRights >> 1) & 1) && is_valid_move(state, bit_ptr, 57, whiteColor) && is_valid_move(state, bit_ptr, 58, whiteColor) && is_valid_move(state, bit_ptr, 59, whiteColor))
            result |= (one << 57);
        if(is_valid_square(60, pieces) && is_valid_square(61, pieces) && is_valid_square(62, pieces) && (state->castlingRights & 1) && is_valid_move(state, bit_ptr, 60, whiteColor) && is_valid_move(state, bit_ptr, 61, whiteColor) && is_valid_move(state, bit_ptr, 59, whiteColor))
            result |= (one << 61);
    }

    
    // TODO: test
    if (whiteColor) SET_BIT(state->WHITE_KING, pos);
    else SET_BIT(state->BLACK_KING, pos);
    
    return result;
}
move legal_move_queen(State *state, int pos, bool whiteColor){
    
    move result = 0x0000000000000000;
    ulld same_color_pieces = piece_map(state, whiteColor);
    ulld pieces = piece_map_all(state);

    // TODO: test
    if (whiteColor) CLEAR_BIT(state->WHITE_QUEEN, pos);
    else CLEAR_BIT(state->BLACK_QUEEN, pos);
    ulld *bit_ptr = whiteColor?&state->WHITE_QUEEN:&state->BLACK_QUEEN;

    // Left
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != BOARD_SIZE-i && is_valid_square(pos + i, pieces) && is_valid_move(state, bit_ptr, pos + i, whiteColor))
            result |= (one << (pos + i));
        else if (pos % BOARD_SIZE != BOARD_SIZE-i && is_valid_square(pos + i, same_color_pieces) && is_valid_move(state, bit_ptr, pos + i, whiteColor)){
                result |= (one << (pos + i));
                break;
            }
        else if (!(pos % BOARD_SIZE != BOARD_SIZE-i && is_valid_square(pos + i, pieces))) break;
    
    // Right
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != i-1 && is_valid_square(pos - i, pieces) && is_valid_move(state, bit_ptr, pos - i, whiteColor))
            result |= (one << (pos - i));
        else if (pos % BOARD_SIZE != i-1 && is_valid_square(pos - i, same_color_pieces) && is_valid_move(state, bit_ptr, pos - i, whiteColor)){
                result |= (one << (pos - i));
                break;
            }
        else if (!(pos % BOARD_SIZE != i-1 && is_valid_square(pos - i, pieces))) break;
    
    // Down
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i, pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i, whiteColor))
            result |= (one << (pos - BOARD_SIZE*i));
        else if (pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i, whiteColor)){
                result |= (one << (pos - BOARD_SIZE*i));
                break;
            }
        else if (!(pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i, pieces))) break;
    
    // Up
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i, pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i, whiteColor))
            result |= (one << (pos + BOARD_SIZE*i));
        else if (pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i, whiteColor)){
                result |= (one << (pos + BOARD_SIZE*i));
                break;
            }
        else if (!(pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i, pieces))) break;

    // Diagonals
    // bottom-right
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i - i, pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i - i, whiteColor))
            result |= (one << (pos - BOARD_SIZE*i - i));
        else if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i - i, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i - i, whiteColor)){
                result |= (one << (pos - BOARD_SIZE*i - i));
                break;
            }
        else if (!(pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i - i, pieces))) break;
    // bottom-left
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i + i, pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i + i, whiteColor))
            result |= (one << (pos - BOARD_SIZE*i + i));
        else if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i + i, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i + i, whiteColor)){
                result |= (one << (pos - BOARD_SIZE*i + i));
                break;
            }
        else if (!(pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i + i, pieces))) break;
    // top-right
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i - i, pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i - i, whiteColor))
            result |= (one << (pos + BOARD_SIZE*i - i));
        else if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i - i, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i - i, whiteColor)){
                result |= (one << (pos + BOARD_SIZE*i - i));
                break;
            }
        else if (!(pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i - i, pieces))) break;
    // top-left
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i + i, pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i + i, whiteColor))
            result |= (one << (pos + BOARD_SIZE*i + i));
        else if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i + i, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i + i, whiteColor)){
                result |= (one << (pos + BOARD_SIZE*i + i));
                break;
            }
        else if (!(pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i + i, pieces))) break;
    
    // TODO: test
    if (whiteColor) SET_BIT(state->WHITE_QUEEN, pos);
    else SET_BIT(state->BLACK_QUEEN, pos);
    
    return result;
}
move legal_move_knight(State *state, int pos, bool whiteColor){
    
    move result = 0x0000000000000000;
    ulld same_color_pieces = piece_map(state, whiteColor);

    // TODO: test
    if (whiteColor) CLEAR_BIT(state->WHITE_KNIGHT, pos);
    else CLEAR_BIT(state->BLACK_KNIGHT, pos);
    ulld *bit_ptr = whiteColor?&state->WHITE_KNIGHT:&state->BLACK_KNIGHT;
    

    // Left-up
    if (pos % BOARD_SIZE != BOARD_SIZE - 2 && pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != 7 && is_valid_square(pos + BOARD_SIZE + 2, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE + 2, whiteColor))
        result |= (one << (pos + BOARD_SIZE + 2));

    // Left-down
    if (pos % BOARD_SIZE != BOARD_SIZE - 2 && pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE + 2, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE + 2, whiteColor))
        result |= (one << (pos - BOARD_SIZE + 2));
    
    // Right-up
    if (pos % BOARD_SIZE != 0 && pos % BOARD_SIZE != 1 && pos / BOARD_SIZE != 7 && is_valid_square(pos + BOARD_SIZE - 2, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE - 2, whiteColor))
        result |= (one << (pos + BOARD_SIZE - 2));

    // Right-down
    if (pos % BOARD_SIZE != 0 && pos % BOARD_SIZE != 1 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE - 2, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE - 2, whiteColor))
        result |= (one << (pos - BOARD_SIZE - 2));
    
    // Down-left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != 1 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE*2 + 1, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*2 + 1, whiteColor))
        result |= (one << (pos - BOARD_SIZE*2 + 1));
    
    // Down-right
    if (pos % BOARD_SIZE != 0 && pos / BOARD_SIZE != 1 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE*2 - 1, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*2 - 1, whiteColor))
        result |= (one << (pos - BOARD_SIZE*2 - 1));
    
    // Up-left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != 6 && pos / BOARD_SIZE != 7 && is_valid_square(pos + BOARD_SIZE*2 + 1, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*2 + 1, whiteColor))
        result |= (one << (pos + BOARD_SIZE*2 + 1));

    // Up-right
    if (pos % BOARD_SIZE != 0 && pos / BOARD_SIZE != 6 && pos / BOARD_SIZE != 7 && is_valid_square(pos + BOARD_SIZE*2 - 1, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*2 - 1, whiteColor))
        result |= (one << (pos + BOARD_SIZE*2 - 1));

    // TODO: test
    if (whiteColor) SET_BIT(state->WHITE_KNIGHT, pos);
    else SET_BIT(state->BLACK_KNIGHT, pos);
    
    return result;
}
move legal_move_bishop(State *state, int pos, bool whiteColor){
    move result = 0x0000000000000000;
    ulld same_color_pieces = piece_map(state, whiteColor);
    ulld pieces = piece_map_all(state); 

    // TODO: test
    if (whiteColor) CLEAR_BIT(state->WHITE_BISHOP, pos);
    else CLEAR_BIT(state->BLACK_BISHOP, pos);
    ulld *bit_ptr = whiteColor?&state->WHITE_BISHOP:&state->BLACK_BISHOP;

    // Diagonals
    // bottom-right
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i - i, pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i - i, whiteColor))
            result |= (one << (pos - BOARD_SIZE*i - i));
        else if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i - i, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i - i, whiteColor)){
                result |= (one << (pos - BOARD_SIZE*i - i));
                break;
            }
        else if (!(pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i - i, pieces))) break;
    // bottom-left
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i + i, pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i + i, whiteColor))
            result |= (one << (pos - BOARD_SIZE*i + i));
        else if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i + i, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i + i, whiteColor)){
                result |= (one << (pos - BOARD_SIZE*i + i));
                break;
            }
        else if (!(pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i + i, pieces))) break;
    // top-right
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i - i, pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i - i, whiteColor))
            result |= (one << (pos + BOARD_SIZE*i - i));
        else if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i - i, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i - i, whiteColor)){
                result |= (one << (pos + BOARD_SIZE*i - i));
                break;
            }
        else if (!(pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i - i, pieces))) break;
    // top-left
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i + i, pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i + i, whiteColor))
            result |= (one << (pos + BOARD_SIZE*i + i));
        else if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i + i, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i + i, whiteColor)){
                result |= (one << (pos + BOARD_SIZE*i + i));
                break;
            }
        else if (!(pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i + i, pieces))) break;
    
    // TODO: test
    if (whiteColor) SET_BIT(state->WHITE_BISHOP, pos);
    else SET_BIT(state->BLACK_BISHOP, pos);

    return result;
}
move legal_move_rook(State *state, int pos, bool whiteColor){
    move result = 0x0000000000000000;
    ulld same_color_pieces = piece_map(state, whiteColor);
    ulld pieces = piece_map_all(state);

    // TODO: test
    if (whiteColor) CLEAR_BIT(state->WHITE_ROOK, pos);
    else CLEAR_BIT(state->BLACK_ROOK, pos);
    ulld *bit_ptr = whiteColor?&state->WHITE_ROOK:&state->BLACK_ROOK;

    // Left
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != BOARD_SIZE-i && is_valid_square(pos + i, pieces) && is_valid_move(state, bit_ptr, pos + i, whiteColor))
            result |= (one << (pos + i));
        else if (pos % BOARD_SIZE != BOARD_SIZE-i && is_valid_square(pos + i, same_color_pieces) && is_valid_move(state, bit_ptr, pos + i, whiteColor)){
                result |= (one << (pos + i));
                break;
            }
        else if (!(pos % BOARD_SIZE != BOARD_SIZE-i && is_valid_square(pos + i, pieces))) break;
    
    // Right
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != i-1 && is_valid_square(pos - i, pieces) && is_valid_move(state, bit_ptr, pos - i, whiteColor))
            result |= (one << (pos - i));
        else if (pos % BOARD_SIZE != i-1 && is_valid_square(pos - i, same_color_pieces) && is_valid_move(state, bit_ptr, pos - i, whiteColor)){
                result |= (one << (pos - i));
                break;
            }
        else if (!(pos % BOARD_SIZE != i-1 && is_valid_square(pos - i, pieces))) break;
    
    // Down
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i, pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i, whiteColor))
            result |= (one << (pos - BOARD_SIZE*i));
        else if (pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i, same_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*i, whiteColor)){
                result |= (one << (pos - BOARD_SIZE*i));
                break;
            }
        else if (!(pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i, pieces))) break;
    
    // Up
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i, pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i, whiteColor))
            result |= (one << (pos + BOARD_SIZE*i));
        else if (pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i, same_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*i, whiteColor)){
                result |= (one << (pos + BOARD_SIZE*i));
                break;
            }
        else if (!(pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i, pieces))) break;

    // TODO: test
    if (whiteColor) SET_BIT(state->WHITE_ROOK, pos);
    else SET_BIT(state->BLACK_ROOK, pos);

    return result;
}

move legal_move_black_pawn(State *state, int pos){
    move result = 0x0000000000000000;
    ulld pieces = piece_map_all(state);
    ulld opp_color_pieces = piece_map(state, true);

    // TODO: test
    CLEAR_BIT(state->BLACK_PAWN, pos);
    ulld *bit_ptr = &state->BLACK_PAWN;

    // Down
    if (is_valid_square(pos - BOARD_SIZE, pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE, false))
        result |= (one << (pos - BOARD_SIZE));

    // Down*2
    if (pos / BOARD_SIZE == BOARD_SIZE-2 && is_valid_square(pos - BOARD_SIZE, pieces) && is_valid_square(pos - BOARD_SIZE*2, pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE*2, false))
        result |= (one << (pos - BOARD_SIZE*2));

    // bottom-right
    if (pos % BOARD_SIZE != 0 && !is_valid_square(pos - BOARD_SIZE - 1, opp_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE - 1, false))
        result |= (one << (pos - BOARD_SIZE - 1));
    // bottom-left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && !is_valid_square(pos - BOARD_SIZE + 1, opp_color_pieces) && is_valid_move(state, bit_ptr, pos - BOARD_SIZE + 1, false))
        result |= (one << (pos - BOARD_SIZE + 1));

    if (state->enPassantSquare){
        // bottom-right-en
        if (pos % BOARD_SIZE != 0 && !is_valid_square(pos - BOARD_SIZE - 1, state->enPassantSquare) && is_valid_move_enpassant(state, bit_ptr, pos - BOARD_SIZE - 1, false))
            result |= (one << (pos - BOARD_SIZE - 1));
        // bottom-left-en
        if (pos % BOARD_SIZE != BOARD_SIZE - 1 && !is_valid_square(pos - BOARD_SIZE + 1, state->enPassantSquare) && is_valid_move_enpassant(state, bit_ptr, pos - BOARD_SIZE + 1, false))
            result |= (one << (pos - BOARD_SIZE + 1));
    }

    // TODO: test
    SET_BIT(state->BLACK_PAWN, pos);
    return result;
}
move legal_move_white_pawn(State *state, int pos){
    move result = 0x0000000000000000;
    ulld pieces = piece_map_all(state);
    ulld opp_color_pieces = piece_map(state, false);

    // TODO: test
    CLEAR_BIT(state->WHITE_PAWN, pos);
    ulld *bit_ptr = &state->WHITE_PAWN;

    // Up
    if (is_valid_square(pos + BOARD_SIZE, pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE, true))
        result |= (one << (pos + BOARD_SIZE));
    // Up*2
    if (pos / BOARD_SIZE == 1 && is_valid_square(pos + BOARD_SIZE, pieces) && is_valid_square(pos + BOARD_SIZE*2, pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE*2, true))
        result |= (one << (pos + BOARD_SIZE*2));
    
    // top-right
    if (pos % BOARD_SIZE != 0 && !is_valid_square(pos + BOARD_SIZE - 1, opp_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE - 1, true))
        result |= (one << (pos + BOARD_SIZE - 1));
    // top-left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && !is_valid_square(pos + BOARD_SIZE + 1, opp_color_pieces) && is_valid_move(state, bit_ptr, pos + BOARD_SIZE + 1, true))
        result |= (one << (pos + BOARD_SIZE + 1));

    if (state->enPassantSquare){
        // top-right
        if (pos % BOARD_SIZE != 0 && !is_valid_square(pos + BOARD_SIZE - 1, state->enPassantSquare) && is_valid_move_enpassant(state, bit_ptr, pos + BOARD_SIZE - 1, true))
            result |= (one << (pos + BOARD_SIZE - 1));
        // top-left
        if (pos % BOARD_SIZE != BOARD_SIZE - 1 && !is_valid_square(pos + BOARD_SIZE + 1, state->enPassantSquare) && is_valid_move_enpassant(state, bit_ptr, pos + BOARD_SIZE + 1, true))
            result |= (one << (pos + BOARD_SIZE + 1));
    }

    // TODO: test
    SET_BIT(state->WHITE_PAWN, pos);

    return result;
}
move legal_move_pawn(State *state, int pos, bool whiteColor){
    // temp_print_move(state->enPassantSquare, -1);
    if (whiteColor)
        return legal_move_white_pawn(state, pos);
    else
        return legal_move_black_pawn(state, pos);
}

move legal_move_by_square_helper(State *state, int pos){
    piece pce = piece_at(state, pos);
    move None = 0x0000000000000000;

    if (pce == '.') return None;
    if ((state->whiteTurn) && (pce > 'a' && pce < 'z')) return None;
    if (!(state->whiteTurn) && (pce > 'A' && pce < 'Z')) return None;
    
    pos = pos_decoder(pos);
    
    switch (pce)
    {
        case 'k': return legal_move_king(state, pos, false);
        case 'K': return legal_move_king(state, pos, true);
        case 'q': return legal_move_queen(state, pos, false);
        case 'Q': return legal_move_queen(state, pos, true);
        case 'n': return legal_move_knight(state, pos, false);
        case 'N': return legal_move_knight(state, pos, true);
        case 'b': return legal_move_bishop(state, pos, false);
        case 'B': return legal_move_bishop(state, pos, true);
        case 'r': return legal_move_rook(state, pos, false);
        case 'R': return legal_move_rook(state, pos, true);
        case 'p': return legal_move_pawn(state, pos, false);
        case 'P': return legal_move_pawn(state, pos, true);
        default: return None;
    }
}
Box64 legal_move_by_square(State *state, int pos){
    move moves = legal_move_by_square_helper(state, pos);
    Box64 result;
    for (int i = 0; i < 64; i++)
        if ((moves>>i) & 1) result.squares[pos_decoder(i)] = true;
        else result.squares[pos_decoder(i)] = false;
    return result;
}

char_4096 legal_moves(State *state){
    char_4096 result;
    result.s[4096] = '\0';
    int idx = 0;
    ulld pieces = piece_map(state, state->whiteTurn);
    for (int pos_start = 0; pos_start < 64; pos_start++) if ((pieces>>pos_start) & 1){
        
        
        move moves = legal_move_by_square_helper(state, pos_decoder(pos_start));

        
        if(moves){
            result.s[idx++] = pos_decoder(pos_start)+101;
            for (int pos_end = 0; pos_end < 64; pos_end++) if ((moves>>pos_end) & 1){
                if (state->whiteTurn && pos_end / BOARD_SIZE == 7 && ((state->WHITE_PAWN>>pos_start)&one)) result.s[idx++] = 201;
                else if (!state->whiteTurn && pos_end / BOARD_SIZE == 0 && ((state->BLACK_PAWN>>pos_start)&one)) result.s[idx++] = 202;
                result.s[idx++] = pos_decoder(pos_end)+1;
            }
        }   
    }
    result.s[idx] = '\0';
    return result;
}

// TODO upgrade piece
void upgrade_pawn(State *state, int pos, char type){
    pos = pos_decoder(pos);
    switch (type)
    {
        case 'Q':
            CLEAR_BIT(state->WHITE_PAWN, pos);
            SET_BIT(state->WHITE_QUEEN, pos);
            break;
        case 'q':
            CLEAR_BIT(state->BLACK_PAWN, pos);
            SET_BIT(state->BLACK_QUEEN, pos);
            break;
        case 'N':
            CLEAR_BIT(state->WHITE_PAWN, pos);
            SET_BIT(state->WHITE_KNIGHT, pos);
            break;
        case 'n':
            CLEAR_BIT(state->BLACK_PAWN, pos);
            SET_BIT(state->BLACK_KNIGHT, pos);
            break;
        case 'B':
            CLEAR_BIT(state->WHITE_PAWN, pos);
            SET_BIT(state->WHITE_BISHOP, pos);
            break;
        case 'b':
            CLEAR_BIT(state->BLACK_PAWN, pos);
            SET_BIT(state->BLACK_BISHOP, pos);
            break;
        case 'R':
            CLEAR_BIT(state->WHITE_PAWN, pos);
            SET_BIT(state->WHITE_ROOK, pos);
            break;
        case 'r':
            CLEAR_BIT(state->BLACK_PAWN, pos);
            SET_BIT(state->BLACK_ROOK, pos);
            break;
        default: break;
    }
}


void push_move_by_square(State *state, int start, int end){
    // Box64 moves = legal_move_by_square(state, start);
    // if (moves.squares[end]) ;
    // TODO castling DONE ...
    // TODO upgrade piece
    // TODO kill piece DONE ...
    if(!state->whiteTurn) state->fullmoveCounter++;
    state->halfmoveClock++; 
    state->whiteTurn = !state->whiteTurn;

    piece pce_start = piece_at(state, start);
    piece pce_end = piece_at(state, end);
    start = pos_decoder(start);
    end = pos_decoder(end);
    // TODO pce_end_enpassant
    ulld pce_end_enpassant = state->enPassantSquare;
    // TODO enpassant
    state->enPassantSquare = 0x0000000000000000;
    switch (pce_start)
    {
        case 'k': 
            CLEAR_BIT(state->BLACK_KING, start);
            SET_BIT(state->BLACK_KING, end);
            state->castlingRights &= ~((1<<1)|(1));
            if (start == 59 && end == 57){
                CLEAR_BIT(state->BLACK_ROOK, 56);
                SET_BIT(state->BLACK_ROOK, 58);
            }else if (start == 59 && end == 61){
                CLEAR_BIT(state->BLACK_ROOK, 63);
                SET_BIT(state->BLACK_ROOK, 60);
            }
            break;
        case 'K': 
            CLEAR_BIT(state->WHITE_KING, start);
            SET_BIT(state->WHITE_KING, end);
            state->castlingRights &= ~((1<<3)|(1<<2));
            if (start == 3 && end == 1){
                CLEAR_BIT(state->WHITE_ROOK, 0);
                SET_BIT(state->WHITE_ROOK, 2);
            }else if (start == 3 && end == 5){
                CLEAR_BIT(state->WHITE_ROOK, 7);
                SET_BIT(state->WHITE_ROOK, 4);
            }
            break;
        case 'q':
            CLEAR_BIT(state->BLACK_QUEEN, start);
            SET_BIT(state->BLACK_QUEEN, end);
            break;
        case 'Q': 
            CLEAR_BIT(state->WHITE_QUEEN, start);
            SET_BIT(state->WHITE_QUEEN, end);
            break;
        case 'n': 
            CLEAR_BIT(state->BLACK_KNIGHT, start);
            SET_BIT(state->BLACK_KNIGHT, end);
            break;
        case 'N': 
            CLEAR_BIT(state->WHITE_KNIGHT, start);
            SET_BIT(state->WHITE_KNIGHT, end);
            break;
        case 'b': 
            CLEAR_BIT(state->BLACK_BISHOP, start);
            SET_BIT(state->BLACK_BISHOP, end);
            break;
        case 'B': 
            CLEAR_BIT(state->WHITE_BISHOP, start);
            SET_BIT(state->WHITE_BISHOP, end);
            break;
        case 'r': 
            CLEAR_BIT(state->BLACK_ROOK, start);
            SET_BIT(state->BLACK_ROOK, end);
            if (start == 63)state->castlingRights &= ~(1);
            else if (start == 56)state->castlingRights &= ~(1<<1);
            break;
        case 'R': 
            CLEAR_BIT(state->WHITE_ROOK, start);
            SET_BIT(state->WHITE_ROOK, end);
            if (start == 7)state->castlingRights &= ~(1<<2);
            else if (start == 0)state->castlingRights &= ~(1<<3);
            break;
        case 'p': 
            // TODO enpassant
            state->halfmoveClock=0;
            CLEAR_BIT(state->BLACK_PAWN, start);
            SET_BIT(state->BLACK_PAWN, end);
            if (start - BOARD_SIZE*2 == end) SET_BIT(state->enPassantSquare, start - BOARD_SIZE);
            if ((one<<end) == pce_end_enpassant) CLEAR_BIT(state->WHITE_PAWN, end + BOARD_SIZE);
            break;
        case 'P': 
            state->halfmoveClock=0;
            CLEAR_BIT(state->WHITE_PAWN, start);
            SET_BIT(state->WHITE_PAWN, end);
            if (start + BOARD_SIZE*2 == end) SET_BIT(state->enPassantSquare, start + BOARD_SIZE);
            if ((one<<end) == pce_end_enpassant) CLEAR_BIT(state->BLACK_PAWN, end - BOARD_SIZE);
            break;
        default: break;
    }
    switch (pce_end)
    {
        case 'k': 
            CLEAR_BIT(state->BLACK_KING, end);
            state->halfmoveClock=0;
            break;
        case 'K': 
            CLEAR_BIT(state->WHITE_KING, end);
            state->halfmoveClock=0;
            break;
        case 'q':
            CLEAR_BIT(state->BLACK_QUEEN, end);
            state->halfmoveClock=0;
            break;
        case 'Q': 
            CLEAR_BIT(state->WHITE_QUEEN, end);
            state->halfmoveClock=0;
            break;
        case 'n': 
            CLEAR_BIT(state->BLACK_KNIGHT, end);
            state->halfmoveClock=0;
            break;
        case 'N': 
            CLEAR_BIT(state->WHITE_KNIGHT, end);
            state->halfmoveClock=0;
            break;
        case 'b': 
            CLEAR_BIT(state->BLACK_BISHOP, end);
            state->halfmoveClock=0;
            break;
        case 'B': 
            CLEAR_BIT(state->WHITE_BISHOP, end);
            state->halfmoveClock=0;
            break;
        case 'r': 
            CLEAR_BIT(state->BLACK_ROOK, end);
            state->halfmoveClock=0;
            break;
        case 'R': 
            CLEAR_BIT(state->WHITE_ROOK, end);
            state->halfmoveClock=0;
            break;
        case 'p': 
            CLEAR_BIT(state->BLACK_PAWN, end);
            state->halfmoveClock=0;
            break;
        case 'P': 
            CLEAR_BIT(state->WHITE_PAWN, end);
            state->halfmoveClock=0;
            break;
        default: break;
    }
}

move sudo_legal_move_king(State *state, int pos, bool whiteColor){
    
    move result = 0x0000000000000000;
    ulld same_color_pieces = piece_map(state, whiteColor);
    ulld pieces = piece_map_all(state);

    // Left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && is_valid_square(pos + 1, same_color_pieces))
        result |= (one << (pos + 1));
    
    // Right
    if (pos % BOARD_SIZE != 0 && is_valid_square(pos - 1, same_color_pieces))
        result |= (one << (pos - 1));
    
    // Down
    if (pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE, same_color_pieces))
        result |= (one << (pos - BOARD_SIZE));
    
    // Up
    if (pos / BOARD_SIZE != BOARD_SIZE - 1 && is_valid_square(pos + BOARD_SIZE, same_color_pieces))
        result |= (one << (pos + BOARD_SIZE));
    
    // Diagonals
    // bottom-right
    if (pos % BOARD_SIZE != 0 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE - 1, same_color_pieces))
        result |= (one << (pos - BOARD_SIZE - 1));
    // bottom-left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE + 1, same_color_pieces))
        result |= (one << (pos - BOARD_SIZE + 1));
    // top-right
    if (pos % BOARD_SIZE != 0 && pos / BOARD_SIZE != BOARD_SIZE - 1 && is_valid_square(pos + BOARD_SIZE - 1, same_color_pieces))
        result |= (one << (pos + BOARD_SIZE - 1));
    // top-left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != BOARD_SIZE - 1 && is_valid_square(pos + BOARD_SIZE + 1, same_color_pieces))
        result |= (one << (pos + BOARD_SIZE + 1));
    
    // TODO castling rights ... IT MAY generate bug on is_attackers fn ...
    // KQkq
    if (whiteColor)
    {
        if(is_valid_square(1, pieces) && is_valid_square(2, pieces) && ((state->castlingRights >> 3) & 1))
            result |= (one << 1);
        if(is_valid_square(4, pieces) && is_valid_square(5, pieces) && is_valid_square(6, pieces) && ((state->castlingRights >> 2) & 1))
            result |= (one << 5);
    }else{
        if(is_valid_square(57, pieces) && is_valid_square(58, pieces) && ((state->castlingRights >> 1) & 1))
            result |= (one << 57);
        if(is_valid_square(60, pieces) && is_valid_square(61, pieces) && is_valid_square(62, pieces) && (state->castlingRights & 1))
            result |= (one << 61);
    }

    return result;
}
move sudo_legal_move_knight(State *state, int pos, bool whiteColor){
    
    move result = 0x0000000000000000;
    ulld same_color_pieces = piece_map(state, whiteColor);

    // Left-up
    if (pos % BOARD_SIZE != BOARD_SIZE - 2 && pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != 7 && is_valid_square(pos + BOARD_SIZE + 2, same_color_pieces))
        result |= (one << (pos + BOARD_SIZE + 2));

    // Left-down
    if (pos % BOARD_SIZE != BOARD_SIZE - 2 && pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE + 2, same_color_pieces))
        result |= (one << (pos - BOARD_SIZE + 2));
    
    // Right-up
    if (pos % BOARD_SIZE != 0 && pos % BOARD_SIZE != 1 && pos / BOARD_SIZE != 7 && is_valid_square(pos + BOARD_SIZE - 2, same_color_pieces))
        result |= (one << (pos + BOARD_SIZE - 2));

    // Right-down
    if (pos % BOARD_SIZE != 0 && pos % BOARD_SIZE != 1 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE - 2, same_color_pieces))
        result |= (one << (pos - BOARD_SIZE - 2));
    
    // Down-left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != 1 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE*2 + 1, same_color_pieces))
        result |= (one << (pos - BOARD_SIZE*2 + 1));
    
    // Down-right
    if (pos % BOARD_SIZE != 0 && pos / BOARD_SIZE != 1 && pos / BOARD_SIZE != 0 && is_valid_square(pos - BOARD_SIZE*2 - 1, same_color_pieces))
        result |= (one << (pos - BOARD_SIZE*2 - 1));
    
    // Up-left
    if (pos % BOARD_SIZE != BOARD_SIZE - 1 && pos / BOARD_SIZE != 6 && pos / BOARD_SIZE != 7 && is_valid_square(pos + BOARD_SIZE*2 + 1, same_color_pieces))
        result |= (one << (pos + BOARD_SIZE*2 + 1));

    // Up-right
    if (pos % BOARD_SIZE != 0 && pos / BOARD_SIZE != 6 && pos / BOARD_SIZE != 7 && is_valid_square(pos + BOARD_SIZE*2 - 1, same_color_pieces))
        result |= (one << (pos + BOARD_SIZE*2 - 1));

    return result;
}
move sudo_legal_move_bishop(State *state, int pos, bool whiteColor){
    move result = 0x0000000000000000;
    ulld same_color_pieces = piece_map(state, whiteColor);
    ulld pieces = piece_map_all(state); 

    // Diagonals
    // bottom-right
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i - i, pieces))
            result |= (one << (pos - BOARD_SIZE*i - i));
        else if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i - i, same_color_pieces)){
                result |= (one << (pos - BOARD_SIZE*i - i));
                break;
            }
        else break;
    // bottom-left
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i + i, pieces))
            result |= (one << (pos - BOARD_SIZE*i + i));
        else if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i + i, same_color_pieces)){
                result |= (one << (pos - BOARD_SIZE*i + i));
                break;
            }
        else break;
    // top-right
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i - i, pieces))
            result |= (one << (pos + BOARD_SIZE*i - i));
        else if (pos % BOARD_SIZE != i-1 && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i - i, same_color_pieces)){
                result |= (one << (pos + BOARD_SIZE*i - i));
                break;
            }
        else break;
    // top-left
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i + i, pieces))
            result |= (one << (pos + BOARD_SIZE*i + i));
        else if (pos % BOARD_SIZE != BOARD_SIZE - i && pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i + i, same_color_pieces)){
                result |= (one << (pos + BOARD_SIZE*i + i));
                break;
            }
        else break;

    return result;
}
move sudo_legal_move_rook(State *state, int pos, bool whiteColor){
    move result = 0x0000000000000000;
    ulld same_color_pieces = piece_map(state, whiteColor);
    ulld pieces = piece_map_all(state);

    // Left
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != BOARD_SIZE-i && is_valid_square(pos + i, pieces))
            result |= (one << (pos + i));
        else if (pos % BOARD_SIZE != BOARD_SIZE-i && is_valid_square(pos + i, same_color_pieces)){
                result |= (one << (pos + i));
                break;
            }
        else break;
    
    // Right
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos % BOARD_SIZE != i-1 && is_valid_square(pos - i, pieces))
            result |= (one << (pos - i));
        else if (pos % BOARD_SIZE != i-1 && is_valid_square(pos - i, same_color_pieces)){
                result |= (one << (pos - i));
                break;
            }
        else break;
    
    // Down
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i, pieces))
            result |= (one << (pos - BOARD_SIZE*i));
        else if (pos / BOARD_SIZE != i-1 && is_valid_square(pos - BOARD_SIZE*i, same_color_pieces)){
                result |= (one << (pos - BOARD_SIZE*i));
                break;
            }
        else break;
    
    // Up
    for(int i = 1; i < BOARD_SIZE; i++)
        if (pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i, pieces))
            result |= (one << (pos + BOARD_SIZE*i));
        else if (pos / BOARD_SIZE != BOARD_SIZE - i && is_valid_square(pos + BOARD_SIZE*i, same_color_pieces)){
                result |= (one << (pos + BOARD_SIZE*i));
                break;
            }
        else break;
    return result;
}
bool is_attackers_exclude_pos(State *state, int pos, bool whiteColor, ulld exclude_pos) {
    move temp_move_rook = sudo_legal_move_rook(state, pos, !whiteColor);
    move temp_move_bishop = sudo_legal_move_bishop(state, pos, !whiteColor);
    move temp_move_king = sudo_legal_move_king(state, pos, !whiteColor);
    move temp_move_knight = sudo_legal_move_knight(state, pos, !whiteColor);
    
    ulld rook = whiteColor?state->WHITE_ROOK:state->BLACK_ROOK;
    ulld king = whiteColor?state->WHITE_KING:state->BLACK_KING;
    ulld queen = whiteColor?state->WHITE_QUEEN:state->BLACK_QUEEN;
    ulld bishop = whiteColor?state->WHITE_BISHOP:state->BLACK_BISHOP;
    ulld knight = whiteColor?state->WHITE_KNIGHT:state->BLACK_KNIGHT;
    ulld pawn = whiteColor?state->WHITE_PAWN:state->BLACK_PAWN;
    
    if (temp_move_rook & (rook & ~(exclude_pos)))return true;
    if (temp_move_bishop & (bishop & ~(exclude_pos)))return true;
    if ((temp_move_rook & (queen & ~(exclude_pos)))|(temp_move_bishop & (queen & ~(exclude_pos))))return true;
    if (temp_move_knight & (knight & ~(exclude_pos)))return true;
    if (temp_move_king & (king & ~(exclude_pos)))return true;
    
    // TODO exclude pos
    // pawn 
    if (whiteColor)
    {   
        // return (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE) && (!);
        // bottom-right
        if (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE && pos % BOARD_SIZE != 0 && (((pawn & ~(exclude_pos)) >> (pos - BOARD_SIZE - 1)) & 1)) return true;
        // bottom-left
        if (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE && pos % BOARD_SIZE != BOARD_SIZE - 1 && (((pawn & ~(exclude_pos)) >> (pos - BOARD_SIZE + 1)) & 1)) return true;
    }else{
        // top-right
        if (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE && pos % BOARD_SIZE != 0 && (((pawn & ~(exclude_pos)) >> (pos + BOARD_SIZE - 1)) & 1)) return true;
        // top-left
        if (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE && pos % BOARD_SIZE != BOARD_SIZE - 1 && (((pawn & ~(exclude_pos)) >> (pos + BOARD_SIZE + 1)) & 1)) return true;
    }

    return false;
}
bool is_attackers(State *state, int pos, bool whiteColor) {
    move temp_move_rook = sudo_legal_move_rook(state, pos, !whiteColor);
    move temp_move_bishop = sudo_legal_move_bishop(state, pos, !whiteColor);
    move temp_move_king = sudo_legal_move_king(state, pos, !whiteColor);
    move temp_move_knight = sudo_legal_move_knight(state, pos, !whiteColor);
    
    ulld rook = whiteColor?state->WHITE_ROOK:state->BLACK_ROOK;
    ulld king = whiteColor?state->WHITE_KING:state->BLACK_KING;
    ulld queen = whiteColor?state->WHITE_QUEEN:state->BLACK_QUEEN;
    ulld bishop = whiteColor?state->WHITE_BISHOP:state->BLACK_BISHOP;
    ulld knight = whiteColor?state->WHITE_KNIGHT:state->BLACK_KNIGHT;
    ulld pawn = whiteColor?state->WHITE_PAWN:state->BLACK_PAWN;
    
    if (temp_move_rook & rook)return true;
    if (temp_move_bishop & bishop)return true;
    if ((temp_move_rook & queen)|(temp_move_bishop & queen))return true;
    if (temp_move_knight & knight)return true;
    if (temp_move_king & king)return true;
    
    // pawn 
    if (whiteColor)
    {   
        // return (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE) && (!);
        // bottom-right
        if (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE && pos % BOARD_SIZE != 0 && ((pawn >> (pos - BOARD_SIZE - 1)) & 1)) return true;
        // bottom-left
        if (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE && pos % BOARD_SIZE != BOARD_SIZE - 1 && ((pawn >> (pos - BOARD_SIZE + 1)) & 1)) return true;
    }else{
        // top-right
        if (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE && pos % BOARD_SIZE != 0 && ((pawn >> (pos + BOARD_SIZE - 1)) & 1)) return true;
        // top-left
        if (pos >= 0 && pos < BOARD_SIZE * BOARD_SIZE && pos % BOARD_SIZE != BOARD_SIZE - 1 && ((pawn >> (pos + BOARD_SIZE + 1)) & 1)) return true;
    }

    return false;
}

State* copy_state(State *state){
    State *result = malloc(sizeof(State));
    result->BLACK_KING =   state->BLACK_KING;
    result->BLACK_QUEEN =  state->BLACK_QUEEN;
    result->BLACK_KNIGHT = state->BLACK_KNIGHT;
    result->BLACK_ROOK =   state->BLACK_ROOK;
    result->BLACK_PAWN =   state->BLACK_PAWN;
    result->BLACK_BISHOP = state->BLACK_BISHOP;

    result->WHITE_KING =   state->WHITE_KING;
    result->WHITE_QUEEN =  state->WHITE_QUEEN;
    result->WHITE_KNIGHT = state->WHITE_KNIGHT;
    result->WHITE_ROOK =   state->WHITE_ROOK;
    result->WHITE_PAWN =   state->WHITE_PAWN;
    result->WHITE_BISHOP = state->WHITE_BISHOP;

    result->whiteTurn =       state->whiteTurn;
    result->castlingRights =  state->castlingRights;
    result->enPassantSquare = state->enPassantSquare;
    result->halfmoveClock =   state->halfmoveClock;
    result->fullmoveCounter = state->fullmoveCounter;
    return result;
}

State* Board(char *fen, bool whiteTurn, ushort castlingRights, ushort en_passant_square, ushort halfmoveClock, ushort fullmoveCounter){
    State *state = malloc(sizeof(State));
    
    ulld enPassantSquare = 0;
    if (en_passant_square<100) enPassantSquare = one<<pos_decoder(en_passant_square);

    if (strlen(fen) != 64) init_default(state);
    else init(state, fen, whiteTurn, castlingRights, enPassantSquare, halfmoveClock, fullmoveCounter);

    return state;
}

void del_state(State *state)
{
    free(state);
}

int position_value(piece symbol, int square){
    int WHITE_PAWN_PREFERRED_COORDINATES[] = {
                0,  0,  0,  0,  0,  0,  0,  0,
                90, 90, 90, 90, 90, 90, 90, 90,
                30, 30, 40, 60, 60, 40, 30, 30,
                10, 10, 20, 40, 40, 20, 10, 10,
                5,  5, 10, 20, 20, 10,  5,  5,
                0,  0,  0,-10,-10,  0,  0,  0,
                5, -5,-10,  0,  0,-10, -5,  5,
                0,  0,  0,  0,  0,  0,  0,  0
    };
    int BLACK_PAWN_PREFERRED_COORDINATES[] = {
                0,  0,  0,  0,  0,  0,  0,  0,
                5, -5,-10,  0,  0,-10, -5,  5,
                0,  0,  0,-10,-10,  0,  0,  0,
                5,  5, 10, 20, 20, 10,  5,  5,
                10, 10, 20, 40, 40, 20, 10, 10,
                30, 30, 40, 60, 60, 40, 30, 30,
                90, 90, 90, 90, 90, 90, 90, 90,
                0,  0,  0,  0,  0,  0,  0,  0
    };
    int WHITE_KNIGHT_PREFERRED_COORDINATES[] = {
                -50,-40,-30,-30,-30,-30,-40,-50,
                -40,-20,  0,  5,  5,  0,-20,-40,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -40,-20,  0,  0,  0,  0,-20,-40,
                -50,-40,-30,-30,-30,-30,-40,-50
    };
    int BLACK_KNIGHT_PREFERRED_COORDINATES[] = {
                -50,-40,-30,-30,-30,-30,-40,-50,
                -40,-20,  0,  0,  0,  0,-20,-40,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -40,-20,  0,  5,  5,  0,-20,-40,
                -50,-40,-30,-30,-30,-30,-40,-50,
    };
    int WHITE_BISHOP_PREFERRED_COORDINATES[] = {
                -20,-10,-10,-10,-10,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5, 10, 10,  5,  0,-10,
                -10,  5,  5, 10, 10,  5,  5,-10,
                -10,  0, 10, 15, 15, 10,  0,-10,
                -10, 10, 10, 10, 10, 10, 10,-10,
                -10,  5,  0,  0,  0,  0,  5,-10,
                -20,-10,-10,-10,-10,-10,-10,-20
    };
    int BLACK_BISHOP_PREFERRED_COORDINATES[] = {
                -20,-10,-10,-10,-10,-10,-10,-20,
                -10,  5,  0,  0,  0,  0,  5,-10,
                -10, 10, 10, 10, 10, 10, 10,-10,
                -10,  0, 10, 15, 15, 10,  0,-10,
                -10,  5, 10, 15, 15, 10,  5,-10,
                -10,  0, 10, 10, 10, 10,  0,-10,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -20,-10,-10,-10,-10,-10,-10,-20
    };
    int WHITE_ROOK_PREFERRED_COORDINATES[] = {
                0,  0,  0,  0,  0,  0,  0,  0,
                5, 20, 20, 20, 20, 20, 20,  5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                0,  0,  0,  5,  5,  0,  0,  0
    };
    int BLACK_ROOK_PREFERRED_COORDINATES[] = {
                0,  0,  0,  5,  5,  0,  0,  0,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                5, 20, 20, 20, 20, 20, 20,  5,
                0,  0,  0,  0,  0,  0,  0,  0,
    };
    int WHITE_QUEEN_PREFERRED_COORDINATES[] = {
                -20,-10,-10, -5, -5,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5,  5,  5,  5,  0,-10,
                -5,  0,  5, 10, 10,  5,  0, -5,
                -5,  0,  5, 10, 10,  5,  0, -5,
                -10,  0,  5,  5,  5,  5,  0,-10,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -20,-10,-10, -5, -5,-10,-10,-20
    };
    int BLACK_QUEEN_PREFERRED_COORDINATES[] = {
                -20,-10,-10, -5, -5,-10,-10,-20,
                -10,  0,  5,  0,  0,  0,  0,-10,
                -10,  5,  5,  5,  5,  5,  0,-10,
                -5,  0,  5, 10, 10,  5,  0, -5,
                -5,  0,  5, 10, 10,  5,  0, -5,
                -10,  0,  5,  5,  5,  5,  0,-10,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -20,-10,-10, -5, -5,-10,-10,-20
    };
    int WHITE_KING_PREFERRED_COORDINATES[] = {
                -50,-30,-30,-30,-30,-30,-30,-50,
                -30,-30,  0,  0,  0,  0,-30,-30,
                -30,-10, 20, 30, 30, 20,-10,-30,
                -30,-10, 30, 40, 40, 30,-10,-30,
                -30,-10, 30, 40, 40, 30,-10,-30,
                -30,-10, 20, 30, 30, 20,-10,-30,
                -30,-20,-10,  0,  0,-10,-20,-30,
                -50,-40,-30,-20,-20,-30,-40,-50
    };
    int BLACK_KING_PREFERRED_COORDINATES[] = {
                -50,-40,-30,-20,-20,-30,-40,-50,
                -30,-20,-10,  0,  0,-10,-20,-30,
                -30,-10, 20, 30, 30, 20,-10,-30,
                -30,-10, 30, 40, 40, 30,-10,-30,
                -30,-10, 30, 40, 40, 30,-10,-30,
                -30,-10, 20, 30, 30, 20,-10,-30,
                -30,-30,  0,  0,  0,  0,-30,-30,
                -50,-30,-30,-30,-30,-30,-30,-50
    };

    square = pos_decoder(63-square);
    switch (symbol)
    {
        case 'p': return -BLACK_PAWN_PREFERRED_COORDINATES[square];
        case 'P': return WHITE_PAWN_PREFERRED_COORDINATES[square];
        case 'r': return -BLACK_ROOK_PREFERRED_COORDINATES[square];
        case 'R': return WHITE_ROOK_PREFERRED_COORDINATES[square];
        case 'b': return -BLACK_BISHOP_PREFERRED_COORDINATES[square];
        case 'B': return WHITE_BISHOP_PREFERRED_COORDINATES[square];
        case 'n': return -BLACK_KNIGHT_PREFERRED_COORDINATES[square];
        case 'N': return WHITE_KNIGHT_PREFERRED_COORDINATES[square];
        case 'q': return -BLACK_QUEEN_PREFERRED_COORDINATES[square];
        case 'Q': return WHITE_QUEEN_PREFERRED_COORDINATES[square];
        case 'k': return -BLACK_KING_PREFERRED_COORDINATES[square];
        case 'K': return WHITE_KING_PREFERRED_COORDINATES[square];
        default: return 0;
    }
}
int piece_value_mod(piece symbol){
    switch (symbol)
    {
        case 'p': return 100;
        case 'P': return 100;
        case 'r': return 500;
        case 'R': return 500;
        case 'b': return 300;
        case 'B': return 300;
        case 'n': return 300;
        case 'N': return 300;
        case 'q': return 900;
        case 'Q': return 900;
        case 'k': return 10000;
        case 'K': return 10000;
        default: return 0;
    }
}
int piece_value(piece symbol){
    switch (symbol)
    {
        case 'p': return -100;
        case 'P': return 100;
        case 'r': return -500;
        case 'R': return 500;
        case 'b': return -300;
        case 'B': return 300;
        case 'n': return -300;
        case 'N': return 300;
        case 'q': return -900;
        case 'Q': return 900;
        case 'k': return -10000;
        case 'K': return 10000;
        default: return 0;
    }
}
bool is_game_over(State* state){
    return strlen(legal_moves(state).s) == 0;
}
short int winner(State *state){
    // TODO if attacker at black_pos # return WHITE
    for (int i = 0; i < 64; i++)
    {
        if ((state->WHITE_KING>>i)&one)
        {
            if(is_attackers(state, i, false)) return -1;
            break;
        }
    }
    for (int i = 0; i < 64; i++)
    {
        if ((state->BLACK_KING>>i)&one){
            if(is_attackers(state, i, true)) return 1;
            break;
        }
    }
    return 0;
}
int evaluate_board(State *state){
    int evaluation = 0;
    for (int square = 0; square < 64; square++)
    {
        piece pce = piece_at(state, square);
        if (pce == '.') continue;
        evaluation += piece_value(pce);
        evaluation += position_value(pce, square);
    }
    return evaluation;
}

typedef struct move_list_t{
    int start;
    int end;
    piece upgrade_type; // default '\0'
} move_list_t;

bool move_list_t_is_equal(move_list_t *a, move_list_t *b){
    return a->start == b->start && a->end == b->end && a->upgrade_type == b->upgrade_type;
}

int len_move_list(State *state){
    ulld pieces = piece_map(state, state->whiteTurn);
    int idx = 0;
    for (int pos_start = 0; pos_start < 64; pos_start++) if ((pieces>>pos_start) & 1){
        move moves = legal_move_by_square_helper(state, pos_decoder(pos_start));
        if(moves){
            for (int pos_end = 0; pos_end < 64; pos_end++) if ((moves>>pos_end) & 1){
                if (state->whiteTurn && pos_end / BOARD_SIZE == 7 && ((state->WHITE_PAWN>>pos_start)&one)) {
                    idx++;
                    idx++;
                    idx++;
                    idx++;
                }
                else if (!state->whiteTurn && pos_end / BOARD_SIZE == 0 && ((state->BLACK_PAWN>>pos_start)&one)){
                    idx++;
                    idx++;
                    idx++;
                    idx++;
                }
                else{
                    idx++;
                }
            }
        }   
    }
    return idx;
}

void set_move_list(State *state, move_list_t list[], int n){
    ulld pieces = piece_map(state, state->whiteTurn);
    int idx = 0;
    for (int pos_start = 0; pos_start < 64; pos_start++) if ((pieces>>pos_start) & 1){
        move moves = legal_move_by_square_helper(state, pos_decoder(pos_start));
        if(moves){
            for (int pos_end = 0; pos_end < 64; pos_end++) if ((moves>>pos_end) & 1){
                if (state->whiteTurn && pos_end / BOARD_SIZE == 7 && ((state->WHITE_PAWN>>pos_start)&one)) {
                    list[idx].start = pos_decoder(pos_start);
                    list[idx].end = pos_decoder(pos_end);
                    list[idx++].upgrade_type = 'Q';

                    list[idx].start = pos_decoder(pos_start);
                    list[idx].end = pos_decoder(pos_end);
                    list[idx++].upgrade_type = 'N';
                    
                    list[idx].start = pos_decoder(pos_start);
                    list[idx].end = pos_decoder(pos_end);
                    list[idx++].upgrade_type = 'B';
                    
                    list[idx].start = pos_decoder(pos_start);
                    list[idx].end = pos_decoder(pos_end);
                    list[idx++].upgrade_type = 'R';
                }
                else if (!state->whiteTurn && pos_end / BOARD_SIZE == 0 && ((state->BLACK_PAWN>>pos_start)&one)){
                    list[idx].start = pos_decoder(pos_start);
                    list[idx].end = pos_decoder(pos_end);
                    list[idx++].upgrade_type = 'q';

                    list[idx].start = pos_decoder(pos_start);
                    list[idx].end = pos_decoder(pos_end);
                    list[idx++].upgrade_type = 'n';
                    
                    list[idx].start = pos_decoder(pos_start);
                    list[idx].end = pos_decoder(pos_end);
                    list[idx++].upgrade_type = 'b';
                    
                    list[idx].start = pos_decoder(pos_start);
                    list[idx].end = pos_decoder(pos_end);
                    list[idx++].upgrade_type = 'r';
                }
                else{
                    list[idx].start = pos_decoder(pos_start);
                    list[idx].end = pos_decoder(pos_end);
                    list[idx++].upgrade_type = '\0';
                }
            }
        }   
    }
    if (idx>n) printf("ERROR: in set_move_list as n is greater than idx\n");
}
bool is_NullMove(move_list_t mve){
    return mve.start==-1 && mve.end==-1 && mve.upgrade_type == '\0';
}
void print_move_t(move_list_t mve){
    if (!is_NullMove(mve)){
        int start = mve.start;
        int end = mve.end;
        printf("%c%d%c%d", ((start % 8) + 97), (start / 8) + 1,
                        ((end % 8) + 97), (end / 8) + 1);
        if(mve.upgrade_type!='\0') putchar(mve.upgrade_type);
    }
}
void print_move_list(move_list_t list[], int n){
    for (int i = 0; i < n; i++)
    {
        print_move_t(list[i]);
        putchar(' ');
    }
    putchar('\n');
}

void push_move(State *state, int start, int end, piece upgrade_type){
    push_move_by_square(state, start, end);
    if (upgrade_type != '\0') upgrade_pawn(state, end, upgrade_type);
}
void push_move_t(State *state, move_list_t move_t){
    push_move(state, move_t.start, move_t.end, move_t.upgrade_type);
}

int minmax_helper_slow(State *state, unsigned short int depth, int *num_fn_calls, double *time_generate_move, double *time_board_copy, double *time_push_move, double *time_other){
    clock_t time_generate_move_start = clock();

    int len_legal_move = len_move_list(state);
    if (len_legal_move==0){
        switch (winner(state))
        {
            case 1: return 10000+depth;
            case -1: return -10000-depth;
            default: return 0;
        }
    }
    else if (depth == 0) return evaluate_board(state);

    move_list_t move_list[len_legal_move]; 

    set_move_list(state, move_list, len_legal_move);
    *time_generate_move += (double)(clock() - time_generate_move_start) / CLOCKS_PER_SEC;

    int best_score;
    if (state->whiteTurn) best_score = -100000;
    else best_score = 100000;

    for(int idx = 0; idx < len_legal_move; idx++){
        clock_t time_board_copy_start = clock();
        State *tmp = copy_state(state);
        *time_board_copy += (double)(clock() - time_board_copy_start) / CLOCKS_PER_SEC;
        *num_fn_calls += 1;
        
        clock_t time_push_move_start = clock();
        push_move_t(tmp, move_list[idx]);
        *time_push_move += (double)(clock() - time_push_move_start) / CLOCKS_PER_SEC;

        clock_t time_helper_call_start = clock();
        int score = minmax_helper_slow(tmp, depth-1, num_fn_calls, time_generate_move, time_board_copy, time_push_move, time_other);
        *time_other -= (double)(clock() - time_helper_call_start) / CLOCKS_PER_SEC;
        
        if (state->whiteTurn && score > best_score) best_score = score;
        else if(!state->whiteTurn && score < best_score) best_score = score;

        del_state(tmp);
        
    }
    *time_other += (double)(clock() - time_generate_move_start) / CLOCKS_PER_SEC;
    return best_score;
}
// TODO recursive function taking too much time...
move_list_t minmax_slow(State *state, unsigned short int depth, bool debug){
    clock_t time_start = clock();
    int bestmove_idx;
    int num_fn_calls = 0;
    double time_generate_move = 0;
    double time_board_copy = 0;
    double time_push_move = 0;
    double time_other = 0;
    double time_helper_call = 0;

    clock_t time_generate_move_start = clock();

    int len_legal_move = len_move_list(state);
    move_list_t move_list[len_legal_move]; 

    set_move_list(state, move_list, len_legal_move);
    time_generate_move += (double)(clock() - time_generate_move_start) / CLOCKS_PER_SEC;

    int best_score;
    if (state->whiteTurn) best_score = -100000;
    else best_score = 100000;

    for(int idx = 0; idx < len_legal_move; idx++){
        clock_t time_board_copy_start = clock();
        State *tmp = copy_state(state);
        time_board_copy += (double)(clock() - time_board_copy_start) / CLOCKS_PER_SEC;
        
        num_fn_calls++;

        clock_t time_push_move_start = clock();
        push_move_t(tmp, move_list[idx]);
        time_push_move += (double)(clock() - time_push_move_start) / CLOCKS_PER_SEC;

        clock_t time_helper_call_start = clock();
        int score = minmax_helper_slow(tmp, depth-1, &num_fn_calls, &time_generate_move, &time_board_copy, &time_push_move, &time_other);
        time_helper_call += (double)(clock() - time_helper_call_start) / CLOCKS_PER_SEC;
        time_other -= (double)(clock() - time_helper_call_start) / CLOCKS_PER_SEC;


        if (state->whiteTurn && score > best_score) {
            best_score = score;
            bestmove_idx = idx;
        }
        else if(!state->whiteTurn && score < best_score) {
            best_score = score;
            bestmove_idx = idx;
        }

        del_state(tmp);
    }
    time_other += (double)(clock() - time_generate_move_start) / CLOCKS_PER_SEC;

    move_list_t bestmove = move_list[bestmove_idx];
    double elapsed_time = (double)(clock() - time_start) / CLOCKS_PER_SEC;
    if (debug)
    {
        printf("ETA: %.2f ms\n", elapsed_time*1000);
        printf("NPS: %.6f\n", ((double)num_fn_calls)/elapsed_time);
        printf("TOTAL NODES: %d\n", num_fn_calls);
        printf("MOVE: "); print_move_t(bestmove);
        putchar('\n');
        printf("PROFILE: \n");
        printf("ETA move_generated: %.2f ms\n", time_generate_move*1000);
        printf("ETA board_copy: %.2f ms\n", time_board_copy*1000);
        printf("ETA push_move: %.2f ms\n", time_push_move*1000);
        printf("ETA other: %.2f ms\n", time_other*1000);
        printf("ETA helper_call: %.2f ms\n", time_helper_call*1000);
        
    }
    return bestmove;
}

int minmax_helper(State *state, unsigned short int depth, int *num_fn_calls){
    if (is_game_over(state)){
        switch (winner(state))
        {
            case 1: return 10000+depth;
            case -1: return -10000-depth;
            default: return 0;
        }
    }
    else if (depth == 0) return evaluate_board(state);

    if (state->whiteTurn){
        int maxscore = -100000;

        ulld pieces = piece_map(state, state->whiteTurn);
        int idx = 0;
        for (int pos_start = 0; pos_start < 64; pos_start++) if ((pieces>>pos_start) & 1){
            move moves = legal_move_by_square_helper(state, pos_decoder(pos_start));
            if(moves){
                for (int pos_end = 0; pos_end < 64; pos_end++) if ((moves>>pos_end) & 1){
                    if (state->whiteTurn && pos_end / BOARD_SIZE == 7 && ((state->WHITE_PAWN>>pos_start)&one)) {
                        State *tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'Q');
                        int score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score > maxscore) maxscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'N');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score > maxscore) maxscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'B');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score > maxscore) maxscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'R');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score > maxscore) maxscore = score;
                        del_state(tmp);
                    }
                    else if (!state->whiteTurn && pos_end / BOARD_SIZE == 0 && ((state->BLACK_PAWN>>pos_start)&one)){
                        State *tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'q');
                        int score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score > maxscore) maxscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'n');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score > maxscore) maxscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'b');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score > maxscore) maxscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'r');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score > maxscore) maxscore = score;
                        del_state(tmp);
                    }
                    else{
                        State *tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), '\0');
                        int score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score > maxscore) maxscore = score;
                        del_state(tmp);
                    }
                }
            }   
        }
        
        return maxscore;
    }else{
        int minscore = 100000;

        ulld pieces = piece_map(state, state->whiteTurn);
        int idx = 0;
        for (int pos_start = 0; pos_start < 64; pos_start++) if ((pieces>>pos_start) & 1){
            move moves = legal_move_by_square_helper(state, pos_decoder(pos_start));
            if(moves){
                for (int pos_end = 0; pos_end < 64; pos_end++) if ((moves>>pos_end) & 1){
                    if (state->whiteTurn && pos_end / BOARD_SIZE == 7 && ((state->WHITE_PAWN>>pos_start)&one)) {
                        State *tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'Q');
                        int score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score < minscore) minscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'N');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score < minscore) minscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'B');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score < minscore) minscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'R');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score < minscore) minscore = score;
                        del_state(tmp);
                    }
                    else if (!state->whiteTurn && pos_end / BOARD_SIZE == 0 && ((state->BLACK_PAWN>>pos_start)&one)){
                        State *tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'q');
                        int score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score < minscore) minscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'n');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score < minscore) minscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'b');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score < minscore) minscore = score;
                        del_state(tmp);

                        tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'r');
                        score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score < minscore) minscore = score;
                        del_state(tmp);
                    }
                    else{
                        State *tmp = copy_state(state);
                        *num_fn_calls = *num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), '\0');
                        int score = minmax_helper(tmp, depth-1, num_fn_calls);
                        if (score < minscore) minscore = score;
                        del_state(tmp);
                    }
                }
            }   
        }
        
        return minscore;
    }

}
move_list_t minmax(State *state, unsigned short int depth, bool debug){
    clock_t time_start = clock();
    move_list_t bestmove;
    int num_fn_calls = 0;

    
    if (state->whiteTurn){
        int maxscore = -100000;

        ulld pieces = piece_map(state, state->whiteTurn);
        int idx = 0;
        for (int pos_start = 0; pos_start < 64; pos_start++) if ((pieces>>pos_start) & 1){
            move moves = legal_move_by_square_helper(state, pos_decoder(pos_start));
            if(moves){
                for (int pos_end = 0; pos_end < 64; pos_end++) if ((moves>>pos_end) & 1){
                    if (state->whiteTurn && pos_end / BOARD_SIZE == 7 && ((state->WHITE_PAWN>>pos_start)&one)) {
                        State *tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'Q');
                        int score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score > maxscore) {
                            maxscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'Q';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'N');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score > maxscore){
                            maxscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'N';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'B');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score > maxscore){
                            maxscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'B';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'R');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score > maxscore){
                            maxscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'R';
                        }
                        del_state(tmp);
                    }
                    else if (!state->whiteTurn && pos_end / BOARD_SIZE == 0 && ((state->BLACK_PAWN>>pos_start)&one)){
                        State *tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'q');
                        int score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score > maxscore){
                            maxscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'q';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'n');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score > maxscore){
                            maxscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'n';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'b');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score > maxscore){
                            maxscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'b';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'r');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score > maxscore){
                            maxscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'r';
                        }
                        del_state(tmp);
                    }
                    else{
                        State *tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), '\0');
                        int score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score > maxscore){
                            maxscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = '\0';
                        }
                        del_state(tmp);
                    }
                }
            }   
        }
        
    }else{
        int minscore = 100000;

        ulld pieces = piece_map(state, state->whiteTurn);
        int idx = 0;
        for (int pos_start = 0; pos_start < 64; pos_start++) if ((pieces>>pos_start) & 1){
            move moves = legal_move_by_square_helper(state, pos_decoder(pos_start));
            if(moves){
                for (int pos_end = 0; pos_end < 64; pos_end++) if ((moves>>pos_end) & 1){
                    if (state->whiteTurn && pos_end / BOARD_SIZE == 7 && ((state->WHITE_PAWN>>pos_start)&one)) {
                        State *tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'Q');
                        int score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score < minscore){
                            minscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'Q';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'N');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score < minscore){
                            minscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'N';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'B');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score < minscore){
                            minscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'B';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'R');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score < minscore){
                            minscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'R';
                        }
                        del_state(tmp);
                    }
                    else if (!state->whiteTurn && pos_end / BOARD_SIZE == 0 && ((state->BLACK_PAWN>>pos_start)&one)){
                        State *tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'q');
                        int score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score < minscore){
                            minscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'q';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'n');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score < minscore){
                            minscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'n';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'b');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score < minscore){
                            minscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'b';
                        }
                        del_state(tmp);

                        tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), 'r');
                        score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score < minscore){
                            minscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = 'r';
                        }
                        del_state(tmp);
                    }
                    else{
                        State *tmp = copy_state(state);
                        num_fn_calls = num_fn_calls + 1;
                        push_move(tmp, pos_decoder(pos_start), pos_decoder(pos_end), '\0');
                        int score = minmax_helper(tmp, depth-1, &num_fn_calls);
                        if (score < minscore){
                            minscore = score;
                            bestmove.start = pos_decoder(pos_start);
                            bestmove.end = pos_decoder(pos_end);
                            bestmove.upgrade_type = '\0';
                        }
                        del_state(tmp);
                    }
                }
            }   
        }
    }


    clock_t time_end = clock();
    double elapsed_time = (double)(time_end - time_start) / CLOCKS_PER_SEC;
    if (debug)
    {
        printf("ETA: %.6f\n", elapsed_time);
        printf("NPS: %.6f\n", ((double)num_fn_calls)/elapsed_time);
        printf("TOTAL NODES: %d\n", num_fn_calls);
        printf("MOVE: "); print_move_t(bestmove);
        putchar('\n');
    }
    return bestmove;
}

bool is_capture(State *state, move_list_t mve){
    if ((piece_map_all(state)>>pos_decoder(mve.end)) & 1) return true;
    else if ((state->enPassantSquare>>pos_decoder(mve.end)) & 1) return true;
    return false;
}
bool gives_check(State *state, move_list_t mve){
    State *tmp = copy_state(state);
    push_move_t(tmp, mve);
    bool check = is_check(tmp, tmp->whiteTurn);
    del_state(tmp);
    return check;
}
bool gives_capture(State *state, move_list_t mve){
    State *tmp = copy_state(state);
    push_move_t(tmp, mve);
    bool attacker = is_attackers(state, mve.end, !tmp->whiteTurn);
    del_state(tmp);
    return attacker;
}
int move_value(void *s, const void *a, const void *b) {
    State *state = (State *)s;
    move_list_t *moveA = (move_list_t *)a;
    move_list_t *moveB = (move_list_t *)b;

    int valueA = 0;
    int valueB = 0;

    if ((piece_map_all(state)>>pos_decoder(moveA->end)) & 1){
        valueA = (1000 * piece_value_mod(piece_at(state, moveA->end))) - piece_value_mod(piece_at(state, moveA->start));
    }else if ((state->enPassantSquare>>pos_decoder(moveA->end)) & 1){
        valueA = (1000 * piece_value_mod(state->whiteTurn?'p':'P')) - piece_value_mod(piece_at(state, moveB->start));
    }
    // else if(gives_check(state, *moveA)) valueA = 100;  takes too much time
    // else if(gives_capture(state, *moveA)) valueA = 50; takes too much time

    if ((piece_map_all(state)>>pos_decoder(moveB->end)) & 1){
        valueB = (1000 * piece_value_mod(piece_at(state, moveB->end))) - piece_value_mod(piece_at(state, moveB->start));
    }else if ((state->enPassantSquare>>pos_decoder(moveA->end)) & 1){
        valueB = (1000 * piece_value_mod(state->whiteTurn?'p':'P')) - piece_value_mod(piece_at(state, moveB->start));
    }
    // else if(gives_check(state, *moveB)) valueB = 100;  takes too much time
    // else if(gives_capture(state, *moveB)) valueB = 50; takes too much time

    return valueB - valueA; // Sort in descending order
}
void order_moves(State* state, move_list_t list[], int n){
    qsort_s(list, n, sizeof(move_list_t), move_value, state);
}


int alphabeta_helper_no_pv(State *state, unsigned short int depth, int alpha, int beta, int *num_fn_calls, move_list_t *pv){
    int len_legal_move = len_move_list(state);
    if (len_legal_move==0){
        switch (winner(state))
        {
            case 1: return 10000+depth;
            case -1: return -10000-depth;
            default: return 0;
        }
    }
    else if (depth == 0) return evaluate_board(state);

    move_list_t move_list[len_legal_move]; 
    set_move_list(state, move_list, len_legal_move);
    order_moves(state, move_list, len_legal_move);

    int bestmove_idx;

    int best_score;
    if (state->whiteTurn) best_score = -100000;
    else best_score = 100000;

    for(int idx = 0; idx < len_legal_move; idx++){
        State *tmp = copy_state(state);
        *num_fn_calls = *num_fn_calls + 1;
        push_move_t(tmp, move_list[idx]);
        move_list_t pv_tmp[depth-1];
        int score = alphabeta_helper_no_pv(tmp, depth-1, alpha, beta, num_fn_calls, pv_tmp);
        if (state->whiteTurn){
            if (score > best_score){
                best_score = score;
                bestmove_idx = idx;
                for (int i = 1; i < depth; i++) pv[i] = pv_tmp[i-1];
            }
            if(score>alpha) alpha = score;
        }else{
            if (score < best_score){
                best_score = score;
                bestmove_idx = idx;
                for (int i = 1; i < depth; i++) pv[i] = pv_tmp[i-1];
            }
            if(score<beta) beta = score;
        }
        del_state(tmp);
        if(beta<=alpha) break;
    }
    
    pv[0] = move_list[bestmove_idx];

    return best_score;
}
move_list_t alphabeta_no_pv(State *state, unsigned short int depth, bool debug){
    clock_t time_start = clock();
    int bestmove_idx;
    int num_fn_calls = 0;
    int alpha = -100000;
    int beta = 100000;

    move_list_t pv[depth];
    
    int best_score = alphabeta_helper_no_pv(state, depth, alpha, beta, &num_fn_calls, pv);

    move_list_t bestmove = pv[0];

    clock_t time_end = clock();
    double elapsed_time = (double)(time_end - time_start) / CLOCKS_PER_SEC;
    if (debug)
    {
        printf("info score cp %d depth %d nodes %d nps %d time %d pv ", best_score, depth, num_fn_calls, (int)(num_fn_calls/(elapsed_time+.0001)), (int)(elapsed_time*1000)); 
        print_move_list(pv, depth);
    }
    return bestmove;
}

typedef struct context_qsort{
    State *state;
    move_list_t *pv;
    unsigned short int depth;
} context_qsort;

int move_value_pv(void *s, const void *a, const void *b) {
    context_qsort *context = (context_qsort *)s;
    State *state = context->state;
    move_list_t *pv = context->pv;

    move_list_t *moveA = (move_list_t *)a;
    move_list_t *moveB = (move_list_t *)b;

    int valueA = 0;
    int valueB = 0;

    if (context->depth != 1 && move_list_t_is_equal(moveA, pv)) valueA = 10000;
    else if ((piece_map_all(state)>>pos_decoder(moveA->end)) & 1)
        valueA = (1000 * piece_value_mod(piece_at(state, moveA->end))) - piece_value_mod(piece_at(state, moveA->start));
    else if ((state->enPassantSquare>>pos_decoder(moveA->end)) & 1)
        valueA = (1000 * piece_value_mod(state->whiteTurn?'p':'P')) - piece_value_mod(piece_at(state, moveB->start));
    else if(gives_capture(state, *moveA)) valueA = 50; //takes too much time
    else if(gives_check(state, *moveA)) valueA = 100;  //takes too much time


    if (context->depth != 1 && move_list_t_is_equal(moveB, pv)) valueB = 10000;
    else if ((piece_map_all(state)>>pos_decoder(moveB->end)) & 1)
        valueB = (1000 * piece_value_mod(piece_at(state, moveB->end))) - piece_value_mod(piece_at(state, moveB->start));
    else if ((state->enPassantSquare>>pos_decoder(moveA->end)) & 1)
        valueB = (1000 * piece_value_mod(state->whiteTurn?'p':'P')) - piece_value_mod(piece_at(state, moveB->start));
    else if(gives_capture(state, *moveB)) valueB = 50; //takes too much time
    else if(gives_check(state, *moveB)) valueB = 100;  //takes too much time

    return valueB - valueA; // Sort in descending order
}
void order_moves_pv(State* state, move_list_t list[], int n, move_list_t *pv, unsigned short int depth){
    context_qsort context;
    context.state = state;
    context.pv = pv;
    context.depth = depth;
    qsort_s(list, n, sizeof(move_list_t), move_value_pv, &context);
}

int quiescence_search(State *state, int alpha, int beta, int *num_fn_calls){
    int stand_pat = 0;
    if (is_game_over(state)){
        switch (winner(state))
        {
            case 1: stand_pat = 10000; break;
            case -1: stand_pat = -10000; break;
        }
    }
    else stand_pat = evaluate_board(state);

    if(stand_pat >= beta) return beta;
    if(alpha < stand_pat) alpha = stand_pat;
    
    int len_legal_move = len_move_list(state);
    move_list_t move_list[len_legal_move]; 
    set_move_list(state, move_list, len_legal_move);
    
    for(int idx = 0; idx < len_legal_move; idx++)if (is_capture(state, move_list[idx])) // move.is_capture() or move.is_check() or move.is_promotion()
    {
        State *tmp = copy_state(state);
        *num_fn_calls += 1;
        push_move_t(tmp, move_list[idx]);
        int score = -quiescence_search(tmp, -beta, -alpha, num_fn_calls);
        del_state(tmp);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    
    return alpha;
}

int alphabeta_helper(State *state, unsigned short int depth, int alpha, int beta, int *num_fn_calls, move_list_t *pv, unsigned short int depth_true){
    int len_legal_move = len_move_list(state);
    if (len_legal_move==0){
        switch (winner(state))
        {
            case 1: return 10000+depth;
            case -1: return -10000-depth;
            default: return 0;
        }
    }
    else if (depth == 0) return evaluate_board(state);

    // if (depth == 0 || len_legal_move==0) return quiescence_search(state, alpha, beta, num_fn_calls); // TAKING TOO MUCH TIME TODO ADD MAX_DEPTH = 2*depth

    move_list_t move_list[len_legal_move]; 
    set_move_list(state, move_list, len_legal_move);
    order_moves_pv(state, move_list, len_legal_move, pv, depth);

    int bestmove_idx;

    int best_score = (state->whiteTurn)?-100000:100000;

    move_list_t pv_tmp[depth-1];

    for(int idx = 0; idx < len_legal_move; idx++){
        State *tmp = copy_state(state);
        *num_fn_calls+=1;
        push_move_t(tmp, move_list[idx]);
        
        for (int i = 1; i < depth; i++) pv_tmp[i-1] = pv[i]; // TODO pv sort
        int score = alphabeta_helper(tmp, depth-1, alpha, beta, num_fn_calls, pv_tmp, depth_true);
        if (state->whiteTurn){
            if (score > best_score){
                best_score = score;
                bestmove_idx = idx;
                for (int i = 1; i < depth; i++) pv[i] = pv_tmp[i-1];
            }
            if(score>alpha) alpha = score;
        }else{
            if (score < best_score){
                best_score = score;
                bestmove_idx = idx;
                for (int i = 1; i < depth; i++) pv[i] = pv_tmp[i-1];
            }
            if(score<beta) beta = score;
        }
        del_state(tmp);
        if(beta<=alpha) break;
    }
    
    pv[0] = move_list[bestmove_idx];

    return best_score;
}
move_list_t alphabeta(State *state, unsigned short int depth, bool debug){
    int bestmove_idx;
    
    int alpha = -100000;
    int beta = 100000;

    move_list_t pv[depth];

    move_list_t NullMove; NullMove.start = -1; NullMove.end = -1; NullMove.upgrade_type = '\0';
    for (int i = 0; i < depth; i++)pv[i] = NullMove;

    for (int i = 1; i <= depth; i++)
    {
        int num_fn_calls = 0;
        clock_t time_start = clock();
        int best_score = alphabeta_helper(state, i, alpha, beta, &num_fn_calls, pv, i);
        if (debug)
        {
            clock_t time_end = clock();
            double elapsed_time = (double)(time_end - time_start) / CLOCKS_PER_SEC;
            printf("info score cp %d depth %d nodes %d nps %d time %d pv ", best_score, i, num_fn_calls, (int)(num_fn_calls/(elapsed_time+.0001)), (int)(elapsed_time*1000)); 
            // BUG may as pv is garbage if game over before then depth...
            print_move_list(pv, i);
        }
    }
    return pv[0];
}