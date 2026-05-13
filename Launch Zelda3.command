#!/bin/bash
cd "$(dirname "$0")"
nohup ./zelda3 >/dev/null 2>&1 &
osascript -e 'tell application "Terminal" to close first window' &>/dev/null
