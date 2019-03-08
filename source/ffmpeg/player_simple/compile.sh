#!/bin/bash
gcc -o ffplayer ffplayer.c -lavformat -lavcodec -lavutil -lswscale -lSDL2
