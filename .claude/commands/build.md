#!/usr/bin/env bash
# Build the game with parallel jobs
set -e
make -j$(sysctl -n hw.logicalcpu 2>/dev/null || nproc 2>/dev/null || echo 4)
echo "Build succeeded."
