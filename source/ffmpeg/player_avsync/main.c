#include <stdio.h>

#include "player.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Please provide a movie file, usage: \n");
        printf("./ffplayer ring.mp4\n");
        return -1;
    }
    printf("Try playing %s ...\n", argv[1]);
    player_running(argv[1]);

    return 0;
}
