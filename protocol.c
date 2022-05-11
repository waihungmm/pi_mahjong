#include "pi_mahjong.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

int qid2table;
int qid2player;

// =========================================
int flush_mq(void)
{
int rtn;
PI_MAHJONG_MSG_STRUCT msg;
do {
  rtn = msgrcv(qid2table, (void *) &msg, PI_MAHJONG_MSG_LEN, 0, IPC_NOWAIT);
  if (rtn == -1) {
    if (errno == ENOMSG)
      player_log ("Inside %s on queue to table, no more message\n", __func__);
    else {
      fprintf (stderr, "Error in %s, msgrcv has errno = %d\n", __func__, errno);
      return 0;
      }
    }
  else {
    player_log ("Inside %s on queue to table.. ", __func__);
    msg_log (0L, 'R', msg.mtext);
    // printf ("Inside flush_mq, msgrcv (on qid2table) gets %ld %s\n", msg.mtype, msg.mtext);
    }
  } while (rtn != -1);

do {
  rtn = msgrcv(qid2player, (void *) &msg, PI_MAHJONG_MSG_LEN, 0, IPC_NOWAIT);
  if (rtn == -1) {
    if (errno == ENOMSG)
      player_log ("Inside %s on queue to player, no more message\n", __func__);
    else {
      fprintf (stderr, "Error in %s, msgrcv has errno = %d\n", __func__, errno);
      return 0;
      }
    }
  else {
    player_log ("Inside %s on queue to player.. ", __func__);
    msg_log (0L, 'R', msg.mtext);
//, msgrcv (on qid2table) gets %ld %s\n", msg.mtype, msg.mtext);
    }
  } while (rtn != -1);

} // flush_mq()

// =========================================
void remove_mq ()
{
if (msgctl(qid2table, IPC_RMID, NULL) != 0) {
  printf ("msgctl (qid2table) returns %d\n", errno);
  exit (EXIT_FAILURE);
  }

if (msgctl(qid2player, IPC_RMID, NULL) != 0) {
  printf ("msgctl (qid2table) returns %d\n", errno);
  exit (EXIT_FAILURE);
  }
} // remove_mq()

// =========================================
int send_msg_to_table (long player, char *msg_text, int size)
{
int rtn;
PI_MAHJONG_MSG_STRUCT msg;

msg.mtype = player;
memset (msg.mtext, 0, PI_MAHJONG_MSG_LEN); // initialize to null
memcpy (msg.mtext, msg_text, size);
// printf ("inside %s, size = %d, msg.mtext = %s\n", __func__, size, msg.mtext);

rtn = msgsnd(qid2table, (void *) &msg, PI_MAHJONG_MSG_LEN, IPC_NOWAIT);
if (rtn == -1) {
  fprintf (stderr, "msgsnd error\n");
  return 0;
  }
msg_log (player, 'S', msg.mtext);
return 1;
} // send_msg_to_table()

// =========================================
/*
int wait_msg_from_table(long player, char *msg_text)
{
int rtn;
PI_MAHJONG_MSG_STRUCT msg;

int sleep_cnt = 600; // 60 seconds
while (sleep_cnt > 0) {
  rtn = msgrcv(qid2player, (void *) &msg, PI_MAHJONG_MSG_LEN, player, IPC_NOWAIT);
  if (rtn != -1)
    break;
  if (errno != ENOMSG) {
    fprintf (stderr, "Error in %s msgrcv error %d\n", __func__, errno);
    exit (1);
    }
  usleep (10000); // 0.1 second
  sleep_cnt--;
  } // while
// printf ("sleep_cnt = %d\n", sleep_cnt);
if (sleep_cnt == 0) {
  fprintf (stderr, "Error in %s msgrcv timeout\n", __func__);
  exit (1);
  }
//printf ("msgrcv gets %ld %s\n", msg.mtype, msg.mtext);
msg_log (msg.mtype, 'R', msg.mtext);

// cross check
if (msg.mtype != player) {
  printf ("Error: msg.mtype != player\n");
  exit (1);
  }
strncpy (msg_text, msg.mtext, PI_MAHJONG_MSG_LEN);
return 1;
} // wait_msg_from_table()
*/

