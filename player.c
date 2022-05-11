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

int add_player_tile (char *deck, char tile)
{
// assume deck has sufficient spare capacity
int n = strlen (deck);
deck[n] = tile;
deck[n+1] = '\0';
sort_tiles (deck, n+1);
}


// =========================================
char search_tiles (char *tiles, char *label)
{
int n = strlen (tiles);
if (n == 0)
  return 0;
if (strlen (label) != 2)
  return 0;
for (int i=0; i<n; i++) {
  if (strcmp (label, tile_label[tiles[i]]) == 0) {
    char c = tiles[i];
    tiles[i] = ' ';
    remove_spaces (tiles);
    return c;
    }
  } // for
return 0;
} // search_tiles ()

#define RED_COLOR    "\033[0;31m"
#define GREEN_COLOR  "\033[0;32m"
#define YELLOW_COLOR "\033[0;33m"
#define BLUE_COLOR   "\033[0;34m"
#define RESET_COLOR  "\033[0m"
char human_input_discard_tile_label (char *tiles)
{
#ifdef Chinese
int ans;
char discard;
int n = strlen (tiles);
while (1) {
  printf (YELLOW_COLOR "What tile to discard? " RESET_COLOR);
  fscanf (stdin, "%d", &ans);
  if (ans == 0)
    return 0;
  if (!((ans > 0) && (ans <= n))) {
    printf ("Please input the tile label...");
    continue;
    }
  discard = search_tiles (tiles, tile_label[tiles[ans-1]]);
  return discard;
  }
#else
char ans[20] = {'\0'};
char discard;
while (1) {
  printf (YELLOW_COLOR "What tile to discard? " RESET_COLOR);
  fscanf (stdin, "%s", ans);
  if (strlen (ans) == 0)
    return 0;
  if (strlen (ans) != 2) {
    printf ("Please input the tile label...");
    continue;
    }
  discard = search_tiles (tiles, ans);
  if (discard == 0)
    printf ("Tile labels not found... ");
  else
    return discard;
  }
#endif
} // human_input_discard_tile_label ()

