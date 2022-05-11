#include "pi_mahjong.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

char mybin_type[20];
char mybin[5][20];
int try_all_triplet;
int try_one_suit;

int strategy_num;
/*
0 : random
1 : http://mahjongjoy.com/mahjong_bots_ai_algorithm.shtml
    Bot Algorithm for Mahjong Hong Kong MP0
2 : dito
    Bot Algorithm for Mahjong Hong Kong 1 < MP <= 8 (1, 2, 3 Fan) 
3 : bin
4 : human
*/

// =========================================

int check_winning_hand (char *hand0)
/* The logic is based on
https://sites.google.com/a/g2.nctu.edu.tw/unimath/2018-07/mj2
*/
{
// hand should be null terminated
int tile_count = strlen (hand0);
if ((tile_count % 3) != 2) {
  printf ("Error: tile number (%d) is not 2, 5, 8, 11, 14\n", tile_count);
  exit (0);
  }
if (tile_count > 14) {
  printf ("Error : tile_count is greater than 14\n");
  exit (0);
  }
char hand[15];
strcpy (hand, hand0);
sort_tiles (hand, tile_count);
int i;
char new_hand[15];
for (i=0; i<tile_count-1; i++) {
  // find eye
  if (hand[i] == hand[i+1]) {
    strcpy (new_hand, hand);
    new_hand[i] = ' ';
    new_hand[i+1] = ' ';
    remove_spaces (new_hand);
    if (check_all_melds (new_hand) == 1) {
      player_log ("winning hand has eye %s %s and others %s\n", tile_label[hand[i]], tile_label[hand[i+1]], tile_labels(new_hand));
      return 1;
      }
    } // hand[i] == hand[i+1]
  } // for
return 0;
} // check_winning_hand ()

// =========================================

void mask_color (char *tiles, char start_tile, char end_tile)
{
int n = strlen (tiles);
if (n == 0)
  return;
int i;
char c;
for (i=0; i<n; i++) {
  c = tiles[i];
  if ((c >= start_tile) && (c <= end_tile))
    tiles[i] = ' ';
  } // for
} // mask_color ()

// =========================================

void mask_triplet (char *tiles, char start_tile, char end_tile)
{
char c;
char *p;
int n;
int i;
for (c=start_tile; c<=end_tile; c++) {
  n = num_occurrence (c, tiles);
  if (n >= 3) {
    player_log ("masking %s from discarding\n", tile_label[c]);
    for (i=1; i<=n; i++) {
      p = strchr (tiles, c);
      if (p == NULL) {
        fprintf (stderr, "Error in %s p == NULL\n", __func__);
        exit (1);
        }
      *p = ' ';  // space to prevent subsequent selection
      } // for i
    }
  } // for c
} //  mask_triplet ()

// =========================================

void mask_twins (char *tiles, char start_tile, char end_tile)
{
char c;
char *p;
int n;
int i;
for (c=start_tile; c<=end_tile; c++) {
  n = num_occurrence (c, tiles);
  if (n >= 2) {
    player_log ("masking %s from discarding\n", tile_label[c]);
    for (i=1; i<=n; i++) {
      p = strchr (tiles, c);
      if (p == NULL) {
        fprintf (stderr, "Error in %s p == NULL\n", __func__);
        exit (1);
        }
      *p = ' ';  // space to prevent subsequent selection
      } // for i
    }
  } // for c
} //  mask_twins ()

// =========================================

void mask_consecutives (char *tiles)
{
int n = strlen (tiles);
if (n <= 1)
  return;
int i;
char c;
for (i=0; i<(n-1); i++) {
  c = tiles[i];
  if (c+1 == tiles [i+1]) {
    player_log ("masking consecutives \"%s %s\" from discarding\n", tile_label[tiles[i]], tile_label[tiles[i+1]]);
    tiles[i] = ' ';
    tiles[i+1] = ' ';
    if ((i+2 < n) && (c+2 == tiles[i+2])) {
      player_log ("also masking \"%s\"\n", tile_label[tiles[i+2]]);
      tiles[i+2] = ' ';
      i += 2;
      }
    else
      i += 1;
    }
  } // for i
} // mask_consecutives ()

// =========================================
char discard_strategy0 (char *tiles)
{ // purely random as a control
int n = strlen (tiles);
if (n == 0) {
  fprintf (stderr, "Error: n == 0 in %s\n", __func__);
  exit (1);
  }
int i = random() % n;
return tiles[i];
} // discard_strategy0 ()



