#!/bin/bash
gcc -o ffmpeg_vfilter -g ffmpeg_vfilter.c -lavutil -lavformat -lavcodec -lavutil -lswscale -lavfilter -lSDL2
