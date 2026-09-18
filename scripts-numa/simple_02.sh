#!/bin/sh
#SBATCH --job-name=simple_t02
#SBATCH --output=./logs/simple_02.txt
#SBATCH --error=./logs/simple_02.err
#SBATCH -p stan
#SBATCH --time=00:05:00
set -Eeu pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p logs
export TBB_NUM_THREADS=2
EXEC="../build/par/render-par-simple"
CONFIG="../scenes/config5.txt"
SCENE="../scenes/scene5.txt"
OUTPUT="./logs/output_simple_02.ppm"
echo "=== SIMPLE PARTITIONER - 2 THREADS ==="
echo "--- Grain: 64 (demo que 2 hilos no es eficiente) ---"
perf stat -e power/energy-pkg/ $EXEC $CONFIG $SCENE $OUTPUT 0 64 64 2>&1
echo "Tests completed"