// =========================================
char discard_strategy1 (char *tiles0)
{ // implement the actual strategy based on 
// http://mahjongjoy.com/mahjong_bots_ai_algorithm.shtml
// separate into colors
char dot[15];
char bamboo[15];
char character[15];
char wind[15];
char dragon[15];
int i;
char tiles[15];
strcpy (tiles, tiles0);
int n1 = strlen (tiles);
char c;

separate_colors (tiles, character, dot, bamboo, wind, dragon);

player_log ("after separation of colors, character = %s\n", tile_labels(character));
player_log ("after separation of colors, bamboo = %s\n", tile_labels(bamboo));
player_log ("after separation of colors, dot = %s\n", tile_labels(dot));
player_log ("after separation of colors, wind = %s\n", tile_labels(wind));
player_log ("after separation of colors, dragon = %s\n", tile_labels(dragon));
if (strlen (dot) == 1) {
  player_log ("returning the only instance dot\n", tile_label[dot[0]]);
  return dot[0];
  }
if (strlen (character) == 1) {
  player_log ("returning the only instance character\n", tile_label[character[0]]);
  return character[0];
  }
if (strlen (bamboo) == 1) {
  player_log ("returning the only instance of bamboo\n", tile_label[bamboo[0]]);
  return bamboo[0];
  }
for (c=EAST_WIND; c<=NORTH_WIND; c++) {
  if ((num_occurrence (c, wind) == 1) && 
      (num_occurrence_in_open_tiles (c) >= 1 )) {
    player_log ("returning singleton %s\n", tile_label[c]);
    return c;
    }
  }
for (c=RED_DRAGON; c<=WHITE_DRAGON; c++) {
  if ((num_occurrence (c, dragon) == 1) && 
      (num_occurrence_in_open_tiles (c) >= 1 )) {
    player_log ("returning singleton %s\n", tile_label[c]);
    return c;
    }
  }
if (strlen (wind) == 1) {
  player_log ("returning singleton wind\n");
  return wind[0];
  }
if (strlen (dragon) == 1) {
  player_log ("returning singleton dragon\n");
  return dragon[0];
  }
mask_twins (dot, ONE_DOT, NINE_DOT);
mask_twins (bamboo, ONE_BAMBOO, NINE_BAMBOO);
mask_twins (character, ONE_CHARACTER, NINE_CHARACTER);
mask_twins (wind, EAST_WIND, NORTH_WIND);
mask_twins (dragon, RED_DRAGON, WHITE_DRAGON);
/*
char *p;
for (c=ONE_DOT; c<=NINE_DOT; c++) {
  n = num_occurrence (c, dot);
  if (n >= 2) {
    player_log ("masking %s from discarding\n", tile_label[c]);
    for (i=1; i<=n; i++) {
      p = strchr (dot, c);
      if (p == NULL) {
        fprintf (stderr, "Error in %s p == NULL\n", __func__);
        exit (1);
        }
      *p = ' ';  // space to prevent subsequent selection
      } // for i
    }
  } // for c
for (c=ONE_BAMBOO; c<=NINE_BAMBOO; c++) {
  n = num_occurrence (c, bamboo);
  if (n >= 2) {
    player_log ("masking %s from discarding\n", tile_label[c]);
    for (i=1; i<=n; i++) {
      p = strchr (bamboo, c);
      if (p == NULL) {
        fprintf (stderr, "Error in %s p == NULL\n", __func__);
        exit (1);
        }
      *p = ' ';  // space to prevent subsequent selection
      } // for i
    }
  } // for c
for (c=ONE_CHARACTER; c<=NINE_CHARACTER; c++) {
  n = num_occurrence (c, character);
  if (n >= 2) {
    player_log ("masking %s from discarding\n", tile_label[c]);
    for (i=1; i<=n; i++) {
      p = strchr (character, c);
      if (p == NULL) {
        fprintf (stderr, "Error in %s p == NULL\n", __func__);
        exit (1);
        }
      *p = ' ';  // space to prevent subsequent selection
      } // for i
    }
  } // for c
for (c=EAST_WIND; c<=NORTH_WIND; c++) {
  n = num_occurrence (c, wind);
  if (n >= 2) {
    player_log ("masking %s from discarding\n", tile_label[c]);
    for (i=1; i<=n; i++) {
      p = strchr (wind, c);
      if (p == NULL) {
        fprintf (stderr, "Error in %s p == NULL\n", __func__);
        exit (1);
        }
      *p = ' ';  // space to prevent subsequent selection
      } // for i
    }
  } // for c
for (c=RED_DRAGON; c<=WHITE_DRAGON; c++) {
  n = num_occurrence (c, dragon);
  if (n >= 2) {
    player_log ("masking %s from discarding\n", tile_label[c]);
    for (i=1; i<=n; i++) {
      p = strchr (dragon, c);
      if (p == NULL) {
        fprintf (stderr, "Error in %s p == NULL\n", __func__);
        exit (1);
        }
      *p = ' ';  // space to prevent subsequent selection
      } // for i
    }
  } // for c
*/
remove_spaces (dot);
remove_spaces (bamboo);
remove_spaces (character);
remove_spaces (wind);
remove_spaces (dragon);

player_log ("after remove_spaces, character = %s\n", tile_labels(character));
player_log ("after remove_spaces, bamboo = %s\n", tile_labels(bamboo));
player_log ("after remove_spaces, dot = %s\n", tile_labels(dot));
player_log ("after remove_spaces, wind = %s\n", tile_labels(wind));
player_log ("after remove_spaces, dragon = %s\n", tile_labels(dragon));

mask_consecutives (dot);
mask_consecutives (bamboo);
mask_consecutives (character);

remove_spaces (dot);
remove_spaces (bamboo);
remove_spaces (character);
remove_spaces (wind);
remove_spaces (dragon);

player_log ("after remove_spaces, character = %s\n", tile_labels(character));
player_log ("after remove_spaces, bamboo = %s\n", tile_labels(bamboo));
player_log ("after remove_spaces, dot = %s\n", tile_labels(dot));
player_log ("after remove_spaces, wind = %s\n", tile_labels(wind));
player_log ("after remove_spaces, dragon = %s\n", tile_labels(dragon));
if (strlen (dot) == 1) {
  player_log ("returning singleton dot\n");
  return dot[0];
  }
if (strlen (bamboo) == 1) {
  player_log ("returning singleton bamboo\n");
  return bamboo[0];
  }
if (strlen (character) == 1) {
  player_log ("returning singleton character\n");
  return character[0];
  }
if (strlen (wind) == 1) {
  player_log ("returning singleton wind\n");
  return wind[0];
  }
if (strlen (dragon) == 1) {
  player_log ("returning singleton dragon\n");
  return dragon[0];
  }
player_log ("returning random\n");
char tiles2[15];
sprintf (tiles2, "%s%s%s%s%s", character, bamboo, dot, wind, dragon);
int n2 = strlen (tiles2);
if (n2 == 0) {
  player_log ("n2 == 0 and use random to select a tile from the original concealed tiles\n");
  i = random() % n1;
  return tiles[i];
  }
i = random() % n2;
return tiles2[i];
} // discard_strategy1 ()

