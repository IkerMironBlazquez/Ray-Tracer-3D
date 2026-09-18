#!/bin/sh
#SBATCH --job-name=static_t02
#SBATCH --output=./logs/static_02.txt
#SBATCH --error=./logs/static_02.err
#SBATCH -p stan
#SBATCH --time=00:05:00
set -Eeu pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p logs
export TBB_NUM_THREADS=2
EXEC="../build/par/render-par-static"
CONFIG="../scenes/config5.txt"
SCENE="../scenes/scene5.txt"
OUTPUT="./logs/output_static_02.ppm"
echo "=== STATIC PARTITIONER - 2 THREADS ==="
echo "--- Grain: 64 ---"
perf stat -e power/energy-pkg/ $EXEC $CONFIG $SCENE $OUTPUT 0 64 64 2>&1
echo "Tests completed"
