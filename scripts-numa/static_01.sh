#!/bin/sh
#SBATCH --job-name=static_t01
#SBATCH --output=./logs/static_01.txt
#SBATCH --error=./logs/static_01.err
#SBATCH -p stan
#SBATCH --time=00:05:00
set -Eeu pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p logs
export TBB_NUM_THREADS=1
EXEC="../build/par/render-par-static"
CONFIG="../scenes/config5.txt"
SCENE="../scenes/scene5.txt"
OUTPUT="./logs/output_static_01.ppm"
echo "=== STATIC PARTITIONER - 1 THREAD ==="
echo "--- Grain: 64 ---"
perf stat -e power/energy-pkg/ $EXEC $CONFIG $SCENE $OUTPUT 0 64 64 2>&1
echo "Tests completed"