// =========================================
char discard_strategy2 (char *tiles0)
{
char dot[15];
char bamboo[15];
char character[15];
char wind[15];
char dragon[15];
int i;
char tiles[15];
// strcpy (tiles, tiles0);
sprintf (tiles, "%s%s", open_meld[my_player], tiles0);

player_log ("Inside %s, tiles = [%s]\n", __func__, tile_labels(tiles));
// player_log ("open_meld[my_player] = [%s]\n", tile_labels(open_meld[my_player]));

// determine which is more likely... One suit (maxed or pure) vs All triplets
// int triplet_count = count_triplet (tiles) + count_triplet (open_meld[my_player]);
int triplet_count = count_triplet (tiles);
player_log ("triplet_count = %d\n", triplet_count);
int pair_count = count_pair (tiles);
player_log ("pair_count = %d\n", pair_count);
int triplet_score = 5 * triplet_count + 2 * pair_count;
player_log ("triplet_score = %d\n", triplet_score);

separate_colors (tiles, character, dot, bamboo, wind, dragon);
// int character_count = count_character (tiles) + count_character (open_meld[my_player]);
int character_count = strlen (character);
// int dot_count = count_dot (tiles) + count_dot (open_meld[my_player]);
int dot_count = strlen (dot);
// int bamboo_count = count_bamboo (tiles) + count_bamboo (open_meld[my_player]);
int bamboo_count = strlen (bamboo);
player_log ("character_count = %d\n", character_count);
player_log ("dot_count = %d\n", dot_count);
player_log ("bamboo_count = %d\n", bamboo_count);
int majority_color_count;
int minority_color_count;
char majority_color;
if ((character_count >= dot_count) && (character_count >= bamboo_count)) {
  majority_color = 'C';
  majority_color_count = character_count;
  minority_color_count = dot_count + bamboo_count;
  }
if ((dot_count >= character_count) && (dot_count >= bamboo_count)) {
  majority_color = 'D';
  majority_color_count = dot_count;
  minority_color_count = character_count + bamboo_count;
  }
if ((bamboo_count >= character_count) && (bamboo_count >= dot_count)) {
  majority_color = 'B';
  majority_color_count = bamboo_count;
  minority_color_count = character_count + dot_count;
  }
player_log ("majority_color_count = %d\n", majority_color_count);
player_log ("minority_color_count = %d\n", minority_color_count);
int wind_triplet_count = count_triplet (wind);
player_log ("wind_triplet_count = %d\n", wind_triplet_count);
int dragon_triplet_count = count_triplet (dragon);
player_log ("dragon_triplet_count = %d\n", dragon_triplet_count);
int wind_pair_count = count_pair (wind);
player_log ("wind_pair_count = %d\n", wind_pair_count);
int dragon_pair_count = count_pair (dragon);
player_log ("dragon_pair_count = %d\n", dragon_pair_count);
int one_suit_score = majority_color_count - minority_color_count + 3 * wind_triplet_count + 3 * dragon_triplet_count + 2 * wind_pair_count + 2 * dragon_pair_count;
player_log ("one_suit_score = %d\n", one_suit_score);

try_all_triplet = 0;
try_one_suit = 0;

if (triplet_score > one_suit_score)
  try_all_triplet = 1;
else
  try_one_suit = 1;
player_log ("try_all_triplet = %d try_one_suit = %d\n", try_all_triplet, try_one_suit);
// after determining the strategy, ignore the open melds and just consider the concealed tiles
char tiles2[15];
strcpy (tiles2, tiles0);
if (try_all_triplet) {
  mask_twins (tiles2, ONE_DOT, NINE_DOT);
  mask_twins (tiles2, ONE_BAMBOO, NINE_BAMBOO);
  mask_twins (tiles2, ONE_CHARACTER, NINE_CHARACTER);
  mask_twins (tiles2, EAST_WIND, NORTH_WIND);
  mask_twins (tiles2, RED_DRAGON, WHITE_DRAGON);
  remove_spaces (tiles2);
  if (strlen (tiles2) != 0)
    return discard_strategy1(tiles2);
  strcpy (tiles2, tiles0);
  mask_triplet (tiles2, ONE_DOT, NINE_DOT);
  mask_triplet (tiles2, ONE_BAMBOO, NINE_BAMBOO);
  mask_triplet (tiles2, ONE_CHARACTER, NINE_CHARACTER);
  mask_triplet (tiles2, EAST_WIND, NORTH_WIND);
  mask_triplet (tiles2, RED_DRAGON, WHITE_DRAGON);
  remove_spaces (tiles2);
  if (strlen (tiles2) == 0) {
    fprintf (stderr, "Error in %s, strlen (tiles2) == 0\n", __func__);
    exit (1);
    }
  return discard_strategy1(tiles2);
  }
if (try_one_suit) {
  if (minority_color_count > 0) {
    player_log ("going to discard minority color first\n");
    separate_colors (tiles2, character, dot, bamboo, wind, dragon);
    char minority_tiles[20];
    switch (majority_color) {
      case 'B': sprintf (minority_tiles, "%s%s", character, dot);
                break;
      case 'D': sprintf (minority_tiles, "%s%s", character, bamboo);
                break;
      case 'C': sprintf (minority_tiles, "%s%s", dot, bamboo);
                break;
      default : fprintf (stderr, "Error in %s, should not be here %d\n", __func__, __LINE__);
                exit (1);
      } // switch
    if (strlen (minority_tiles) != 0) {
      player_log ("minority_tiles = [%s]\n", tile_labels (minority_tiles));
      return discard_strategy0 (minority_tiles);
      }
    else
      player_log ("strlen (minority_tiles) == 0 means they are in open_melds\n");
    } // if (minority_color_count > 0)
    
  player_log ("masking majority color %c\n", majority_color);
  switch (majority_color) {
    case 'B': mask_color (tiles2, ONE_BAMBOO, NINE_BAMBOO);
              break;
    case 'D': mask_color (tiles2, ONE_DOT, NINE_DOT);
              break;
    case 'C': mask_color (tiles2, ONE_CHARACTER, NINE_CHARACTER);
              break;
    default : fprintf (stderr, "Error in %s, should not be here\n", __func__);
              exit (1);
    } // switch
  player_log ("masking wind tiles\n");
  mask_twins (tiles2, EAST_WIND, NORTH_WIND);
  player_log ("masking dragon tiles\n");
  mask_twins (tiles2, RED_DRAGON, WHITE_DRAGON);
  remove_spaces (tiles2);
  player_log ("now tiles becomes [%s]\n", tile_labels (tiles2));
  if (strlen (tiles2) == 0)
    return discard_strategy1(tiles0);
  return discard_strategy1(tiles2);
  }
} // discard_strategy2()

