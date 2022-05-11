#include "pi_mahjong.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// =========================================
int add_table_tile (int tile, int count)
{
define_tile_labels();
int i;
for (i=0; i<count; i++) {
  tiles [tiles_count] = tile;
  tiles_count ++;
  }
} // add_table_tile()

// =========================================
int create_deck ()
{
tiles_count = 0;
memset (tiles, '\0', sizeof(tiles));
// for (i=0; i<MAX_TILES; i++)
//   tiles [i] = 0;
add_table_tile (ONE_DOT, 4);
add_table_tile (TWO_DOT, 4);
add_table_tile (THREE_DOT, 4);
add_table_tile (FOUR_DOT, 4);
add_table_tile (FIVE_DOT, 4);
add_table_tile (SIX_DOT, 4);
add_table_tile (SEVEN_DOT, 4);
add_table_tile (EIGHT_DOT, 4);
add_table_tile (NINE_DOT, 4);

add_table_tile (ONE_BAMBOO, 4);
add_table_tile (TWO_BAMBOO, 4);
add_table_tile (THREE_BAMBOO, 4);
add_table_tile (FOUR_BAMBOO, 4);
add_table_tile (FIVE_BAMBOO, 4);
add_table_tile (SIX_BAMBOO, 4);
add_table_tile (SEVEN_BAMBOO, 4);
add_table_tile (EIGHT_BAMBOO, 4);
add_table_tile (NINE_BAMBOO, 4);

add_table_tile (ONE_CHARACTER, 4);
add_table_tile (TWO_CHARACTER, 4);
add_table_tile (THREE_CHARACTER, 4);
add_table_tile (FOUR_CHARACTER, 4);
add_table_tile (FIVE_CHARACTER, 4);
add_table_tile (SIX_CHARACTER, 4);
add_table_tile (SEVEN_CHARACTER, 4);
add_table_tile (EIGHT_CHARACTER, 4);
add_table_tile (NINE_CHARACTER, 4);

add_table_tile (EAST_WIND, 4);
add_table_tile (SOUTH_WIND, 4);
add_table_tile (WEST_WIND, 4);
add_table_tile (NORTH_WIND, 4);

add_table_tile (RED_DRAGON, 4);
add_table_tile (GREEN_DRAGON, 4);
add_table_tile (WHITE_DRAGON, 4);
} // create_deck()

// =========================================
void shuffle_deck ()
// using Knuth algorithm
{
srandom(time(NULL));
int i;
for (i=tiles_count-1; i>0; i--)
{
int index = random() % i;
int a = tiles[index];
tiles [index] = tiles[i];
tiles [i] = a;
} // for
} // Knuth_Shuffle()

// =========================================

