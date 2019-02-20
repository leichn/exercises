#!/bin/bash
gcc -o ffplayer ffplayer.c -lavutil -lavformat -lavcodec -lavutil -lswscale -lSDL2