// =========================================

int player_check_win (char *deck, char tile)
{
char tmp_deck[15];
sprintf (tmp_deck, "%s%c", deck, tile);
return check_winning_hand (tmp_deck);
}

// =========================================
int player_check_chow1 (char *deck, char chow_tile, char *other_tile1, char *other_tile2)
{ 
player_log ("%s starts with deck = %s and chow_tile = %c\n", __func__, deck, chow_tile);
// player.c has already checked the wind position
// but not yet checked the tile color
if (is_wind(chow_tile) || is_dragon(chow_tile))
  return 0;
if (num_occurrence (chow_tile, deck) != 0)
  return 0;
int num;
if (is_dot (chow_tile))
  num = chow_tile - ONE_DOT + 1;
else if (is_character (chow_tile))
  num = chow_tile - ONE_CHARACTER + 1;
else if (is_bamboo (chow_tile))
  num = chow_tile - ONE_BAMBOO + 1;
else {
  fprintf (stderr, "Error in %s at line %d\n", __func__, __LINE__);
  exit (1);
  }
if (num == 1) {
  if ((num_occurrence (chow_tile+1, deck) == 1) &&
      (num_occurrence (chow_tile+2, deck) == 1)) {
    *other_tile1 = chow_tile+1;
    *other_tile2 = chow_tile+2;
    return 1;
    }
  return 0;
  } // if (num == 1)
if (num == 2) {
  if ((num_occurrence (chow_tile-1, deck) == 1) &&
      (num_occurrence (chow_tile+1, deck) == 1)) {
    *other_tile1 = chow_tile-1;
    *other_tile2 = chow_tile+1;
    return 1;
    }
  if ((num_occurrence (chow_tile+1, deck) == 1) &&
      (num_occurrence (chow_tile+2, deck) == 1)) {
    *other_tile1 = chow_tile+1;
    *other_tile2 = chow_tile+2;
    return 1;
    }
  return 0;
  } // if (num == 2)
if (num == 8) {
  if ((num_occurrence (chow_tile-1, deck) == 1) &&
      (num_occurrence (chow_tile+1, deck) == 1)) {
    *other_tile1 = chow_tile-1;
    *other_tile2 = chow_tile+1;
    return 1;
    }
  if ((num_occurrence (chow_tile-2, deck) == 1) &&
      (num_occurrence (chow_tile-1, deck) == 1)) {
    *other_tile1 = chow_tile-2;
    *other_tile2 = chow_tile-1;
    return 1;
    }
  return 0;
  } // if (num == 8)
if (num == 9) {
  if ((num_occurrence (chow_tile-2, deck) == 1) &&
      (num_occurrence (chow_tile-1, deck) == 1)) {
    *other_tile1 = chow_tile-2;
    *other_tile2 = chow_tile-1;
    return 1;
    }
  return 0;
  } // if (num == 9)
// now num should be from 3 to 7 inclusive
if ((num_occurrence (chow_tile-2, deck) == 1) &&
    (num_occurrence (chow_tile-1, deck) == 1)) {
  *other_tile1 = chow_tile-2;
  *other_tile2 = chow_tile-1;
  return 1;
  }
if ((num_occurrence (chow_tile+1, deck) == 1) &&
    (num_occurrence (chow_tile+2, deck) == 1)) {
  *other_tile1 = chow_tile+1;
  *other_tile2 = chow_tile+2;
  return 1;
  }
if ((num_occurrence (chow_tile-1, deck) == 1) &&
    (num_occurrence (chow_tile+1, deck) == 1)) {
  *other_tile1 = chow_tile-1;
  *other_tile2 = chow_tile+1;
  return 1;
  }
return 0;
} // player_check_chow1()