long arbitrate_chow_pung_win (long discard_player, char *respond, int *player2wind)
{
if (strlen (respond) != 5) {
  printf ("Error, strlen (respond) !=5\n");
  exit (1);
  }
#ifdef DEBUG
player_log ("%s (%d, \"%s\") starts\n", __func__, discard_player, respond);
#endif
char *p_win1;
char *p_chow;
char *p_pung;
long win1;
p_win1  = strchr (respond, 'W');
p_chow = strchr (respond, 'S');
p_pung = strchr (respond, 'T');
// multiple win not implemented yet
if (p_win1 != NULL) {
  int num_win = num_occurrence ('W', respond);
  if (num_win == 1) {
    player_log ("only one player declares win\n");
    win1 = (long) (p_win1 - respond);
    player_log ("Player %ld Wins\n", win1);
    return win1;
    }
  player_log ("%d players declare Win\n", num_win);
  if (num_win == 4) {
    fprintf (stderr, "It is impossible for four players to declare Win\n");
    exit (1);
    }
  char *p_win2 = strchr (p_win1+1, 'W');
  if (p_win2 == NULL) {
    fprintf (stderr, "Error in %s p_win2 == NULL\n", __func__);
    exit (1);
    }
  char *p_win3 = strchr (p_win2+1, 'W'); // p_win3 may be NULL
  long win2, win3;
  win1 = (long) (p_win1 - respond);
  win2 = (long) (p_win2 - respond);
  if (p_win3)
    win3 = (long) (p_win3 - respond);
  else
    win3 = 0L;
/*
  player_log ("The two players are %d and %d and player2wind map is [%d%d%d%d]\n",
    win1, win2,
    player2wind[1], player2wind[2],  player2wind[3],  player2wind[4]);
*/
  int d1 = player2wind[win1] - player2wind[discard_player];
  int d2 = player2wind[win2] - player2wind[discard_player];
  int d3;
  if (win3)
    d3 = player2wind[win3] - player2wind[discard_player];
  else
    d3 = 999;
  if (d1 < 0)
    d1 += 4;
  if (d2 < 0)
    d2 += 4;
  if (d3 < 0)
    d3 += 4;
  if ((d1 < d2) && (d1 < d3)) {
    player_log ("Player %ld Wins\n", win1);
    return win1;
    }
  if ((d2 < d1) && (d2 < d3)) {
    player_log ("Player %ld Wins\n", win2);
    return win2;
    }
  if (win3) {
    if ((d3 < d1) && (d3 < d2)) {
      player_log ("Player %ld Wins\n", win3);
      return win3;
      }
    }
  fprintf (stderr, "Error in %s should not arrive here at line %d\n", __func__, __LINE__);
  exit (1);
  } // if (p_win1 != NULL)
if (p_pung != NULL) {
  if (strchr (p_pung+1, 'T') != NULL) {
    fprintf (stderr, "Error: more than one Pung in %s\n", respond);
    exit (1);
    }
  long pung_player =  (long) (p_pung - respond);
  player_log ("Player %ld Pungs\n", pung_player);
  return pung_player;
  }
if (p_chow != NULL) {
  long chow_player =  (long) (p_chow - respond);
  if ((player2wind[chow_player] - player2wind[discard_player] + 4) % 4 != 1) {
    fprintf (stderr, "Chow player (%d) is not downstream of the discard player (%d) and the player2wind map is [%d%d%d%d]\n",
      chow_player, discard_player,
      player2wind[1], player2wind[2],  player2wind[3],  player2wind[4]);
    exit (1);
    }
  player_log ("Player %ld Chows\n", chow_player);
  return chow_player;
  } // if (p_chow != NULL)
fprintf (stderr, "Error in %s: no win / chow / pung found\n", __func__);
exit (1);
}



