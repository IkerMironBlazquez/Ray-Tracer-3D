#!/bin/sh
#SBATCH --job-name=auto_t04
#SBATCH --output=./logs/auto_04.txt
#SBATCH --error=./logs/auto_04.err
#SBATCH -p stan
#SBATCH --time=00:05:00
set -Eeu pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p logs
export TBB_NUM_THREADS=4
EXEC="../build/par/render-par-auto"
CONFIG="../scenes/config5.txt"
SCENE="../scenes/scene5.txt"
OUTPUT="./logs/output_auto_04.ppm"
echo "=== AUTO PARTITIONER - 4 THREADS ==="
echo "--- Grain: 64 ---"
perf stat -e power/energy-pkg/ $EXEC $CONFIG $SCENE $OUTPUT 0 64 64 2>&1
echo "Tests completed"