// =========================================
int player_check_chow2 (char *deck, char chow_tile, char *other_tile1, char *other_tile2)
{
if (try_all_triplet)
  return 0;

return player_check_chow1 (deck, chow_tile, other_tile1, other_tile2);
} // player_check_chow2()

// =========================================
int player_check_chow3 (char *deck, char chow_tile, char *other_tile1, char *other_tile2)
{
if (mybin_type[0] == '\0')
  return 0;
char tmp[20];
strcpy (tmp, mybin_type);
char *p;
while (p = strstr (tmp, "C2")) {
  p[0] = ' '; p[1] = ' ';
  int bin_num = (p - tmp) / 2;
  if (strlen (mybin[bin_num]) != 2) {
    fprintf (stderr, "Error in %s bin_num = %d mybin[bin_num] = %s strlen !=2\n", __func__, bin_num, mybin[bin_num]);
    exit (1);
    }
  if (is_chow (mybin[bin_num][0], mybin[bin_num][1], chow_tile)) {
    *other_tile1 = mybin[bin_num][0];
    *other_tile2 = mybin[bin_num][1];
    return 1;
    }
  } // while
return 0;
} // player_check_chow3()

// =========================================

int player_check_chow (char *deck, char chow_tile, char *other_tile1, char *other_tile2)
{
switch (strategy_num) {
  case 1:
    return player_check_chow1 (deck, chow_tile, other_tile1, other_tile2);
  case 2:
    return player_check_chow2 (deck, chow_tile, other_tile1, other_tile2);
  case 3:
    return player_check_chow3 (deck, chow_tile, other_tile1, other_tile2);
  default:
    return 0;
  }
return 0;
}

// =========================================

int player_check_pung (char *deck, char tile)
{
int n;
n = num_occurrence (tile, deck);
return (n == 2);
}

// =========================================
// http://userpage.fu-berlin.de/~ram/pub/pub_jf47ht81Ht/c_preprocessor_applications_en

/*
C3 = chow
C2 = proposed chow
P3 = pong
P2 = proposed pong or eye
*/

#define BIN_TYPES \
C(P3P3P3P2P2) \
C(P3P3P2P2C3)  \
C(P3P3P2C3C2)  \
C(P3P2C3C3C2)  \
C(P3P3P2P2P2)   \
C(P3P3P2P2C2)  \
C(P3P3P2C2C2)  \
C(P3P2P2C3C2)  \
C(P3P2C3C2C2)  \
C(P2P2C3C3C2)  \
C(P3P2P2P2P2)   \
C(P2P2P2P2C3)  \
C(P3P2P2C2C2)  \
C(P3P2C2C2C2)  \
C(P2P2C3C2C2)  \
C(P2P2P2P2P2)   \
C(P2P2P2P2C2)  \
C(P2P2P2C3C2)  \
C(P2P2P2C2C2)  \
C(P2P2C2C2C2)  \
C(P3P3P3P3)   \
C(P3P3P3C3)  \
C(P3P3C3C3)  \
C(P3C3C3C3)  \
C(C3C3C3C3)  \
C(P3P3P2C3)   \
C(P3P3P3C2)  \
C(P3P3C3C2)  \
C(P3P2C3C3)  \
C(P3C3C3C2)  \
C(P2C3C3C3)  \
C(P3P3P2C2)   \
C(P3P3C2C2)  \
C(P3P2C3C2)  \
C(P2P2C3C3)  \
C(P3C3C2C2)  \
C(P2C3C3C2)  \
C(P3P2P2C2)   \
C(P2P2P2C3)  \
C(P3P2C2C2)  \
C(P2P2C3C2)  \
C(P3C2C2C2)  \
C(P2C3C2C2)  \
C(P2P2P2P2)   \
C(P2P2P2C2)  \
C(P2P2C2C2)  \
C(P2C2C2C2)  \
C(C2C2C2C2)  \
C(P3P3P3)  \
C(P3P3C3)  \
C(P3C3C3)  \
C(C3C3C3)  \
C(P3P3P2)   \
C(P3P3C2)  \
C(P3C3C2)  \
C(C3C3C2)  \
C(P3P2P2)   \
C(P3P2C2)  \
C(P2P2C3)  \
C(P3C2C2)  \
C(P2C3C2)  \
C(C3C2C2)  \
C(P2P2P2)   \
C(P2P2C2)  \
C(P2C2C2)  \
C(C2C2C2)  \
C(P3P3)    \
C(P3C3)  \
C(C3C3)  \
C(P3P2)  \
C(P3C2)  \
C(P2C3)  \
C(P2P2)  \
C(P2C2)  \
C(C2C2)  \
C(C3C2)  \
C(P3)   \
C(C3)  \
C(P2)   \
C(C2)

#define C(x) x,
enum bin_type { BIN_TYPES BIN_COUNT };
#undef C
#define C(x) #x,
const char * const bin_name[] = { BIN_TYPES };

// =========================================

