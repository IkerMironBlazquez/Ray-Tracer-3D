#!/bin/sh
#SBATCH --job-name=simple_t256
#SBATCH --output=./logs/simple_256.txt
#SBATCH --error=./logs/simple_256.err
#SBATCH -p stan
#SBATCH --time=00:05:00
set -Eeu pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p logs
export TBB_NUM_THREADS=256
EXEC="../build/par/render-par-simple"
CONFIG="../scenes/config5.txt"
SCENE="../scenes/scene5.txt"
OUTPUT="./logs/output_simple_256.ppm"
echo "=== SIMPLE PARTITIONER - 256 THREADS ==="
for GRAIN in 1 4 16 64 256; do
  echo "--- Grain: $GRAIN ---"
  perf stat -e power/energy-pkg/ $EXEC $CONFIG $SCENE $OUTPUT 0 $GRAIN $GRAIN 2>&1
  echo ""
done
echo "Tests completed"
