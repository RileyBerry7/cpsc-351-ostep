#!/usr/bin/env bash

gcc msg_queue.c -lrt -o msg_queue
ln -sf msg_queue recv
ln -sf msg_queue sender
