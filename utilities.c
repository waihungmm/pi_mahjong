#include "pi_mahjong.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>


char wind_label [4] = {'E', 'S', 'W', 'N'};
char * tile_label[127];

unsigned char tiles [MAX_TILES+1];
int tiles_count;
char open_discard[5][MAX_TILES+1]; // the rest are also
char open_meld[5][20];  // the rest as also
char my_secret_tiles[15];

long my_player;

// =========================================

int wind2num (char c)
{
switch (c) {
  case 'E': return 0;
  case 'S': return 1;
  case 'W': return 2;
  case 'N': return 3;
  }
fprintf (stderr, "Error in %s has input %c\n", __func__, c);
exit (1);
return -1; // make compiler happy
}

// =========================================

char next_seat (char c){
switch (c) {
  case 'E': return 'S';
  case 'S': return 'W';
  case 'W': return 'N';
  case 'N': return 'E';
  }
return ' ';
}

// =========================================
int num_occurrence (char c, char *str)
{
int a;
int i;
int n;
n = strlen (str);
if (n == 0)
  return 0;
a = 0;
for (i=0; i<n; i++) {
  if (c == str[i])
    a++;
  }
return a;
} // num_occurrence ()

// =========================================

int num_occurrence_in_open_tiles (char c)
{
int n = 
    num_occurrence (c, open_discard[1]) + num_occurrence (c, open_meld[1]) +
    num_occurrence (c, open_discard[2]) + num_occurrence (c, open_meld[2]) +
    num_occurrence (c, open_discard[3]) + num_occurrence (c, open_meld[3]) +
    num_occurrence (c, open_discard[4]) + num_occurrence (c, open_meld[4]);
return n;
}

// =========================================
int num_occurrence_in_open_and_secret (char c)
{
int n =
    num_occurrence (c, open_discard[1]) + num_occurrence (c, open_meld[1]) +
    num_occurrence (c, open_discard[2]) + num_occurrence (c, open_meld[2]) +
    num_occurrence (c, open_discard[3]) + num_occurrence (c, open_meld[3]) +
    num_occurrence (c, open_discard[4]) + num_occurrence (c, open_meld[4]) +
    num_occurrence (c, my_secret_tiles);
return n;
}

// =========================================

char previous_seat (char c){
switch (c) {
  case 'E': return 'N';
  case 'S': return 'E';
  case 'W': return 'S';
  case 'N': return 'W';
  }
return ' ';
}

// =========================================

void player_log (char * format, ... )
{
char TimeString[128];
struct timeval curTime;

static FILE *fp;
char filename[100];
if (fp == NULL) {
  if (my_player == 0)
    sprintf (filename, "table.log");
  else
    sprintf (filename, "player%1ld.log", my_player);

//  sprintf (filename, "pi_mahjong.log");
  fp = fopen (filename, "w");
  }

va_list args;
va_start (args, format);

int from_msg_log = 0;
if (strlen (format) >= 8) {
  if (memcmp (format, "MESSAGE-", 8) == 0)
    from_msg_log = 1;
  }

if ((strategy_num == 4) && (from_msg_log == 0))
// human
  vprintf (format, args);
else {
  if (gettimeofday(&curTime, NULL) != 0) {
    fprintf (stderr, "Error in %s, gettimeofday() != 0\n", __func__);
    exit (0);
    }
  if (strftime(TimeString, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec)) == 0) {
    fprintf (stderr, "Error in %s, strftime() == 0\n", __func__);
    exit (0);
    }
  fprintf (fp, "%s.%03d ", TimeString, curTime.tv_usec/1000);
  vfprintf (fp, format, args);
  }
fflush (fp);
va_end (args);
}

// =========================================

void msg_log (long player_id, char direction, char *msg)
{
char *msg_label;
switch (msg[0]) {
  case 'R' : msg_label = "Register"; break;
  case 'N' : msg_label = "New Game"; break;
  case 'H' : msg_label = "New Hand"; break;
  case 'D' : msg_label = "Draw Tile"; break;
  case 'C' : msg_label = "Discard Tile"; break;
  case 'P' : msg_label = "Pass"; break;
  case 'S' : msg_label = "Chow"; break;
  case 'T' : msg_label = "Pung"; break;
  case 'W' : msg_label = "Win"; break;
  case 'X' : msg_label = "Next Drawer"; break;
  case 'E' : msg_label = "End Hand"; break;
  case 'G' : msg_label = "End Game"; break;
  case 'V' : msg_label = "Reveal Tiles"; break;
  default  : msg_label = "UNRECOGNIZED";
  }

if (direction == 'S')
  player_log ("MESSAGE-SENT (%ld) [%s] \"%s\"\n", player_id, msg_label, msg);
else
  player_log ("MESSAGE-RECD (%ld) [%s] \"%s\"\n", player_id, msg_label, msg);

return;
} // msg_log ()

