#!/usr/bin/env bash

gcc msg_queue.c -o msg_queue
ln -sf msg_queue rcvr
ln -sf msg_queue sndr
