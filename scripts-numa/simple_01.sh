#!/bin/sh
#SBATCH --job-name=simple_t01
#SBATCH --output=./logs/simple_01.txt
#SBATCH --error=./logs/simple_01.err
#SBATCH -p stan
#SBATCH --time=00:05:00
set -Eeu pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p logs
export TBB_NUM_THREADS=1
EXEC="../build/par/render-par-simple"
CONFIG="../scenes/config5.txt"
SCENE="../scenes/scene5.txt"
OUTPUT="./logs/output_simple_01.ppm"
echo "=== SIMPLE PARTITIONER - 1 THREAD ==="
echo "--- Grain: 64 ---"
perf stat -e power/energy-pkg/ $EXEC $CONFIG $SCENE $OUTPUT 0 64 64 2>&1
echo "Tests completed"