// =========================================

void define_tile_labels ()
{
tile_label[' ']       = ".";  // spacer
tile_label[ONE_DOT]   = "1D";
tile_label[TWO_DOT]   = "2D";
tile_label[THREE_DOT] = "3D";
tile_label[FOUR_DOT]  = "4D";
tile_label[FIVE_DOT]  = "5D";
tile_label[SIX_DOT]   = "6D";
tile_label[SEVEN_DOT] = "7D";
tile_label[EIGHT_DOT] = "8D";
tile_label[NINE_DOT]  = "9D";

tile_label[ONE_BAMBOO]   = "1B";
tile_label[TWO_BAMBOO]   = "2B";
tile_label[THREE_BAMBOO] = "3B";
tile_label[FOUR_BAMBOO]  = "4B";
tile_label[FIVE_BAMBOO]  = "5B";
tile_label[SIX_BAMBOO]   = "6B";
tile_label[SEVEN_BAMBOO] = "7B";
tile_label[EIGHT_BAMBOO] = "8B";
tile_label[NINE_BAMBOO]  = "9B";


tile_label[ONE_CHARACTER]   = "1C";
tile_label[TWO_CHARACTER]   = "2C";
tile_label[THREE_CHARACTER] = "3C";
tile_label[FOUR_CHARACTER]  = "4C";
tile_label[FIVE_CHARACTER]  = "5C";
tile_label[SIX_CHARACTER]   = "6C";
tile_label[SEVEN_CHARACTER] = "7C";
tile_label[EIGHT_CHARACTER] = "8C";
tile_label[NINE_CHARACTER]  = "9C";

tile_label[EAST_WIND]  = "EW";
tile_label[SOUTH_WIND] = "SW";
tile_label[WEST_WIND]  = "WW";
tile_label[NORTH_WIND] = "NW";

tile_label[RED_DRAGON]  = "RS"; // Special
tile_label[GREEN_DRAGON] = "GS";
tile_label[WHITE_DRAGON]  = "WS";
} // define_tile_labels ()

char *tile_labels (char *tiles)
{
static char return_labels [9 * 144];
// the static issue needs to be addressed because it causes strange behaviour on multiple invocation
int n = strlen (tiles);
int i;
return_labels[0] = '\0';
for (i=0; i<n; i++) {
#ifdef Chinese
  strcat (return_labels, tile_label_c[tiles[i]]);
#else
  strcat (return_labels, tile_label[tiles[i]]);
#endif
  if (i != n-1)
    strcat (return_labels, " ");
  }
return return_labels;
} // tile_labels ()

// =========================================
int table_draw_tiles (int num, char *tiles_drawn)
{
if (num > tiles_count) {
  fprintf (stderr, "Error in %s, num (%d) > tiles_count (%d)\n", __func__, num, tiles_count);
  exit (1);
  }
memcpy (tiles_drawn, tiles + tiles_count - num, num);
tiles_count -= num;
tiles[tiles_count] = '\0';
#ifdef DEBUG
player_log ("tiles_count is now %d\n", tiles_count);
#endif
return 1;
} // table_draw_tiles ()

// =========================================
int tile_cmp (const void * a, const void * b) {
   return ( *(char*)a - *(char*)b );
} // tile_cmp()

// =========================================

void sort_tiles (char *tiles, int count)
{
qsort(tiles, count, sizeof(char), tile_cmp);
} // sort_tiles()

// =========================================

void remove_spaces (char *str)
{
/*
printf ("before is \"%s\"\n", str);
char s2[255];
int n = strlen(str);
if (n > 254) {
  fprintf (stderr, "Error: str (%s) is too long\n");
  exit (1);
  }
int i, j;
j = 0;
for (i=0; i<n; i++) {
  if (str[i] == ' ')
    continue;
  s2[j] = str[i];
  j++;
  };
s2[j] = '\0';
strcpy (str, s2);
printf ("after is \"%s\"\n", str);
return;
*/
int i = 0, j = 0;
while (str[i]) {
  if (str[i] != ' ')
    str[j++] = str[i];
  i++;
  }
str[j] = '\0';
}