int main(int argc, char *argv[])
{
int self_draw_win_count = 0;
int self_draw_win = 0;
int other_win_count = 0;
int no_one_win_count = 0;
int stay_seat_count = 0;
int max_round;
int round_num;
int hand_num;
int player_win_count[5] = {0};
int fund[5] = {0};
char finally_revealled_tiles[5][19];

if (argc != 2) {
  fprintf (stderr, "Usage: %s number_of_round\n", argv[0]);
  exit(EXIT_FAILURE);
  }

max_round = atoi (argv[1]);

// only table has right to create the MQ
qid2table = msgget(TABLE_MSGKEY , IPC_CREAT | 0666);
if (qid2table == -1) {
  fprintf (stderr, "msgget get errno %d\n", errno);
  exit(EXIT_FAILURE);
  }
player_log ("qid2table = %d\n", qid2table);

qid2player = msgget(PLAYER_MSGKEY , IPC_CREAT | 0666);
if (qid2player == -1) {
  fprintf (stderr, "msgget get errno %d\n", errno);
  exit(EXIT_FAILURE);
  }
player_log ("qid2player = %d\n", qid2player);

flush_mq();

// wait for player
// int player_registered[5] = {0,0,0,0,0};
long wind2player [4] = {0L, 0L, 0L, 0L}; // 0:E; 1:S; 2:W; 3:N
int player2wind [5] = {-1, -1, -1, -1, -1}; // 0:E; 1:S; 2:W; 3:N
long from_player;
char msg[PI_MAHJONG_MSG_LEN];
int wind = 0; // 0:E; 1:S; 2:W; 3:N
while ((player2wind[1]==-1) || (player2wind[2]==-1) || (player2wind[3]==-1) || (player2wind[4]==-1)) {
  from_player = wait_msg_from_player(msg);
  if (from_player == 0) {
    fprintf (stderr, "Error: from_player is 0 in %s\n", __func__);
    exit (1);
    }
  if (msg[0] == 'R') {
    if (player2wind [from_player] == -1) {
      player2wind[from_player] = wind;
      wind2player[wind] = from_player;
      wind++;
      fund[from_player] = atoi (((struct register_format *) msg)->initial_fund);
      }
    else {
      fprintf (stderr, "Error: player %ld double register\n", from_player);
      exit (1);
      }
    }
  else {
    fprintf (stderr, "Error: Expect R (Register) message but get %c\n", msg[0]);
    exit (1);
    }
  } // while

player_log ("players have the following initial funds: (1) %d (2) %d (3) %d (4) %d\n", fund[1], fund[2], fund[3], fund[4]);
player_log ("starting new game\n");
int i;
for (i=0; i<4; i++) {
  player_log ("Calling send_new_game_msg() to player %ld for seat %c\n", wind2player[i], wind_label[i]);
  send_new_game_msg (wind2player[i], wind_label[i]);
  } 

round_num = 1;
hand_num = 1;
//long winner_player = 0L;

while (1) { // a new hand
long winner_player = 0L;
create_deck();
// player_log ("after create_deck()\n");
player_log ("deck \"%s\"\n", tile_labels(tiles));
shuffle_deck();
player_log ("deck \"%s\"\n", tile_labels(tiles));
player_log ("starting new hand number (%d)\n", hand_num);
char tmp_tiles[13];

long player_num;
for (player_num=1; player_num<=4; player_num++) {
  char tmp_tiles[14];
  if (table_draw_tiles (13, tmp_tiles) != 1) {
    fprintf (stderr, "Error in table_draw_tiles\n");
    exit (1);
    }
  player_log ("Calling send_new_hand_msg() to player %d\n", player_num);
  send_new_hand_msg (player_num, wind_label[(round_num-1)%4], wind_label[(hand_num-1)%4],
 wind_label[player2wind[1]], wind_label[player2wind[2]], wind_label[player2wind[3]], wind_label[player2wind[4]],
 tmp_tiles);
  } 

long draw_seat = 0; // 'E'
player_log ("initial draw seat is %c\n", wind_label [draw_seat]);
char player_respond[6];
memset (open_discard, '\0', sizeof(open_discard));
memset (open_meld, '\0', sizeof(open_meld));

  char discard_tile;
  long discard_player;

while (tiles_count > 0) { // iterate draw seat
  // draw a tile
  if (table_draw_tiles (1, tmp_tiles) != 1) {
    fprintf (stderr, "Error in table_draw_tiles\n");
    exit (1);
    }
  player_log ("player %ld draws tile %s\n", wind2player[draw_seat], tile_label[tmp_tiles[0]]);
  send_draw_tile_msg (wind2player[draw_seat], tmp_tiles[0]);

  from_player = wait_msg_from_player(msg);
  if (from_player != wind2player[draw_seat]) {
    fprintf (stderr, "Error getting message from player %ld which is not draw seat (%s)\n", from_player, wind_label[draw_seat]);
    exit (1);
    }
  if (msg[0] == 'W') {
    player_log ("self draw win\n");
    broadcast_win_msg (from_player);  // then all players should reveal all tiles
    winner_player = from_player;
    self_draw_win_count++;
    self_draw_win = 1;
    player_win_count[winner_player]++;
    break;
    }
  if (msg[0] != 'C') {
    fprintf (stderr, "Error, expect Discard message but receive %c\n", msg[0]);
    exit (1);
    }
//  char discard_tile;
//  long discard_player;
DISCARD_LABEL:
  discard_tile = ((struct discard_format *) msg)->discard_tile;
  discard_player = from_player;
  if (('0' + discard_player) != ((struct discard_format *) msg)->discard_player) {
    fprintf (stderr, "Error from_player (%ld) != discard_player (%c)\n", from_player, ((struct discard_format *) msg)->discard_player);
    exit (1);
    }

  player_log ("Player %ld discards %s\n", discard_player, tile_label[discard_tile]);
  relay_discard_msg (discard_player, discard_tile);

  // wait for Pass, Chow , Pung or Win
  long player_i;
  strcpy (player_respond, "     ");
  player_respond[discard_player] = 'P'; // the original discard player always passes
  char chow_tile;
  char other_tile1;
  char other_tile2;
  do {
    player_i = wait_msg_from_player (msg);
    if (player_respond [player_i] != ' ') {
      fprintf (stderr, "Error: player %ld double responds (previous %c, now %c)\n", player_i, player_respond[player_i], msg[0]);
      exit (1);
      }
    player_respond[player_i] = msg[0];
    if (msg[0] == 'S') { // chow
      chow_tile = ((struct chow_format *) msg)->chow_tile;
      other_tile1 = ((struct chow_format *) msg)->other_tile1;
      other_tile2 = ((struct chow_format *) msg)->other_tile2;
      if (chow_tile != discard_tile) {
        fprintf (stderr, "Error, chow_tile (%s) != discard_tile (%s)\n", tile_label[chow_tile], tile_label[discard_tile]);
        exit (1);
        }
      } // chow msg
    } while ((player_respond[1] == ' ') ||
             (player_respond[2] == ' ') ||
             (player_respond[3] == ' ') ||
             (player_respond[4] == ' '));
//   player_log ("player responses are \"%c%c%c%c\"\n", player_respond[1], player_respond[2], player_respond[3], player_respond[4]);

  // check all pass
  if ((player_respond[1] == 'P') && (player_respond[2] == 'P') &&
      (player_respond[3] == 'P') && (player_respond[4] == 'P')) {
   player_log ("all players pass\n");
  add_open_discard (discard_tile, discard_player);
   if (tiles_count > 0) {
     if (++draw_seat == 4)
       draw_seat = 0;
     player_log ("now next seat %c draws\n", wind_label[draw_seat]);
     broadcast_next_drawer_msg (wind_label[draw_seat]);
     }
    } // all pass
  else {
    long arbitrate_player = arbitrate_chow_pung_win (discard_player, player_respond, player2wind);
    // player_log ("player %d %c overrides in arbitration\n", arbitrate_player, player_respond[arbitrate_player]);
    if (player_respond[arbitrate_player] == 'W') {
      winner_player = arbitrate_player;
      broadcast_win_msg (arbitrate_player);  // then all players should reveal all tiles
      other_win_count++;
      self_draw_win = 0;
      player_win_count[winner_player]++;
      player_log ("next round\n");
      break;
      }
    if (player_respond[arbitrate_player] == 'T') {
      add_pung_tiles (arbitrate_player, discard_tile);
      broadcast_pung_msg (arbitrate_player, discard_tile);
      // printf ("arbitrate_player = %ld\n", arbitrate_player);
      draw_seat = player2wind[arbitrate_player];
      player_log ("changing draw seat to arbitrate player %c\n", wind_label[draw_seat]);
      from_player = wait_msg_from_player(msg);
      if (msg[0] != 'C') {
        fprintf (stderr, "Error, expect Discard message but receive %c\n", msg[0]);
        exit (1);
        }
      if (from_player != arbitrate_player) {
        fprintf (stderr, "Error getting message from player %ld which is not arbitrate_player (%ld)\n", from_player, arbitrate_player);
        exit (1);
        }
      goto DISCARD_LABEL;
      } // Pung

    if (player_respond[arbitrate_player] == 'S') { // Chow
      add_chow_tiles (arbitrate_player, chow_tile, other_tile1, other_tile2);
      broadcast_chow_msg (arbitrate_player, chow_tile, other_tile1, other_tile2);
      draw_seat = player2wind[arbitrate_player];
      player_log ("changing draw set to arbitrate player %c\n", wind_label[draw_seat]);
      from_player = wait_msg_from_player(msg);
      if (msg[0] != 'C') {
        fprintf (stderr, "Error, expect Discard message but receive %c\n", msg[0]);
        exit (1);
        }
      if (from_player != arbitrate_player) {
        fprintf (stderr, "Error getting message from player %ld which is not arbitrate_player (%ld)\n", from_player, arbitrate_player);
        exit (1);
        }
      goto DISCARD_LABEL;
      } // Chow
    
    } // else all pass

  } // while (tiles_count > 0)
if (tiles_count == 0) {
  player_log ("Round %d (%c) Hand %d (%c) ends with no one wins\n", round_num, wind_label[(round_num-1)%4], hand_num, wind_label[(hand_num-1)%4]);
  no_one_win_count++;
  broadcast_end_hand_msg (wind_label[(round_num-1)%4], wind_label[(hand_num-1)%4]);
  }
else {
  if (winner_player == 0L) {
    fprintf (stderr, "Error: winner_player == 0L\n");
    exit (1);
    }
  player_log ("Round %d (%c) Hand %d (%c) ends with player %ld wins\n", round_num, wind_label[(round_num-1)%4], hand_num, wind_label[(hand_num-1)%4], winner_player);
  broadcast_end_hand_msg (wind_label[(round_num-1)%4], wind_label[(hand_num-1)%4]);
  }

long player_e;
strcpy (player_respond, "     ");
do {
    player_e = wait_msg_from_player (msg);
    if (player_respond [player_e] != ' ') {
      fprintf (stderr, "Error: player %ld double responds (previous %c, now %c)\n", player_e, player_respond[player_e], msg[0]);
      exit (1);
      }
    player_respond[player_e] = msg[0];
// player_log ("Player %ld reveals all tiles \"%s\"\n", player_e, tile_labels(((struct end_hand_format *) msg)->final_revealed_tiles));
strcpy (finally_revealled_tiles[player_e], (((struct end_hand_format *) msg)->final_revealed_tiles));

} while ((player_respond[1] == ' ') ||
             (player_respond[2] == ' ') ||
             (player_respond[3] == ' ') ||
             (player_respond[4] == ' '));
//  player_log ("player responses are \"%c%c%c%c\"\n", player_respond[1], player_respond[2], player_respond[3], player_respond[4]);

if (memcmp (player_respond+1, "EEEE", 4 ) != 0) {
  fprintf (stderr, "Error: player_respond is not EEEE\n");
  exit (1);
  }

if (tiles_count != 0) {
// someone wins
int fan = calculate_fan (open_meld[winner_player], finally_revealled_tiles[winner_player],
  wind_label[(round_num-1)%4],  wind_label[(hand_num-1)%4], wind_label[player2wind[winner_player]]);

int point = fan2point (fan);
player_log ("fan = %d point = %d\n", fan, point);
if (self_draw_win) {
  fund[1] -= point;
  fund[2] -= point;
  fund[3] -= point;
  fund[4] -= point;
  fund[winner_player] += (4 * point);
  }
else {
  fund[winner_player] += point;
  fund[discard_player] -= point;
  }
player_log ("players have the following final funds: (1) %d (2) %d (3) %d (4) %d\n", fund[1], fund[2], fund[3], fund[4]);
} // someone wins

broadcast_reveal_msg (fund, finally_revealled_tiles[1], finally_revealled_tiles[2],
finally_revealled_tiles[3], finally_revealled_tiles[4]);

char t[1000];
player_log ("Revealing all tiles:\n");
strcpy (t, tile_labels(open_meld[1]));
player_log ("Player 1 %s | %s\n", t, tile_labels(finally_revealled_tiles[1]));
strcpy (t, tile_labels(open_meld[2]));
player_log ("Player 2 %s | %s\n", t, tile_labels(finally_revealled_tiles[2]));
strcpy (t, tile_labels(open_meld[3]));
player_log ("Player 3 %s | %s\n", t, tile_labels(finally_revealled_tiles[3]));
strcpy (t, tile_labels(open_meld[4]));
player_log ("Player 4 %s | %s\n", t, tile_labels(finally_revealled_tiles[4]));
/*
player_log ("Player 1 %s | ", tile_labels(open_meld[1]));
player_log ("%s\n", tile_labels(finally_revealled_tiles[1]));
player_log ("Player 2 %s | ", tile_labels(open_meld[2]));
player_log ("%s\n", tile_labels(finally_revealled_tiles[2]));
player_log ("Player 3 %s | ", tile_labels(open_meld[3]));
player_log ("%s\n", tile_labels(finally_revealled_tiles[3]));
player_log ("Player 4 %s | ", tile_labels(open_meld[4]));
player_log ("%s\n", tile_labels(finally_revealled_tiles[4]));
*/

player_log ("open_discards are as follows:\n");
player_log ("Player 1 (%d tiles): %s\n", strlen(open_discard[1]), tile_labels(open_discard[1]));
player_log ("Player 2 (%d tiles): %s\n", strlen(open_discard[2]), tile_labels(open_discard[2]));
player_log ("Player 3 (%d tiles): %s\n", strlen(open_discard[3]), tile_labels(open_discard[3]));
player_log ("Player 4 (%d tiles): %s\n", strlen(open_discard[4]), tile_labels(open_discard[4]));

if (tiles_count != strlen (tiles)) {
  fprintf (stderr, "Error: tiles_count (%d) != strlen (tiles) (%d)\n", tiles_count, strlen (tiles));
  exit (1);
  }
sort_tiles (tiles, tiles_count);
player_log ("Wall tiles (%d) are \"%s\"\n", tiles_count, tile_labels(tiles));

if (player2wind[winner_player] == 0) { // East
  stay_seat_count++;
  player_log ("stay seats because winner_player (%ld) is the dealer (%d)\n", winner_player, player2wind[winner_player]);
  }
else {
  hand_num++;
  if (hand_num < 5)
    player_log ("increment hand_num and now becomes %d\n", hand_num);
  else {
    round_num++;
    if (round_num > max_round) {
      player_log ("Game over after reaching %d round(s)\n", max_round);
      break;
      // exit (0);
      }
    hand_num = 1;
    player_log ("new round (round_num = %d) (and hand_num reset to 1)\n", round_num);
    }
  rotate_seats (player2wind, wind2player);
  }
} // while hand
broadcast_end_game_msg (max_round);
printf ("self_draw_win_count = %d\n" , self_draw_win_count);
printf ("other_win_count = %d\n", other_win_count);
printf ("no_one_win_count = %d\n", no_one_win_count);
printf ("stay_seat_count = %d\n", stay_seat_count);
printf ("player 1 wins %d\n", player_win_count[1]);
printf ("player 2 wins %d\n", player_win_count[2]);
printf ("player 3 wins %d\n", player_win_count[3]);
printf ("player 4 wins %d\n", player_win_count[4]);
printf ("players have the following final funds: (1) %d (2) %d (3) %d (4) %d\n", fund[1], fund[2], fund[3], fund[4]);
sleep (1);

remove_mq();
printf ("Game ends\n");
} // main()