int has_pchow (char *hand, char *bin)
// proposed chow is (c, c+1) or (c, c+2)
// return 1 if successful
{
int n = strlen (hand);
if (n < 2) {
  player_log ("Error in %s : n (%d) < 2\n", __func__, n);
  exit (1);
  }
int i;
char c;
char *ptr1;
#ifdef DEBUG
player_log ("%s hand=[%s] starts\n", __func__, hand);
#endif
*bin = '\0'; // initialize first
for (i=0; i<=(n-2); i++) {
  c = hand[i];
  if (is_wind(c) || is_dragon(c))
    continue;
  ptr1 = strchr (hand+1, c+1);
  if (ptr1 != NULL) {
    int j1 = 0;
    int j2 = 0;
    int j3 = 0;
    if (((c+1==NINE_CHARACTER) || (c+1==NINE_DOT) || (c+1==NINE_BAMBOO)) &&
        (num_occurrence_in_open_and_secret (c-1) > 3))
      j1 = 1;
    if (((c==ONE_CHARACTER) || (c==ONE_DOT) || (c==ONE_BAMBOO)) &&
        (num_occurrence_in_open_and_secret (c+2) > 3))
      j2 = 1;
    if ((c!=ONE_CHARACTER) && (c!=ONE_DOT) && (c!=ONE_BAMBOO) &&
        (c+1!=NINE_CHARACTER) && (c+1!=NINE_DOT) && (c+1!=NINE_BAMBOO) &&
        (num_occurrence_in_open_and_secret (c-1) > 3) &&
        (num_occurrence_in_open_and_secret (c+2) > 3))
      j3 = 1;
    if (j1 || j2 || j3)
      player_log ("%s rejects \"%s %s\" because j1=%d j2=%d j3=%d\n", __func__, tile_label[c], tile_label[c+1], j1, j2, j3);
    else {
      hand[i] = ' ';
      *ptr1 = ' ';
      remove_spaces (hand);
      sprintf (bin, "%c%c", c, c+1);
#ifdef DEBUG
      player_log ("%s success returning hand=[%s] bin=[%s]\n", __func__, hand, bin);
#endif
      return 1;
      }
    }
  ptr1 = strchr (hand+1, c+2);
  if (ptr1 != NULL) {
    if (num_occurrence_in_open_tiles (c+1) > 3)
      player_log ("%s rejects \"%s %s\" because no. of occurrences of %s in open tiles > 3\n", __func__, tile_label[c], tile_label[c+2], tile_label[c+1]);
    else {
      hand[i] = ' ';
      *ptr1 = ' ';
      remove_spaces (hand);
      sprintf (bin, "%c%c", c, c+2);
#ifdef DEBUG
      player_log ("%s success returning hand=[%s] bin=[%s]\n", __func__, hand, bin);
#endif
      return 1;
      }
    }
  } // for
return 0;
} // has_pchow ()

// =========================================

int has_chow (char *hand, char *bin)
// the candidate chow is c, c+1, c+2
// return 1 if successful
// not yet check for dragon tiles
{
int n = strlen (hand);
if (n < 3) {
  player_log ("Error in %s n <= 3\n", __func__);
  return 0;
  // exit (1);
  }
int i;
char c;
char *ptr1;
char *ptr2;
*bin = '\0'; // initialize first
#ifdef DEBUG
player_log ("%s (%s) starts\n", __func__, hand);
#endif
for (i=0; i<=(n-3); i++) {
  c = hand[i];
  if (is_wind(c) || is_dragon(c))
    continue;
  ptr1 = strchr (hand+1, c+1);
  if (ptr1 == NULL)
    continue;
  ptr2 = strchr (ptr1+1, c+2);
  if (ptr2 == NULL)
    continue;
  hand[i] = ' ';
  *ptr1 = ' ';
  *ptr2 = ' ';
  remove_spaces (hand);
  sprintf (bin, "%c%c%c", c, c+1, c+2);
#ifdef DEBUG
player_log ("%s success returning hand=[%s] bin=[%s]\n", __func__, hand, bin);
#endif
  return 1;
  } // for
return 0;
} // has_chow()
// =========================================
char remove_tile (char c, int count, char *tiles)
{ // return 0 if failed
#ifdef DEBUG
player_log ("%s (%c, %d, %s) starts\n", __func__, c, count, tiles);
#endif
int i;
char tiles2[20];
strcpy (tiles2, tiles);
int n = strlen (tiles2);
if (count > n) {
  player_log ("Error in %s count (%d) > n (%d)\n", __func__, count, n);
  exit (1);
  }
int cnt = 0;
for (i=0; i<n; i++) {
  if (c == tiles2[i]) {
    tiles2[i] = ' ';
    cnt++;
    if (cnt == count)
      break;
    }
  }
if (cnt == count) {
  remove_spaces (tiles2);
  strcpy (tiles, tiles2);
  return c;
  }
else
  return 0;
} // remove_tile ()

// =========================================

char try_remove_tile (char c, int count, char *tiles)
{ // return 0 if failed
#ifdef DEBUG
player_log ("%s (%c, %d, %s) starts\n", __func__, c, count, tiles);
#endif
int i;
char tiles2[20];
strcpy (tiles2, tiles);
int n = strlen (tiles2);
if (count > n) {
  player_log ("Error in %s count (%d) > n (%d)\n", __func__, count, n);
  exit (1);
  }
if (count == 1) {
  player_log ("Error in %s count == 1\n", __func__);
  exit (1);
  }
if (count == 2) {
  int n = num_occurrence_in_open_tiles (c);
  if (n > 2) {
#ifdef DEBUG
    player_log ("%s rejects because number of %s in open tiles (%d) > 2\n", __func__, tile_label[c], n);
#endif
    return 0;
    }
  }
int cnt = 0;
for (i=0; i<n; i++) {
  if (c == tiles2[i]) {
    tiles2[i] = ' ';
    cnt++;
    if (cnt == count)
      break;
    }
  }
if (cnt == count) {
  remove_spaces (tiles2);
  strcpy (tiles, tiles2);
// player_log ("found and tiles is now \"%s\"\n", tiles);
  return c;
  }
else
  return 0;
} // try_remove_tile ()

