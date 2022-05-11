//#define DEBUG

#define P3_BONUS 2
#define C3_BONUS 1

#define PI_MAHJONG_MSG_LEN  255

#define TABLE_MSGKEY 0x0007AB1E
#define PLAYER_MSGKEY 0x0001234

#define MAX_TILES 144

typedef struct msg_struct {
  long mtype;   /* 1=player1 etc */
  char mtext[PI_MAHJONG_MSG_LEN];
  } PI_MAHJONG_MSG_STRUCT;

// Global variables
extern char wind_label [];
extern char * tile_label[];
#ifdef Chinese
extern char * tile_label_c[];
void chinese_display (char *tiles);
#endif
extern char mybin_type[20];
extern char mybin[5][20];
extern int qid2table;
extern int qid2player;

extern unsigned char tiles [MAX_TILES+1];
extern int tiles_count;

extern char open_discard[5][MAX_TILES+1]; // the rest are also
extern char open_meld[5][20];  // the rest as also
extern char my_secret_tiles[15];

extern long my_player;

extern int strategy_num;

// Bonus tiles (Flower and Season) are not implemented yet and therefore a deck has only 136 tiles

#define ONE_DOT    'a'
#define TWO_DOT    'b'
#define THREE_DOT  'c'
#define FOUR_DOT   'd'
#define FIVE_DOT   'e'
#define SIX_DOT    'f'
#define SEVEN_DOT  'g'
#define EIGHT_DOT  'h'
#define NINE_DOT   'i'

#define ONE_BAMBOO    'A'
#define TWO_BAMBOO    'B'
#define THREE_BAMBOO  'C'
#define FOUR_BAMBOO   'D'
#define FIVE_BAMBOO   'E'
#define SIX_BAMBOO    'F'
#define SEVEN_BAMBOO  'G'
#define EIGHT_BAMBOO  'H'
#define NINE_BAMBOO   'I'

#define ONE_CHARACTER    '1'
#define TWO_CHARACTER    '2'
#define THREE_CHARACTER  '3'
#define FOUR_CHARACTER   '4'
#define FIVE_CHARACTER   '5'
#define SIX_CHARACTER    '6'
#define SEVEN_CHARACTER  '7'
#define EIGHT_CHARACTER  '8'
#define NINE_CHARACTER   '9'

#define EAST_WIND     'p'
#define SOUTH_WIND    'q'
#define WEST_WIND     'r'
#define NORTH_WIND    's'

#define RED_DRAGON      'w'
#define GREEN_DRAGON    'x'
#define WHITE_DRAGON    'y'

// =========================================

struct register_format {
  char code;
  char initial_fund[6]; // max 99999
  } register_msg;

struct new_game_format {
  char code;
  char seat;
  };

struct new_hand_format {
  char code;
  char round;
  char hand;
  char seats[4];
  char tiles[13];
  };

struct draw_tile_format {
  char code;
  char tile;
  };

struct discard_format {
  char code;
  char discard_player;
  char discard_tile;
  };

struct pass_format {
  char code;
  };

struct win_format {
  char code;
  char win_player;
  };

struct reveal_format {
  char code;
  char fund1[5];
  char fund2[5];
  char fund3[5];
  char fund4[5];
  char reveal[100]; // player revealled tiles are separated by space
  };

struct chow_format {
  char code;
  char chow_player;
  char chow_tile;
  char other_tile1;
  char other_tile2;
  };

struct pung_format {
  char code;
  char pung_player;
  char tile;
  };

struct next_drawer_format {
  char code;
  char seat;
  };

struct end_hand_format {
  char code;
  char round;
  char hand;
  char final_revealed_tiles[19]; // only from player, originally revealed tiles and originally unrevealed tiles separated by a space
  };

struct end_game_format {
  char code;
  char from;
  char to;
  char total_round[4];
  };

// =========================================
// Function Prototypes
//
void define_tile_labels ();
#ifdef Chinese
void define_tile_labels_c ();
#endif

void rotate_seats (int *p2w, long *w2p);
char *tile_labels (char *tiles);
int calculate_fan (char *open_melds, char *private_tiles, char round_wind, char hand_wind, char player_wind);
int fan2point (int fan);
char discard_one_tile (char *tiles);
char remove_tile (char c, int count, char *tiles);
int wind2num (char c);
char next_seat (char c);
int num_occurrence (char c, char *str);
void player_log (char * format, ... );
int num_str (const char *c1, const char *c2);
int num_occurrence_in_open_and_secret (char c);
int is_chow (char c1, char c2, char c3);
int num_occurrence_in_open_tiles (char c);
int count_pair (char *tiles);
int count_triplet (char *tiles);
int is_character (char c);
int is_bamboo (char c);
int is_dot (char c);
int is_dragon (char c);
int is_wind (char c);
int check_all_melds (char *hand);
void remove_spaces (char *str);
void sort_tiles (char *tiles, int count);
void msg_log (long player_id, char direction, char *msg);
void separate_colors (char *tiles, char *character, char *dot, char *bamboo, char *wind, char *dragon);
int decompose_winning_hand (char *hand0, char *melds, char *eye);
int decompose_all_melds (char *hand, char *melds);
int remove_chow (char *from_hand, char *to_hand);
void add_pung_tiles (long pung_player, char tile);
void add_chow_tiles (long chow_player, char chow_tile, char other_tile1, char other_tile2);
void add_open_discard (char c, long discard_player);
int table_draw_tiles (int num, char *tiles_drawn);
int player_check_win (char *deck, char tile);
int player_check_pung (char *deck, char tile);
int player_check_chow (char *deck, char chow_tile, char *other_tile1, char *other_tile2);

int flush_mq(void);
void remove_mq ();
long wait_msg_from_player(char *msg_text);
int wait_msg_from_table(long player, char *msg_text);
int send_win_msg (long player);
int broadcast_win_msg (long win_player);
int send_new_hand_msg (long player, char new_round, char new_hand, char player1_seat, char player2_seat, char player3_seat, char player4_seat, char *tiles);
int send_new_game_msg (long player, char player_initial_seat);
int broadcast_end_hand_msg (char round, char hand);
int reply_end_hand_msg (long player, char round, char hand, char *tiles);
int broadcast_end_game_msg (int total_round);
int send_chow_msg (long player, char chow_tile, char other_tile1, char other_tile2);
int broadcast_chow_msg (long chow_player, char chow_tile, char other_tile1, char other_tile2);
int send_pung_msg (long player, char pung_tile);
int broadcast_pung_msg (long pung_player, char pung_tile);
int broadcast_reveal_msg (int *fund, char *reveal1, char *reveal2, char *reveal3, char *reveal4);
int broadcast_next_drawer_msg (char seat);
int send_discard_msg (long player, char tile);
int relay_discard_msg (long discard_player, char tile);
int send_draw_tile_msg (long player, char tile);
int send_register_msg (long player, int fund);
int send_pass_msg (long player);



