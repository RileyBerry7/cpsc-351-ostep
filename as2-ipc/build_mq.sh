#!/usr/bin/env bash

gcc msg_queue.c -lrt -o msg_queue
ln -sf msg_queue rcvr
ln -sf msg_queue sndr
