#include "pi_mahjong.h"

char * tile_label_c[127];

void define_tile_labels_c ()
{
tile_label_c[' ']       = ".";  // spacer
tile_label_c[ONE_DOT]   = "一筒";
tile_label_c[TWO_DOT]   = "二筒";
tile_label_c[THREE_DOT] = "三筒";
tile_label_c[FOUR_DOT]  = "四筒";
tile_label_c[FIVE_DOT]  = "五筒";
tile_label_c[SIX_DOT]   = "六筒";
tile_label_c[SEVEN_DOT] = "七筒";
tile_label_c[EIGHT_DOT] = "八筒";
tile_label_c[NINE_DOT]  = "九筒";

tile_label_c[ONE_BAMBOO]   = "一索";
tile_label_c[TWO_BAMBOO]   = "二索";
tile_label_c[THREE_BAMBOO] = "三索";
tile_label_c[FOUR_BAMBOO]  = "四索";
tile_label_c[FIVE_BAMBOO]  = "五索";
tile_label_c[SIX_BAMBOO]   = "六索";
tile_label_c[SEVEN_BAMBOO] = "七索";
tile_label_c[EIGHT_BAMBOO] = "八索";
tile_label_c[NINE_BAMBOO]  = "九索";


tile_label_c[ONE_CHARACTER]   = "一萬";
tile_label_c[TWO_CHARACTER]   = "二萬";
tile_label_c[THREE_CHARACTER] = "三萬";
tile_label_c[FOUR_CHARACTER]  = "四萬";
tile_label_c[FIVE_CHARACTER]  = "五萬";
tile_label_c[SIX_CHARACTER]   = "六萬";
tile_label_c[SEVEN_CHARACTER] = "七萬";
tile_label_c[EIGHT_CHARACTER] = "八萬";
tile_label_c[NINE_CHARACTER]  = "九萬";

tile_label_c[EAST_WIND]  = "東風";
tile_label_c[SOUTH_WIND] = "南風";
tile_label_c[WEST_WIND]  = "西風";
tile_label_c[NORTH_WIND] = "北風";

tile_label_c[RED_DRAGON]  = "紅中"; 
tile_label_c[GREEN_DRAGON] = "發財";
tile_label_c[WHITE_DRAGON]  = "白板";
} // define_tile_labels_c ()