// =========================================
int wait_msg_from_table(long player, char *msg_text)
{
int rtn;
PI_MAHJONG_MSG_STRUCT msg;

rtn = msgrcv(qid2player, (void *) &msg, PI_MAHJONG_MSG_LEN, player, 0);
if (rtn == -1) {
   printf ("Error in %s error %d\n", __func__, errno);
   exit (1);
  }

// printf ("msgrcv gets %ld %s\n", msg.mtype, msg.mtext);
msg_log (msg.mtype, 'R', msg.mtext);

// cross check
if (msg.mtype != player) {
  printf ("Error: msg.mtype != player\n");
  exit (1);
  }
strncpy (msg_text, msg.mtext, PI_MAHJONG_MSG_LEN);
return 1;
} // wait_msg_from_table()

// =========================================
int send_msg_to_player (long player, char *msg_text, int size)
{
int rtn;
PI_MAHJONG_MSG_STRUCT msg;

msg.mtype = player;
memset (msg.mtext, 0, PI_MAHJONG_MSG_LEN); // initialize to null
memcpy (msg.mtext, msg_text, size);

rtn = msgsnd(qid2player, (void *) &msg, PI_MAHJONG_MSG_LEN, IPC_NOWAIT);
if (rtn == -1) {
  fprintf (stderr, "Error: msgsnd error\n");
  exit (1);
  }
msg_log (player, 'S', msg.mtext);
// printf ("message sent [%.*s]\n", size, msg.mtext);
// printf ("size = %d, strlen = %d\n", size, strlen (msg.mtext));
return 1;
} // send_msg_to_player()

// =========================================
/*
long get_msg_from_player(char *msg_text)
{ // return player number if successful; 0 if no message; -1 if error
int rtn;
long player;
PI_MAHJONG_MSG_STRUCT msg;

rtn = msgrcv(qid2table, (void *) &msg, PI_MAHJONG_MSG_LEN, 0, IPC_NOWAIT);
if (rtn == -1) {
  if (errno != ENOMSG)
    printf ("msgrcv error\n");
  return -1;
  }
// printf ("msgrcv gets %ld %s\n", msg.mtype, msg.mtext);
// msg_log (0L, 'R', msg.mtext);
player = msg.mtype;
msg_log (player, 'R', msg.mtext);
if (!((player >=1) && (player <=4))) {
  printf ("Error: player is not 1-4\n");
  exit (1);
  }
strncpy (msg_text, msg.mtext, PI_MAHJONG_MSG_LEN);
return player;
} // get_msg_from_player()
*/

// =========================================
long wait_msg_from_player(char *msg_text)
{ // return player number if successful
int rtn;
long player;
PI_MAHJONG_MSG_STRUCT msg;

rtn = msgrcv(qid2table, (void *) &msg, PI_MAHJONG_MSG_LEN, 0, 0);
if (rtn == -1) {
   printf ("Error in %s error %d\n", __func__, errno);
   exit (1);
  }
// printf ("msgrcv gets %ld %s\n", msg.mtype, msg.mtext);
// msg_log (0L, 'R', msg.mtext);
player = msg.mtype;
msg_log (player, 'R', msg.mtext);
if (!((player >=1) && (player <=4))) {
msg_log (0L, 'R', msg.mtext);
  printf ("Error: player is not 1-4\n");
  exit (1);
  }
strncpy (msg_text, msg.mtext, PI_MAHJONG_MSG_LEN);
return player;
} // wait_msg_from_player()

// =========================================


/* message spec
Message    msg_code Direction    
Register   R        Player->Table
New Game   N        Table->Player
New Hand   H        Table->Player
Draw Tile  D        Table->Player
Discard Tile  C     Player<->Table
Pass       P        Player->Table
Chow       S        Player<->Table
Pung       T        Player<->Table
Win        W        Player<->Table
Next Drawer X       Table->Player
End Hand   E        Table->Player
End Game   G        Table->Player
Reveal Tile V       Table->Player
*/

// =========================================

int send_register_msg (long player, int fund)
{
if (fund > 99999) {
  fprintf (stderr, "Error, initial_fund > 99999\n");
  exit (1);
  }
register_msg.code = 'R';           // msg_code
sprintf (register_msg.initial_fund, "%05d", fund);
send_msg_to_table (player, (char *) &register_msg, sizeof (register_msg));
} // send_register_msg()

// =========================================

int send_new_game_msg (long player, char player_initial_seat)
{
struct new_game_format new_game_msg;
new_game_msg.code = 'N';           // msg_code
//new_game_msg.from = '0';           // from table
//new_game_msg.to   = '0' + player;  // to player
new_game_msg.seat = player_initial_seat;
send_msg_to_player (player, (char *) &new_game_msg, sizeof (new_game_msg));
}

