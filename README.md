# pi_mahjong

I learned the mahjong game months ago.  Like other knowledge, the best way to reinforce the understanding to teach others.  So I taught my Raspberry Pi computer to play this game.  I used the C language to realize the algorithm. 

There are mainly two types of instances:

- player (four instances representing four players)
- table (many others call "wall")

The player intance can be started by:
player player_number(1-4) strategy_number(0-4)

The strategy number can be:

0 : purely random (just for benmarking)

1 : Bot Algorithm for Mahjong Hong Kong MP0 (http://mahjongjoy.com/mahjong_bots_ai_algorithm.shtml)

2 : Bot Algorithm for Mahjong Hong Kong 1 < MP <= 8 (1, 2, 3 Fan) (ditto)

3 : bin algorithm

4 : human player
