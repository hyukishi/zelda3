#!/usr/bin/env bash
# Clean and rebuild the game
set -e
make clean all -j$(sysctl -n hw.logicalcpu 2>/dev/null || nproc 2>/dev/null || echo 4)
echo "Clean build succeeded."