// =========================================

int send_new_hand_msg (long player, char new_round, char new_hand, char player1_seat, char player2_seat, char player3_seat, char player4_seat, char *tiles)
{
struct new_hand_format new_hand_msg;
new_hand_msg.code = 'H';           // msg_code
// new_hand_msg.from = '0';           // from table
// new_hand_msg.to   = '0' + player;  // to player
new_hand_msg.round = new_round;
new_hand_msg.hand = new_hand;
new_hand_msg.seats[0] = player1_seat;
new_hand_msg.seats[1] = player2_seat;
new_hand_msg.seats[2] = player3_seat;
new_hand_msg.seats[3] = player4_seat;
memcpy (new_hand_msg.tiles, tiles, 13);
send_msg_to_player (player, (char *) &new_hand_msg, sizeof(new_hand_msg));
}

// =========================================

int send_draw_tile_msg (long player, char tile)
{
struct draw_tile_format draw_tile_msg;
draw_tile_msg.code = 'D';           // msg_code
//draw_tile_msg.from = '0';           // from table
//draw_tile_msg.to   = '0' + player;  // to player
draw_tile_msg.tile = tile;
send_msg_to_player (player, (char *) &draw_tile_msg, sizeof (draw_tile_msg));
}


int send_discard_msg (long player, char tile)
{
struct discard_format discard_msg;
discard_msg.code = 'C';           // msg_code
// discard_msg.from = '0' + player;
// discard_msg.to   = '0';           // to table
discard_msg.discard_player = '0' + player;
discard_msg.discard_tile = tile;
// printf ("sizeof (discard_msg) = %d \n", sizeof(discard_msg));
send_msg_to_table (player, (char *) &discard_msg, sizeof (discard_msg));
}

// =========================================

int relay_discard_msg (long discard_player, char tile)
{
struct discard_format discard_msg;
discard_msg.code = 'C';           // msg_code
//discard_msg.from = '0';           // to table
discard_msg.discard_player = '0' + discard_player;
discard_msg.discard_tile = tile;
long i;
for (i=1; i<=4; i++) {
  if (i != discard_player) {
//    discard_msg.to   = '0' + i;
    send_msg_to_player (i, (char *) &discard_msg, sizeof (discard_msg));
    }
  }
}

// =========================================

int send_pass_msg (long player)
{
struct pass_format pass_msg;
pass_msg.code = 'P';           // msg_code
//pass_msg.from = '0' + player;
//pass_msg.to   = '0';           // to table
send_msg_to_table (player, (char *) &pass_msg, sizeof (pass_msg));
}

// =========================================

int send_win_msg (long player)
{ // player declare win but need table confirmation
struct win_format win_msg;
win_msg.code = 'W';
//win_msg.from = '0' + player;
//win_msg.to   = '0';           // to table
win_msg.win_player  = '0' + player;
send_msg_to_table (player, (char *) &win_msg, sizeof (win_msg));
}

// =========================================

int broadcast_win_msg (long win_player)
{
struct win_format win_msg;
win_msg.code = 'W';
//win_msg.from = '0';
win_msg.win_player  = '0' + win_player;
long player;
for (player=1; player<=4; player++) {
//  win_msg.to = '0' + player;
  send_msg_to_player (player, (char *) &win_msg, sizeof (win_msg));
  }
}

// =========================================

int broadcast_reveal_msg (int *fund, char *reveal1, char *reveal2, char *reveal3, char *reveal4)
{
struct reveal_format reveal_msg;
reveal_msg.code = 'V';
char tmp[6];
sprintf (tmp, "%05d", fund[1]); memcpy (reveal_msg.fund1, tmp, 5);
sprintf (tmp, "%05d", fund[2]); memcpy (reveal_msg.fund2, tmp, 5);
sprintf (tmp, "%05d", fund[3]); memcpy (reveal_msg.fund3, tmp, 5);
sprintf (tmp, "%05d", fund[4]); memcpy (reveal_msg.fund4, tmp, 5);
memset (reveal_msg.reveal, 0 , sizeof (reveal_msg.reveal));
sprintf (reveal_msg.reveal, "%s %s %s %s", reveal1, reveal2, reveal3, reveal4);
long player;
for (player=1; player<=4; player++) {
  send_msg_to_player (player, (char *) &reveal_msg, sizeof (reveal_msg));
  }
} // broadcast_reveal_msg()