int try_last (char *tiles, int n1,
char *pattern, char *discard)
{
char c;
int i;
char tiles2[20];
// player_log ("%s starts\n", __func__);
int n = strlen (tiles);;
if (n1 > n) {
  player_log ("Error in %s n1 (%d) > n (%d)\n", __func__, n1, n);
  return -1;
  // exit (1);
  }
for (i=0; i<n; i++) {
  strcpy (tiles2, tiles);
  c = tiles2[i];
  if (try_remove_tile (c, n1, tiles2) != 0) {
    memset (pattern, c, n1); pattern[n1] = '\0';
    strcpy (discard, tiles2);
#ifdef DEBUG
    player_log ("%s finds %d %c and returns pattern=\"%s\" discard=\"%s\"\n", __func__, n1, c, pattern, discard);
#endif
    return strlen (discard);
    }
  } // for
// player_log ("%s returns -1\n", __func__);
return -1;
} // try_last()

// =========================================

int bin_total_len (const char *b_type)
{
int n = strlen (b_type);
if (n % 2 != 0) {
  fprintf (stderr, "Error in %s b_type (%s) length is not even\n", __func__, b_type);
  exit (1);
  }
int i;
int total = 0;
for (i=0; i<n; i+=2) {
  char c = b_type[i];
  int d = b_type[i+1] - '0';
  if (c == 'P') {
    if ((d == 2) || (d == 3))
      total += d;
    else {
      fprintf (stderr, "Error in %s %c%d is not recognized\n", __func__, c, d);
      exit (1);
      }
    }  // P
  else if (c == 'C') {
    if ((d == 2) || (d == 3))
      // total +=3;
      total +=d;
    else {
      fprintf (stderr, "Error in %s %c%d is not recognized\n",__func__, c, d);
      exit (1);
      }
    } // C
  else {
    fprintf (stderr, "Error in %s %c%d is not recognized\n", __func__, c, d);
    exit (1);
    }
  } // for
return total;
} // bin_total_len ()

// =========================================

int try_bin_type (const char *bin_name, char *tiles, char bin[6][20])
{
#ifdef DEBUG
player_log ("%s (%s, %s) starts\n", __func__, bin_name, tiles);
#endif

int r = 0;
	
bin[0][0] = '\0';
bin[1][0] = '\0';
bin[2][0] = '\0';
bin[3][0] = '\0';
bin[4][0] = '\0';
bin[5][0] = '\0';

int t = bin_total_len (bin_name);
int n = strlen (tiles);
if (t >= n) {
  // player_log ("skipping\n");
  return -1;
  }

char bin1_type = bin_name[0];
int bin1_len = bin_name[1] - '0';

char tiles2[20];

if (strlen(bin_name) == 2) {
  strcpy (tiles2, tiles);
  if (bin1_type == 'P')
    return try_last (tiles2, bin1_len, bin[1], bin[0]);
  if  (bin1_type == 'C') {
    if (bin1_len == 3) {
      if (has_chow (tiles2, bin[1]) == 1) {
        strcpy (bin[0], tiles2);
        r = strlen (bin[0]);
        return r;
        }
      else
        return -1;
      } // if (bin1_len == 3) 
    if (bin1_len == 2) {
      if (has_pchow (tiles2, bin[1]) == 1) {
        strcpy (bin[0], tiles2);
        r = strlen (bin[0]);
        return r;
        }
      else
        return -1;	
      } // if (bin1_len == 2) 
    fprintf (stderr, "Error in %s bin1_type == 'C' but bin1_len (%d) is not 2 or 3\n", __func__, bin1_len);
    exit (1);
    }  //  if  (bin1_type == 'C')
  fprintf (stderr, "Should not be here at line %d\n", __LINE__);
  exit (1);
  } // if (strlen(bin_name) == 2)
	
char b[6][20];
char bin_name2[20];
strcpy (bin_name2, bin_name+2);

// player_log ("bin_name2 = [%s] and t = %d\n", bin_name2, t);

if (bin1_type == 'P') {	
char c;
int i;
char tiles2[20];
for (i=0; i<n; i++) {
  strcpy (tiles2, tiles);
  c = tiles2[i];
  if (try_remove_tile (c, bin1_len, tiles2) != 0) {
    // if successful, tiles will be updated
    // player_log ("%s finds %d %c and tiles2=%s\n", __func__, n1, c, tiles2);
    r = try_bin_type (bin_name2, tiles2, b);
    if (r >=0) {
      memset (bin[1], c, bin1_len); bin[1][bin1_len] = '\0';
      strcpy (bin[0], b[0]);
      strcpy (bin[2], b[1]);
      strcpy (bin[3], b[2]);
      strcpy (bin[4], b[3]);
      strcpy (bin[5], b[4]);
      return r;
      }
    }
//  player_log ("%s skips the rest occurrences of %c in %s\n", __func__, c, tiles);
  char *ptr = strrchr (tiles, c);
  if (ptr == NULL) {
    player_log ("Error: ptr == NULL\n");
    exit (1);
    }
  i = ptr - tiles;
  } // for
return -1;
} //   if (bin1_type == 'P)


strcpy (tiles2, tiles);
	
if (bin1_type == 'C') {	
  if (bin1_len == 3) {
    if (has_chow(tiles2, bin[1]) == 0)
    return -1;
    r = try_bin_type (bin_name2, tiles2, b);
    if (r != -1) {
      strcpy (bin[0], b[0]);
      strcpy (bin[2], b[1]);
      strcpy (bin[3], b[2]);
      strcpy (bin[4], b[3]);
      strcpy (bin[5], b[4]);
      return r;
      }
    } // if (bin1_len == 3)
  if (bin1_len == 2) { 
    if (has_pchow(tiles2, bin[1]) == 0)
      return -1;
    r =  try_bin_type (bin_name2, tiles2, b);
    if (r != -1) {
      strcpy (bin[0], b[0]);
      strcpy (bin[2], b[1]);
      strcpy (bin[3], b[2]);
      strcpy (bin[4], b[3]);
      strcpy (bin[5], b[4]);
      return r;
      }
    } // if (bin1_len == 2)
  return -1;
  } // if (bin1_type == 'C')	
fprintf (stderr, "Should not be here at line %d\n", __LINE__);
exit (1);
} // try_bin_type ()