// =========================================

int is_wind (char c)
{
return ((c >= EAST_WIND) && (c <= NORTH_WIND));
}

// =========================================

int is_dragon (char c)
{
return ((c >= RED_DRAGON) && (c <= WHITE_DRAGON));
}

// =========================================

int is_dot (char c)
{
return ((c >= ONE_DOT) && (c <= NINE_DOT));
}

// =========================================

int is_bamboo (char c)
{
return ((c >= ONE_BAMBOO) && (c <= NINE_BAMBOO));
}

// =========================================

int is_character (char c)
{
return ((c >= ONE_CHARACTER) && (c <= NINE_CHARACTER));
}

// =========================================

int count_character (char *tiles)
{
int n = strlen (tiles);
if (n == 0)
  return 0;
int i;
int count = 0;
for (i=0; i<n; i++) {
  if (is_character (tiles[i]))
    count++;
  }
return count;
} // count_character()

// =========================================

int count_dot (char *tiles)
{
int n = strlen (tiles);
if (n == 0)
  return 0;
int i;
int count = 0;
for (i=0; i<n; i++) {
  if (is_dot (tiles[i]))
    count++;
  }
return count;
} // count_dot()

// =========================================

int count_bamboo (char *tiles)
{
int n = strlen (tiles);
if (n == 0)
  return 0;
int i;
int count = 0;
for (i=0; i<n; i++) {
  if (is_bamboo (tiles[i]))
    count++;
  }
return count;
} // count_bamboo()

// =========================================

int count_triplet (char *tiles)
{ // assume tiles are already sorted
int n = strlen (tiles);
if (n < 3)
  return 0;
int i;
int count = 0;
for (i=0; i<=n-3; i++) {
  if ((tiles[i] == tiles[i+1]) && (tiles[i] == tiles[i+2])) {
    count++;
    i += 2;
    }
  } // for 
return count;
} // count_triplet()

// =========================================

int count_pair (char *tiles)
{ // assume tiles are already sorted
int n = strlen (tiles);
if (n < 2)
  return 0;
int i;
int count = 0;
for (i=0; i<=n-2; i++) {
  if ((tiles[i] == tiles[i+1]) && (tiles[i] == tiles[i+2])) {
    // ignore triplet
    // since string is terminated by zero, the last test will not cause buffer overflow
    i += 2;
    }
  else if (tiles[i] == tiles[i+1]) {
    count++;
    i += 1;
    }
  } // for 
return count;
} // count_pair()

// =========================================

int is_chow (char c1, char c2, char c3)
{
int max = -1;
int min = 999;
int other;
if (c1 > max) max = c1;
if (c2 > max) max = c2;
if (c3 > max) max = c3;
if (c1 < min) min = c1;
if (c2 < min) min = c2;
if (c3 < min) min = c3;
if ((max - min) != 2)
  return 0;
int max_min = max + min;
if ((c1 + c1 == max_min) ||
    (c2 + c2 == max_min) ||
    (c3 + c3 == max_min))
  return 1;
else
  return 0;
}

// =========================================

void rotate_seats (int *p2w, long *w2p)
{
int i;
char c;
player_log ("before rotate_seat player2wind = %d%d%d%d wind2player = %ld%ld%ld%ld\n", p2w[1], p2w[2], p2w[3], p2w[4], w2p[0], w2p[1], w2p[2], w2p[3]);
for (i=1; i<=4; i++)
  p2w[i] = (p2w[i] + 3) % 4; // +3 == -1 
c = w2p[0];
w2p[0] = w2p[1];
w2p[1] = w2p[2];
w2p[2] = w2p[3];
w2p[3] = c;
player_log ("after rotate_seat  player2wind = %d%d%d%d wind2player = %ld%ld%ld%ld\n", p2w[1], p2w[2], p2w[3], p2w[4], w2p[0], w2p[1], w2p[2], w2p[3]);
} // rotate_seats ()

// =========================================

int num_str (const char *c1, const char *c2)
{ // find the number of occurrence of c1 in c2
const char *s2;
s2 = c2;
int n1 = strlen (c1);
int n2 = strlen (c2);
if (c1 > c2)
  return 0;
int c = 0;
while (s2 < c2+n2-n1) {
  if (strstr (s2, c1) != NULL)
    c++;
  s2 += n1;
  }
return c;
}

// =========================================

