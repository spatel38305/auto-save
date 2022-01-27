#!/usr/bin/env bash
gcc -o auto-save.so auto-save.c $(yed --print-cflags) $(yed --print-ldflags)