// =========================================

int send_chow_msg (long player, char chow_tile, char other_tile1, char other_tile2)
{ // player declare chow but need table confirmation
struct chow_format chow_msg;
chow_msg.code = 'S'; // sequence
chow_msg.chow_player  = '0' + player;
chow_msg.chow_tile = chow_tile;
chow_msg.other_tile1 = other_tile1;
chow_msg.other_tile2 = other_tile2;
send_msg_to_table (player, (char *) &chow_msg, sizeof (chow_msg));
}

// =========================================

int broadcast_chow_msg (long chow_player, char chow_tile, char other_tile1, char other_tile2)
{
struct chow_format chow_msg;
chow_msg.code   = 'S';
chow_msg.chow_player = '0' + chow_player;
chow_msg.chow_tile   = chow_tile;
chow_msg.other_tile1 = other_tile1;
chow_msg.other_tile2 = other_tile2;
long player;
for (player=1; player<=4; player++) {
  send_msg_to_player (player, (char *) &chow_msg, sizeof (chow_msg));
  }
}

// =========================================

int send_pung_msg (long player, char pung_tile)
{ // player declare pung but need table confirmation
struct pung_format pung_msg;
pung_msg.code = 'T'; // triple
//pung_msg.from = '0' + player;
//pung_msg.to   = '0';           // to table
pung_msg.pung_player  = '0' + player;
pung_msg.tile = pung_tile;
send_msg_to_table (player, (char *) &pung_msg, sizeof (pung_msg));
}

// =========================================

int broadcast_pung_msg (long pung_player, char pung_tile)
{
struct pung_format pung_msg;
pung_msg.code   = 'T'; // triple
//pung_msg.from   = '0';
pung_msg.pung_player = '0' + pung_player;
pung_msg.tile   = pung_tile;
long player;
for (player=1; player<=4; player++) {
//  pung_msg.to = '0' + player;
  send_msg_to_player (player, (char *) &pung_msg, sizeof (pung_msg));
  }
}

// =========================================

int broadcast_next_drawer_msg (char seat)
{
struct next_drawer_format next_drawer_msg;
next_drawer_msg.code = 'X';
//next_drawer_msg.from = '0';
next_drawer_msg.seat = seat;
long player;
for (player=1; player<=4; player++) {
//  next_drawer_msg.to = '0' + player;
  send_msg_to_player (player, (char *) &next_drawer_msg, sizeof (next_drawer_msg));
  }
}

// =========================================

int broadcast_end_hand_msg (char round, char hand)
{ // from table to players
struct end_hand_format end_hand_msg;
end_hand_msg.code = 'E';
//end_hand_msg.from = '0';
end_hand_msg.round = round;
end_hand_msg.hand = hand;
memset (end_hand_msg.final_revealed_tiles, 0, sizeof(end_hand_msg.final_revealed_tiles));
long player;
for (player=1; player<=4; player++) {
//  end_hand_msg.to = '0' + player;
  send_msg_to_player (player, (char *) &end_hand_msg, sizeof (end_hand_msg));
  }
}

// =========================================

int reply_end_hand_msg (long player, char round, char hand, char *tiles)
{ // from players to table
struct end_hand_format end_hand_msg;
if (strlen (tiles) > sizeof(end_hand_msg.final_revealed_tiles) - 1) {
  fprintf (stderr, "Error tiles too long (%s)\n", tiles);
  exit (1);
  }
end_hand_msg.code = 'E';
//end_hand_msg.from = '0' + player;
//end_hand_msg.to   = '0';           // to table
end_hand_msg.round = round;
end_hand_msg.hand = hand;
strcpy (end_hand_msg.final_revealed_tiles, tiles);
send_msg_to_table (player, (char *) &end_hand_msg, sizeof (end_hand_msg));
}

// =========================================

int broadcast_end_game_msg (int total_round)
{ // from table to players
struct end_game_format end_game_msg;
end_game_msg.code = 'G';
end_game_msg.from = '0';
if (total_round > 999) {
  fprintf (stderr, "Error in %s, total_round (%d) > 999\n", __func__, total_round);
  exit (1);
  }
sprintf (end_game_msg.total_round, "%03d", total_round);
long player;
for (player=1; player<=4; player++) {
  end_game_msg.to = '0' + player;
  send_msg_to_player (player, (char *) &end_game_msg, sizeof (end_game_msg));
  }
}