char wind2tile (char wind)
{
switch (wind) {
  case 'E': return EAST_WIND;
  case 'S': return SOUTH_WIND;
  case 'W': return WEST_WIND;
  case 'N': return NORTH_WIND;
  default: fprintf (stderr, "Error in %s, wind (%c) is not ESWN\n", __func__, wind);
           exit (1);
  }
return 0; // to make compiler happy
} // wind2tile()

int calculate_fan (char *open_melds, char *private_tiles, char round_wind, char hand_wind, char player_wind)
{
int fan = 0;
int n;
char tiles[20];
char melds[200] = {'\0'};
char eyes[30] = {'\0'};
char t[1000];
strcpy (t, tile_labels(open_melds));
player_log ("%s starts with open_melds=[%s] private_tiles=[%s] round_wind=%c hand_wind=%c player_wind=%c\n", __func__, t, tile_labels(private_tiles), round_wind, hand_wind, player_wind);
sprintf (tiles, "%s%s", open_melds, private_tiles);
n = strlen (tiles);
if (n != 14) {
  fprintf (stderr, "Error in %s, strlen (%d) != 14\n", __func__, n);
  exit (1);
  }
if (decompose_winning_hand (tiles, melds, eyes) != 1) {
  fprintf (stderr, "Error in %s, check_winning_hand() != 1\n", __func__);
  exit (1);
  }
// printf ("after decompose_winning_hand, melds=[%s] eyes=[%s]\n", melds, eyes);
// check all triplets
if ((melds[0] == melds[1]) && (melds[0] == melds[2]) &&
    (melds[3] == melds[4]) && (melds[3] == melds[5]) &&
    (melds[6] == melds[7]) && (melds[6] == melds[8]) &&
    (melds[9] == melds[10]) && (melds[9] == melds[11])) {
  player_log ("Add 3 Fan because of All Triplets\n");
  fan += 3;
  }
if (strchr (melds, RED_DRAGON) != NULL) {
  player_log ("Add 1 Fan because of RED_DRAGON\n");
  fan += 1;
  }
if (strchr (melds, GREEN_DRAGON) != NULL) {
  player_log ("Add 1 Fan because of GREEN_DRAGON\n");
  fan += 1;
  }
if (strchr (melds, WHITE_DRAGON) != NULL) {
  player_log ("Add 1 Fan because of WHITE_DRAGON\n");
  fan += 1;
  }
// check Win from Wall
if (strlen (open_melds) == 0) {
  player_log ("Add 1 Fan because of Win from Wall\n");
  fan += 1;
  }

char dot[15];
char bamboo[15];
char character[15];
char wind[15];
char dragon[15];
separate_colors (tiles, character, dot, bamboo, wind, dragon);
// check pure one suit
if ((strlen (character) == 14) || 
    (strlen (dot) == 14) || 
    (strlen (bamboo) == 14)) {
  player_log ("Add 7 Fan because of Pure One Suit\n");
  fan += 7;
  }
// check mixed one suit
else if (((strlen (character) != 0) && (strlen (dot) == 0) && (strlen (bamboo) == 0)) ||
         ((strlen (character) == 0) && (strlen (dot) != 0) && (strlen (bamboo) == 0)) ||
         ((strlen (character) == 0) && (strlen (dot) == 0) && (strlen (bamboo) != 0))) {
  player_log ("Add 3 Fan because of Mixed One Suit\n");
  fan += 3;
  }
// check round_wind bonus
if (strchr (melds, wind2tile(round_wind)) != NULL) {
  player_log ("Add 1 Fan because of Round Wind\n");
  fan += 1;
  }
// check hand_wind bonus
if (player_wind == hand_wind) {
  if (strchr (melds, wind2tile(player_wind)) != NULL) {
    player_log ("Add 1 Fan because of Hand Wind\n");
    fan += 1;
    }
  }
return fan;
} // calculate_fan()

// =========================================

#ifdef Chinese
void chinese_display (char *tiles)
{
int i;
int n = strlen (tiles);
char buf[100];
char line[1000];
line[0] = '\0';
for (i=0; i<n; i++) {
  sprintf (buf, "(%d)%s ", i+1, tile_label_c[tiles[i]]);
  strcat (line, buf);
  }
player_log ("%s\n", line);
}
#endif

// =========================================