int main(int argc, char *argv[])
{
int initial_fund = 1000;
char msg[PI_MAHJONG_MSG_LEN]; // mq message

if (argc < 2) {
  fprintf (stderr, "Usage: %s player_numer (1-4) [strategy_num]\n", argv[0]);
  exit(EXIT_FAILURE);
  }

my_player = atol (argv[1]);

if (argc == 3)
  strategy_num = atoi (argv[2]);
else
  strategy_num = 0;

if (!((my_player >=1) && (my_player <=4))) {
  fprintf (stderr, "Error: player should be 1 - 4\n");
  exit(EXIT_FAILURE);
  }

// *** Variable initialization ******
qid2table = msgget(TABLE_MSGKEY , 0666); // no IPC_CREAT
if (qid2table == -1) {
  if (errno == ENOENT)
    fprintf (stderr, "Error: probably the \"table\" process has not be started\n");
  else 
    fprintf (stderr, "msgget(TABLE_MSGKEY) get errno %d\n", errno);
  exit(EXIT_FAILURE);
  }
player_log ("qid2table = %d\n", qid2table);

qid2player = msgget(PLAYER_MSGKEY , 0666); // no IPC_CREAT
if (qid2player == -1) {
  fprintf (stderr, "msgget get errno %d\n", errno);
  exit(EXIT_FAILURE);
  }
player_log ("qid2player = %d\n", qid2player);

define_tile_labels();

#ifdef Chinese
define_tile_labels_c();
#endif

// ################## Game starts ################

send_register_msg (my_player, initial_fund);

wait_msg_from_table(my_player, msg);
if (msg[0] != 'N') { // New Game
  fprintf (stderr, "Player %ld Error: Expect N but receive %c\n", my_player, msg[0]);
  exit (1);
  }

char my_initial_seat_wind;
my_initial_seat_wind = ((struct new_game_format *) msg)->seat;

player_log ("My initial seat wind is %c\n", my_initial_seat_wind);

char round_wind;
char hand_wind;
char draw_seat;

while (1) { // new hand

memset (my_secret_tiles, 0, 15);
memset (open_discard, '\0', sizeof(open_discard));
memset (open_meld, '\0', sizeof(open_meld));
memset (mybin, '\0', sizeof(mybin));
memset (mybin, '\0', sizeof(mybin));
memset (mybin_type, '\0', sizeof(mybin_type));

wait_msg_from_table(my_player, msg);
if (msg[0] == 'G') { // end game
  player_log ("End game around %s rounds\n", ((struct end_game_format *) msg)->total_round);
  exit (0);
  }
if (msg[0] != 'H') {
  fprintf (stderr, "Error: Expect H (New Hand) but receive %c\n", msg[0]);
  exit (1);
  }

round_wind = ((struct new_hand_format *) msg)->round;
hand_wind = ((struct new_hand_format *) msg)->hand;
memcpy (my_secret_tiles, ((struct new_hand_format *) msg)->tiles, 13);
char new_seats[5] = {'\0'};
char my_wind;
memcpy (new_seats, ((struct new_hand_format *) msg)->seats, 4);
my_wind = new_seats[my_player - 1];
player_log ("Round %c Hand %c New Seats \"%s\" gets tiles \"%s\"\n", round_wind, hand_wind, new_seats, tile_labels(my_secret_tiles));
if ((wind2num(my_wind) + wind2num(hand_wind)) % 4 != wind2num(my_initial_seat_wind)) {
  fprintf (stderr, "my_wind (%c) + hand_wind (%c) != my_initial_seat_wind (%c)\n", my_wind, hand_wind, my_initial_seat_wind);
  exit (1);
  }

if (my_wind == 'E')
  player_log ("I am the dealer in this hand\n");

// sort_tiles (new_hand_msg->tiles, 13);
sort_tiles (my_secret_tiles, 13);
// print_tile_labels (my_secret_tiles, labels);
player_log ("after sorting \"%s\"\n", tile_labels(my_secret_tiles));

char discard_tile;
long discard_player;

draw_seat = 'E';

while (1) {
if (my_wind == draw_seat) {
  // I am the drawer
  player_log ("I am the drawer because my_wind (%c) == draw_seat (%c)\n", my_wind, draw_seat);
  wait_msg_from_table(my_player, msg);
  if (msg[0] != 'D') { // Draw Tile
    fprintf (stderr, "Error: Expect D but receive %c\n", msg[0]);
    exit (1);
    }
  char tile_drawn = ((struct draw_tile_format *) msg)->tile;
  player_log ("got tile %s\n", tile_label[tile_drawn]);

  if (player_check_win (my_secret_tiles, tile_drawn) == 1) {
    add_player_tile (my_secret_tiles, tile_drawn);
    send_win_msg (my_player);
    player_log ("***************** Declare Self draw win! **********************\n");
    wait_msg_from_table(my_player, msg);
    if (msg[0] != 'W') {
      fprintf (stderr, "Error: Expect W (Win) but receive %c\n", msg[0]);
      exit (1);
      }
    player_log ("************* I (seat %c) self draw wins\n", my_wind);
    wait_msg_from_table (my_player, msg);
    if (msg[0] != 'E') { // End Hand
      fprintf (stderr, "Error: Expect E (End Hand) but receive %c\n", msg[0]);
      exit (1);
      }
    player_log ("round %c hand %c ends\n", ((struct end_hand_format *) msg)->round, ((struct end_hand_format *) msg)->hand);
    break;
    } // if player_check_win () == 1

  add_player_tile (my_secret_tiles, tile_drawn);

#ifdef Chinese
  chinese_display (my_secret_tiles);
#else
  player_log ("my_secret_tiles become \"%s\"\n", tile_labels(my_secret_tiles));
#endif

  discard_tile = 0;
  if (strategy_num == 4)
    // human player
    discard_tile = human_input_discard_tile_label (my_secret_tiles); // if no input, return 0
  if (discard_tile == 0)
    discard_tile = discard_one_tile (my_secret_tiles);
  player_log ("I discard tile %s\n", tile_label[discard_tile]);
  player_log ("my_secret_tiles become \"%s\"\n", tile_labels(my_secret_tiles));
  discard_player = my_player;
  send_discard_msg (discard_player, discard_tile);
  }
else { // not drawer
  // player_log ("Waiting for drawer to discard tile\n");
  wait_msg_from_table(my_player, msg);
  if (msg[0] == 'W') {
    player_log ("******************* Player %c wins\n", ((struct win_format *) msg)->win_player);
    wait_msg_from_table (my_player, msg);
    if (msg[0] != 'E') { // End Hand
      fprintf (stderr, "Error: Expect E (End Hand) but receive %c\n", msg[0]);
      exit (1);
      }
    player_log ("round %c hand %c ends\n", ((struct end_hand_format *) msg)->round, ((struct end_hand_format *) msg)->hand);
    break;
    }
  if (msg[0] != 'C') {
    fprintf (stderr, "Error: Expect C (Discard) but receive %c\n", msg[0]);
    exit (1);
    }
OTHER_DISCARD:
  discard_tile = ((struct discard_format *) msg)->discard_tile;
  discard_player = (long) (((struct discard_format *) msg)->discard_player - '0');
  player_log ("Player %ld discards tile %s\n",
    discard_player,    
    tile_label[discard_tile]);
  char other_tile1, other_tile2;
  int propose_win, propose_chow, propose_pung;
  propose_chow = 0;
  propose_pung = 0;
  propose_win = player_check_win (my_secret_tiles, discard_tile);
  if (propose_win) {
    send_win_msg (my_player);
    player_log ("***************** Propose Win! **********************\n");
    }
  else {
    propose_pung = player_check_pung (my_secret_tiles, discard_tile);
    if (propose_pung) {
      send_pung_msg (my_player, discard_tile);
      player_log ("***** Propose Pung ********\n");
      }
    else {
/*
player_log ("discard_player = %ld\n", discard_player);
player_log ("seat = %c\n", new_seats[discard_player-1]);
player_log ("next = %c\n", next_seat(new_seats[discard_player-1]));
player_log ("my_wind = %c\n", my_wind);
*/
    if (next_seat(new_seats[discard_player-1]) == my_wind)
      propose_chow = player_check_chow (my_secret_tiles, discard_tile, &other_tile1, &other_tile2);
    else
      propose_chow = 0;
    if (propose_chow) {
      send_chow_msg (my_player, discard_tile, other_tile1, other_tile2);
      player_log ("***** Propose Chow ********\n");
      }
    } // else
   } // else
  if (propose_win || propose_chow || propose_pung) {
    player_log ("waiting for arbitration result\n");
    wait_msg_from_table(my_player, msg);

    if (msg[0] == 'S') { // Chow
      if (((struct chow_format *) msg)->chow_player != ('0' + my_player))
        goto OTHER_PUNG_CHOW_WIN;
      if (!propose_chow) {
        fprintf (stderr, "Error not expected to receive S in line %d\n", __LINE__);
        exit (1);
        }
      player_log ("confirm Chow\n");
      add_chow_tiles (my_player, discard_tile, other_tile1, other_tile2);
      if ((remove_tile (other_tile1, 1, my_secret_tiles) == 0) ||
          (remove_tile (other_tile2, 1, my_secret_tiles) == 0)) {
        fprintf (stderr, "Error in remove_tile()\n");
        exit (1);
        }
#ifdef Chinese
      chinese_display (my_secret_tiles);
#else
      player_log ("my_secret_tiles become \"%s\"\n", tile_labels(my_secret_tiles));
#endif

      discard_tile = 0;
      if (strategy_num == 4)
      // human player
        discard_tile = human_input_discard_tile_label (my_secret_tiles); // if no input, return 0
      if (discard_tile == 0)
        discard_tile = discard_one_tile (my_secret_tiles);

      player_log ("I discard tile %s\n", tile_label[discard_tile]);
      player_log ("my_secret_tiles become \"%s\"\n", tile_labels(my_secret_tiles));
      discard_player = my_player;
      send_discard_msg (discard_player, discard_tile);
      //draw_seat = my_wind;
      } // Chow

    else if (msg[0] == 'T') { // Pung
      if (((struct pung_format *) msg)->pung_player != ('0' + my_player))
        goto OTHER_PUNG_CHOW_WIN;
      if (!propose_pung) {
        fprintf (stderr, "Error not expected to receive T in line %d\n", __LINE__);
        exit (1);
        }
      player_log ("confirm Pung\n");
      add_pung_tiles (my_player, discard_tile);
      if (remove_tile (discard_tile, 2, my_secret_tiles) == 0) {
        fprintf (stderr, "Error in remove_tile()\n");
        exit (1);
        }
#ifdef Chinese
      chinese_display (my_secret_tiles);
#else
      player_log ("my_secret_tiles become \"%s\"\n", tile_labels(my_secret_tiles));
#endif

      discard_tile = 0;
      if (strategy_num == 4)
      // human player
        discard_tile = human_input_discard_tile_label (my_secret_tiles); // if no input, return 0
      if (discard_tile == 0)
        discard_tile = discard_one_tile (my_secret_tiles);

      player_log ("I discard tile %s\n", tile_label[discard_tile]);
      player_log ("my_secret_tiles become \"%s\"\n", tile_labels(my_secret_tiles));
      discard_player = my_player;
      send_discard_msg (discard_player, discard_tile);
      //draw_seat = my_wind;
      }
    else if (msg[0] == 'W') {
      if (((struct win_format *) msg)->win_player != ('0' + my_player))
        goto OTHER_PUNG_CHOW_WIN;
      if (!propose_win) {
        fprintf (stderr, "Error not expected to receive W in line %d\n", __LINE__);
        exit (1);
        }
      player_log ("confirm win\n");
      add_player_tile (my_secret_tiles, discard_tile);
      wait_msg_from_table (my_player, msg);
      if (msg[0] != 'E') { // End Hand
        fprintf (stderr, "Error: Expect E (End Hand) but receive %c\n", msg[0]);
        exit (1);
        }
      player_log ("round %c hand %c ends\n", ((struct end_hand_format *) msg)->round, ((struct end_hand_format *) msg)->hand);
      break;
      }
    }
  else {
    // player_log ("pass\n");
    send_pass_msg (my_player);
    }
  }
wait_msg_from_table (my_player, msg);

OTHER_PUNG_CHOW_WIN:

  if (msg[0] == 'E') { // End Hand
    player_log ("round %c hand %c ends\n", ((struct end_hand_format *) msg)->round, ((struct end_hand_format *) msg)->hand);
    break;
    }
  if (msg[0] == 'W') { // Win
    player_log ("*********************  Player %c wins\n", ((struct win_format *) msg)->win_player);
    wait_msg_from_table (my_player, msg);
    if (msg[0] != 'E') { // End Hand
      fprintf (stderr, "Error: Expect E (End Hand) but receive %c\n", msg[0]);
      exit (1);
      }
    player_log ("round %c hand %c ends\n", ((struct end_hand_format *) msg)->round, ((struct end_hand_format *) msg)->hand);
    break;
    }

  if (msg[0] == 'S') { // Chow (Sequence)
    player_log ("Player %c chow tile %s %s %s\n", ((struct chow_format *) msg)->chow_player,
      tile_label[((struct chow_format *) msg)->chow_tile],
      tile_label[((struct chow_format *) msg)->other_tile1],
      tile_label[((struct chow_format *) msg)->other_tile2]);
    add_chow_tiles (((struct chow_format *) msg)->chow_player - '0',
      ((struct chow_format *) msg)->chow_tile, ((struct chow_format *) msg)->other_tile1,
      ((struct chow_format *) msg)->other_tile2);
    wait_msg_from_table (my_player, msg);
    if (msg[0] != 'C') {
      fprintf (stderr, "Error: Expect C (Discard) but receive %c\n", msg[0]);
      exit (1);
      }
    goto OTHER_DISCARD;
    }

  if (msg[0] == 'T') { // Pung (Triple)
    player_log ("Player %c pung tile %s\n", ((struct pung_format *) msg)->pung_player, tile_label[((struct pung_format *) msg)->tile]);
    add_pung_tiles (((struct pung_format *) msg)->pung_player - '0', ((struct pung_format *) msg)->tile);
    wait_msg_from_table (my_player, msg);
    if (msg[0] != 'C') {
      fprintf (stderr, "Error: Expect C (Discard) but receive %c\n", msg[0]);
      exit (1);
      }
    goto OTHER_DISCARD;
    }

add_open_discard (discard_tile, discard_player);

if (msg[0] != 'X') { // next_drawer_msg
  fprintf (stderr, "Error: Expect X (next) but receive %c\n", msg[0]);
  exit (1);
  }
draw_seat = ((struct next_drawer_format *) msg)->seat; // use table logic to determine next draw seat
player_log ("draw_seat is now %c\n", draw_seat);
} // while (1)
player_log ("Open Meld \"%s\"\n", tile_labels(open_meld[my_player]));
player_log ("Secret tiles \"%s\"\n", tile_labels(my_secret_tiles));
player_log ("Player 1 (%d tiles): %s\n", strlen(open_discard[1]), tile_labels(open_discard[1]));
player_log ("Player 2 (%d tiles): %s\n", strlen(open_discard[2]), tile_labels(open_discard[2]));
player_log ("Player 3 (%d tiles): %s\n", strlen(open_discard[3]), tile_labels(open_discard[3]));
player_log ("Player 4 (%d tiles): %s\n", strlen(open_discard[4]), tile_labels(open_discard[4]));

reply_end_hand_msg (my_player,
 ((struct end_hand_format *) msg)->round,
 ((struct end_hand_format *) msg)->hand,
 my_secret_tiles);

wait_msg_from_table (my_player, msg);
if (msg[0] != 'V') { // Reveal tiles
  fprintf (stderr, "Player %ld Error: Expect V (Reveal) but receive %c\n", my_player, msg[0]);
  exit (1);
  }
int fund1, fund2, fund3, fund4;
if (sscanf ((((struct reveal_format *) msg)->fund1), "%5d%5d%5d%5d", 
&fund1, &fund2, &fund3, &fund4) != 4) {
  fprintf (stderr, "Error sscanf fund != 4\n");
  exit (1);
  }
player_log ("Player 1 fund %5d\n", fund1);
player_log ("Player 2 fund %5d\n", fund2);
player_log ("Player 3 fund %5d\n", fund3);
player_log ("Player 4 fund %5d\n", fund4);
char reveal1[20];
char reveal2[20];
char reveal3[20];
char reveal4[20];
if (sscanf ((((struct reveal_format *) msg)->reveal), "%s %s %s %s", 
reveal1, reveal2, reveal3, reveal4) != 4) {
  fprintf (stderr, "Error sscanf reveal != 4\n");
  exit (1);
  }
player_log ("Player 1 reveals tiles \"%s\"\n",  tile_labels(reveal1));
player_log ("Player 2 reveals tiles \"%s\"\n",  tile_labels(reveal2));
player_log ("Player 3 reveals tiles \"%s\"\n",  tile_labels(reveal3));
player_log ("Player 4 reveals tiles \"%s\"\n",  tile_labels(reveal4));

} // while (hand)
  
return 0;

} // main()
