#!/usr/bin/env bash

gcc -Wall -Wextra -O2 -std=c17 -o sender sender.c
gcc -Wall -Wextra -O2 -std=c17 -o recv recv.c