void add_open_discard (char c, long discard_player)
{
int i;
i = strlen (open_discard[discard_player]);
if ((i+1) >= MAX_TILES) {
  fprintf (stderr, "Error in %s.. buffer overruns\n", __func__);
  exit (1);
  }
#ifdef DEBUG
player_log ("inside %s discard tile %s by player %ld\n", __func__, tile_label[c], discard_player);
#endif
open_discard[discard_player][i] = c;
open_discard[discard_player][i+1] = '\0';
sort_tiles (open_discard[discard_player], i+1);
char t[1000];
player_log ("Now open meld and open discard pool are...\n");
strcpy (t, tile_labels(open_meld[1]));
player_log ("Player 1 : %s | %s\n", t, tile_labels(open_discard[1]));
strcpy (t, tile_labels(open_meld[2]));
player_log ("Player 2 : %s | %s\n", t, tile_labels(open_discard[2]));
strcpy (t, tile_labels(open_meld[3]));
player_log ("Player 3 : %s | %s\n", t, tile_labels(open_discard[3]));
strcpy (t, tile_labels(open_meld[4]));
player_log ("Player 4 : %s | %s\n", t, tile_labels(open_discard[4]));
/*
player_log ("Player 2 (%d tiles): %s\n", strlen(open_discard[2]), tile_labels(open_discard[2]));
player_log ("Player 3 (%d tiles): %s\n", strlen(open_discard[3]), tile_labels(open_discard[3]));
player_log ("Player 4 (%d tiles): %s\n", strlen(open_discard[4]), tile_labels(open_discard[4]));
*/
} // add_open_discard ()

// =========================================

void add_pung_tiles (long pung_player, char tile)
{
char tmp[4];
sprintf (tmp, "%c%c%c", tile, tile, tile);
strcat (open_meld[pung_player], tmp);
player_log ("Now open melds are...\n");
player_log ("Player 1 : %s\n", tile_labels(open_meld[1]));
player_log ("Player 2 : %s\n", tile_labels(open_meld[2]));
player_log ("Player 3 : %s\n", tile_labels(open_meld[3]));
player_log ("Player 4 : %s\n", tile_labels(open_meld[4]));

} // add_pung_tiles ()

// =========================================

void add_chow_tiles (long chow_player, char chow_tile, char other_tile1, char other_tile2)
{
char tmp[4];
sprintf (tmp, "%c%c%c", chow_tile, other_tile1, other_tile2);
strcat (open_meld[chow_player], tmp);
player_log ("Now open melds are...\n");
player_log ("Player 1 : %s\n", tile_labels(open_meld[1]));
player_log ("Player 2 : %s\n", tile_labels(open_meld[2]));
player_log ("Player 3 : %s\n", tile_labels(open_meld[3]));
player_log ("Player 4 : %s\n", tile_labels(open_meld[4]));

} // add_pung_tiles ()

// =========================================

void separate_colors (char *tiles, char *character, char *dot, char *bamboo, char *wind, char *dragon)
{ // separate tiles into colors
character[0] = '\0';
dot[0]      = '\0';
bamboo[0]   = '\0';
wind[0]     = '\0';
dragon[0]   = '\0';
int dot_idx = 0;
int bamboo_idx = 0;
int character_idx = 0;
int wind_idx = 0;
int dragon_idx = 0;
int i;
int n1 = strlen (tiles);
char c;
for (i=0; i<n1; i++) {
  c = tiles[i];
  if (is_dot(c))
    dot[dot_idx++] = c;
  else if (is_bamboo(c))
    bamboo[bamboo_idx++] = c;
  else if (is_character(c))
    character[character_idx++] = c;
  else if (is_wind(c))
    wind[wind_idx++] = c;
  else if (is_dragon(c))
    dragon[dragon_idx++] = c;
  else {
    fprintf (stderr, "Error in separation of colors\n");
    exit (1);
    }
  } // for
dot[dot_idx]             = '\0';
bamboo[bamboo_idx]       = '\0';
character[character_idx] = '\0';
wind[wind_idx]           = '\0';
dragon[dragon_idx]       = '\0';
} // separate_colors()

// =========================================

