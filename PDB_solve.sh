#!/bin/bash
#SBATCH --job-name=puppet_main
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=24000
#SBATCH --gres=gpu:v100:1
#SBATCH --time=24:00:00
#SBATCH --output=outs/%j_puppetPDB.out

source /cluster/$USER/venvs/puppet-dca/bin/activate

cd ~/puppet-cube-v1

echo "Node:"
hostname

make -j"$SLURM_CPUS_PER_TASK"

echo
echo "Run..."
#./Main > output_${SLURM_JOB_ID}.txt
./Main

echo
echo "Finished."