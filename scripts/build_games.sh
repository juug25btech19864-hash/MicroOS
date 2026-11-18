#!/bin/bash
# MicroOS Game Compilation Pipeline
# Compiles all 8 offline arcade games and the arcade launcher into stripped, optimized C binaries

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJ_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_GAMES_DIR="$PROJ_DIR/build/games"

mkdir -p "$BUILD_GAMES_DIR"

CC="${CC:-gcc}"
CFLAGS="-O2 -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE -D_GNU_SOURCE"
LDFLAGS="-s"

echo "=== Compiling MicroOS Arcade Games with $CC ==="

GAMES=("snake" "tetris" "pong" "mines" "2048" "guess" "tictactoe" "spaceinvaders")

for g in "${GAMES[@]}"; do
    src="$PROJ_DIR/games/$g.c"
    out="$BUILD_GAMES_DIR/$g"
    echo "  [CC] $g.c -> $out"
    $CC $CFLAGS "$src" -o "$out" $LDFLAGS
    strip --strip-all "$out" 2>/dev/null || true
done

# Compile arcade launcher
echo "  [CC] arcade.c -> $BUILD_GAMES_DIR/arcade"
$CC $CFLAGS "$PROJ_DIR/arcade/arcade.c" -o "$BUILD_GAMES_DIR/arcade" $LDFLAGS
strip --strip-all "$BUILD_GAMES_DIR/arcade" 2>/dev/null || true

echo "=== Arcade Binaries Compiled Successfully ==="
ls -lh "$BUILD_GAMES_DIR"