int decompose_all_melds (char *hand, char *melds)
{
// printf ("%s starts hand=\"%s\", melds=\"%s\"\n", __func__, hand, melds);
int tile_count;
tile_count = strlen (hand);
if (tile_count == 0)
  return 1;
if ((tile_count % 3) != 0) {
  printf ("Error in %s, hand [%s] count (%d) is not multiple of 3\n", __func__, hand, tile_count);
  exit (1);
  }
char hand1[15];
char a[4];
if ((hand[0] == hand[1]) && (hand[0] == hand[2])) {
  a[0] = hand[0];
  a[1] = hand[0];
  a[2] = hand[0];
  a[3] = '\0';
  if (tile_count == 3) {
    strcat (melds, a);
// printf ("after strcat, melds=[%s]\n", melds);
    return 1;
    }
  strcpy (hand1, hand+3);
  if (decompose_all_melds (hand1, melds) == 1) {
// printf ("before strcat, melds=[%s]\n", melds);
    strcat (melds, a);
// printf ("after strcat, melds=[%s]\n", melds);
    return 1;
    }
  else
    return 0;
  }
else {
  if (remove_chow (hand, hand1) == 1) {
    a[0] = hand[0];
    a[1] = hand[0]+1;
    a[2] = hand[0]+2;
    a[3] = '\0';
    // printf ("remove_chow returns 1 and a=[%s]\n", a);
    if (hand1[0] == '\0') {
      // printf ("going to strcat\n");
      strcat (melds, a);
      // printf ("after strcat\n");
      return 1;
      }
    else {
      int r = decompose_all_melds (hand1, melds);
      if (r == 1) {
        strcat (melds, a);
        return 1;
        }
      else
        return 0;
      }
    }
  else
    return 0;
  }
} // decompose_all_melds ()

// =========================================

int decompose_winning_hand (char *hand0, char *melds, char *eye)
{
int tile_count = strlen (hand0);
if (tile_count != 14) {
  printf ("Error : tile_count != 14\n");
  exit (0);
  }
char hand[15];
strcpy (hand, hand0);
sort_tiles (hand, tile_count);
int i;
char new_hand[15];

// printf ("%s starts with hand=[%s], melds=[%s] eye=[%s]\n", __func__, hand0, melds, eye);
for (i=0; i<tile_count-1; i++) {
  // find eye
  if (hand[i] == hand[i+1]) {
    strcpy (new_hand, hand);
    new_hand[i] = ' ';
    new_hand[i+1] = ' ';
    remove_spaces (new_hand);
    if (decompose_all_melds (new_hand, melds) == 1) {
      // printf ("winning hand has eye %c%c and others %s\n", hand[i], hand[i+1], melds);
      sprintf (eye, "%c%c", hand[i], hand[i+1]);
      return 1;
      }
    } // hand[i] == hand[i+1]
  } // for
return 0;
} // decompose_winning_hand ()

int remove_chow (char *from_hand, char *to_hand)
// the candidate chow is c, c+1, c+2, where c is the first tile in from_hand
// return 1 if successful
{
strcpy (to_hand, from_hand);
char c = from_hand[0];

// only applicabe to Wind or Dragon tiles
if (is_wind (c) || is_dragon(c))
  return 0;

char *ptr;
to_hand[0] = ' '; 
ptr = strchr (to_hand, c+1);
if (ptr != NULL)
  *ptr = ' ';
else
  return 0;
ptr = strchr (to_hand, c+2);
if (ptr != NULL)
  *ptr = ' ';
else
  return 0;
remove_spaces (to_hand);
return 1;
}

int check_all_melds (char *hand)
{ // meld = triple (pung) or sequence (chow)
int tile_count;
tile_count = strlen (hand);
if (tile_count == 0)
  return 1;
if ((tile_count % 3) != 0) {
  printf ("Error in %s, hand [%s] count (%d) is not multiple of 3\n", __func__, hand, tile_count);
  exit (1);
  }
char hand1[15];
if ((hand[0] == hand[1]) && (hand[0] == hand[2])) {
  if (tile_count == 3)
    return 1;
  strcpy (hand1, hand+3);
  if (check_all_melds (hand1) == 1)
    return 1;
  else
    return 0;
  }
else {
  if (remove_chow (hand, hand1) == 1) {
    if (hand1[0] == '\0')
      return 1;
    else
      return check_all_melds (hand1);
    }
  else
    return 0;
  }
} // check_all_melds ()

int fan2point (int fan)
{
if (fan ==  0) return 1;
if (fan ==  1) return 2;
if (fan ==  2) return 4;
if (fan ==  3) return 8;
if (fan ==  4) return 16;
if (fan ==  5) return 24;
if (fan ==  6) return 32;
if (fan ==  7) return 48;
if (fan ==  8) return 64;
if (fan >  8) return 64;
fprintf (stderr, "Error in %s, fan == %d\n", __func__, fan);
exit (1);
} // fan2point()