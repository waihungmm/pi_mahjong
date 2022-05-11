all: table player pi_mahjong.h cplayer

table: table.c pi_mahjong.h utilities.c protocol.c strategy.c
	gcc -o table table.c utilities.c protocol.c strategy.c

player: player.c pi_mahjong.h utilities.c protocol.c strategy.c
	gcc -o player player.c utilities.c protocol.c strategy.c

cplayer: player.c pi_mahjong.h define_tile_labels_c.c utilities.c protocol.c strategy.c
	gcc -DChinese -o cplayer player.c define_tile_labels_c.c utilities.c protocol.c strategy.c
