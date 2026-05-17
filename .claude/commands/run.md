#!/usr/bin/env bash
# Launch the game
if [ ! -f ./zelda3 ]; then
  echo "Binary not found. Run /build first."
  exit 1
fi
exec ./zelda3
