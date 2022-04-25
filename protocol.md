Like all computer program, besides **the** algorithm, the _data structure_ and _protocol_ are important to make the problem work.  So when I decide the mahjong game, I have spent some time to device what is the best protocol.

Mahjong is a four-player gamer.  So there are five (5) entities, 4 players as well as the table (most people called the wall).  Player can communicate to table and vice versa.  But in my design, if player needs to communicate one another, it should be via the table entity.

I use Posix Message queue (one from table to player and another from player to table) for the communication channels.

The following are the list of messages

| Message    | msg_code | Direction | 
| -----------| ---------| --------------| 
| Register   | R        | Player->Table | 
| New Game   | N        | Table->Player | 
| New Hand   | H        | Table->Player | 
| Draw Tile  | D        | Table->Player | 
| Discard Tile |  C     | Player<->Table | 
| Pass       | P        | Player->Table | 
| Chow       | S        | Player<->Table | 
| Pung       | T        | Player<->Table | 
| Win        | W        | Player<->Table | 
| Next Drawer | X       | Table->Player | 
| End Hand   | E        | Table->Player | 
| End Game   | G        | Table->Player | 
| Reveal Tile | V       | Table->Player | 

The following are the detailed format

```
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
//  char from;
//  char to;
  char seat;
  };
  
struct end_hand_format {
  char code;
  char round;
  char hand;
  char final_revealed_tiles[19];
  };

struct end_game_format {
  char code;
  char from;
  char to;
  char total_round[4];
  };
```
  

  
  
  
  
  

  
