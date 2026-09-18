#!/bin/sh
#SBATCH --job-name=static_t16
#SBATCH --output=./logs/static_16.txt
#SBATCH --error=./logs/static_16.err
#SBATCH -p stan
#SBATCH --time=00:05:00
set -Eeu pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p logs
export TBB_NUM_THREADS=16
EXEC="../build/par/render-par-static"
CONFIG="../scenes/config5.txt"
SCENE="../scenes/scene5.txt"
OUTPUT="./logs/output_static_16.ppm"
echo "=== STATIC PARTITIONER - 16 THREADS ==="
for GRAIN in 4 16 64 256; do
  echo "--- Grain: $GRAIN ---"
  perf stat -e power/energy-pkg/ $EXEC $CONFIG $SCENE $OUTPUT 0 $GRAIN $GRAIN 2>&1
  echo ""
done
echo "Tests completed"
