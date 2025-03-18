#!/bin/bash

set -x  # Enable verbose output for debugging

# Start the Docker container
echo "Starting Docker container..."
docker start cse220_rathee || { echo "Failed to start Docker container."; exit 1; }

# Build Scarab inside the Docker container
echo "Building Scarab..."
docker exec \
  --env USER=$(whoami) \
  --user $(whoami) \
  --workdir /home/$(whoami)/scarab/src \
  cse220_rathee \
  make opt || { echo "Failed to build Scarab."; exit 1; }

echo "Cleanup!"
# Base directory
BASE_DIR="/home/rathee/cse220_home/exp/simulations"

# Find and delete 'bp_test' directories under the specific top-level directories
for dir in "${BASE_DIR}/500.perlbench_r" "${BASE_DIR}/502.gcc_r"; do
    if [[ -d "${dir}/bp_test" ]]; then
        echo "Deleting ${dir}/bp_test..."
        rm -rf "${dir}/bp_test"
    else
        echo "No bp_test directory found in ${dir}."
    fi
done

echo "Cleanup complete."

echo "Simulations"
cd ~/Scarab-infra
./run.sh -o /Users/$USER/cse220_home -s 220 -e bp_test 
sleep 300

# Run the Python script on the host
echo "Generating plots..."
cd ~/cse220_home/scarab
python3 ~/plot_benchmarks.py -d ~/cse220_home/exp/simulations/ -c ~/cse220_home/bp_test.json || { echo "Failed to generate plots."; exit 1; }

set +x  # Disable verbose output