// =========================================

int enumerate_bin (char *tiles, int *deficiency_num, char *bin_type, char *bin1, char *bin2, char *bin3, char *bin4, char *bin5, char *discard_bin)
{
bin_type[0] = '\0';
bin1[0] = '\0';
bin2[0] = '\0';
bin3[0] = '\0';
bin4[0] = '\0';
bin5[0] = '\0';
discard_bin[0] = '\0';
int found;
int discard;
int min_discard = 999;
/*
char b_type[15];
char b1[15];
char b2[15];
char b3[15];
char b4[15];
char b5[15];
char dbin[15];
*/
*deficiency_num = -1;
int i;
found = 0;
char bins[6][20];
for (i=0; i<BIN_COUNT; i++) {
discard = try_bin_type (bin_name[i], tiles, bins);
//  discard = try_bin (tiles, i, b_type, b1, b2, b3, b4, b5, dbin);
// fprintf (stderr, "inside %s discard = %d\n", __func__, discard);
  if (discard == -1)
    continue;
  player_log ("candidate found bin_name=%s discard=%d b0=%s b1=%s b2=%s b3=%s b4=%s b5=%s\n", bin_name[i], discard, bins[0], bins[1], bins[2], bins[3], bins[4],bins[5]);
 discard -= (num_str("P3", bin_name[i]) * P3_BONUS ) + num_str("C3", bin_name[i]) * C3_BONUS;
//  player_log ("corrected value of discard = %d\n", discard);
if (strstr (bin_name[i], "P2") != NULL) {
  player_log ("bonus for P2\n");
  discard--; // bonus for existence of eye
  }
  if (discard < min_discard) {
    found = 1;
    min_discard = discard;
    *deficiency_num = discard;
    strcpy (bin_type, bin_name[i]);
    strcpy (bin1, bins[1]);
    strcpy (bin2, bins[2]);
    strcpy (bin3, bins[3]);
    strcpy (bin4, bins[4]);
    strcpy (bin5, bins[5]);
    strcpy (discard_bin, bins[0]);
    }
  } // for
if (found)
  return 1;
return 0;
} // enumerate_bin()

// =========================================

char discard_strategy3 (char *tiles)
{
int deficiency_num;
char bin_type[20];
char bin1[20];
char bin2[20];
char bin3[20];
char bin4[20];
char bin5[20];
char discard_bin[20];

int n = strlen(tiles);

player_log ("calling enumerate_bin\n");
if (enumerate_bin (tiles, &deficiency_num, bin_type, bin1, bin2, bin3, bin4, bin5, discard_bin)) {
player_log ("deficiency_num=%d ", deficiency_num);
player_log ("bin_type=%s ", bin_type);
player_log ("bin1=[%s] ", tile_labels(bin1));
player_log ("bin2=[%s] ", tile_labels(bin2));
player_log ("bin3=[%s] ", tile_labels(bin3));
player_log ("bin4=[%s] ", tile_labels(bin4));
player_log ("bin5=[%s] ", tile_labels(bin5));
player_log ("discard_bin=[%s]\n", tile_labels(discard_bin));
strcpy (mybin_type, bin_type);
strcpy (mybin[0], bin1);
strcpy (mybin[1], bin2);
strcpy (mybin[2], bin3);
strcpy (mybin[3], bin4);
strcpy (mybin[4], bin5);
if (strlen(discard_bin) == 1)
  return discard_bin[0];
if (strlen(discard_bin) != 0)
  return discard_strategy0 (discard_bin);
}
mybin_type[0] = '\0';
mybin[0][0] = '\0';
mybin[1][0] = '\0';
mybin[2][0] = '\0';
mybin[3][0] = '\0';
mybin[4][0] = '\0';

return discard_strategy1 (tiles);
} // discard_strategy3

// =========================================
char discard_strategy (char *tiles)
{
switch (strategy_num) {
  case 1:
    return discard_strategy1 (tiles);
  case 2:
    return discard_strategy2 (tiles);
  case 3:
    return discard_strategy3 (tiles);
  case 4:
    fprintf (stderr, "Error in %s, should not be here!\n", __func__);
    exit (1);
  default:
    return discard_strategy0 (tiles);
  }
} // discard_strategy ()

// =========================================
char discard_one_tile (char *tiles)
{
int n;
char c;
n = strlen (tiles);
if (n == 0) {
  fprintf (stderr, "Error: strlen is already 0 in %s\n", __func__);
  exit (0);
  }
c = discard_strategy (tiles);
// TEMPORARILY always discard the last
// c = tiles[n-1];
// tiles[n-1] = '\0';
char *p = strchr (tiles, c);
if (p == NULL) {
  fprintf (stderr, "Error in %s, strchr returns NULL\n", __func__);
  exit (1);
  }
*p = ' ';
remove_spaces (tiles);
// player_log ("tiles's length changes from %d to %d\n", n, strlen(tiles));
return c;
} // discard_one_tile ()