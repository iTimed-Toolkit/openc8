#!/bin/bash

#SBATCH -J correlate
#SBATCH -o corr.%j.out
#SBATCH -N 8
#SBATCH -n 8
#SBATCH -t 01:30:00
#SBATCH -p broadwell

NODEFILE=nodefile.txt
rank=0

echo $SLURM_NODELIST | tr -d c | tr -d [ | tr -d ] | perl -pe 's/(\d+)-(\d+)/join(",",$1..$2)/eg' | awk 'BEGIN { RS=","} { print "c"$1 }' > $NODEFILE

for node in 'cat $NODEFILE'; do
  ssh -n $node "mkdir /tmp/ghaas/ && tar -xf data00.tar.gz -C /tmp/ghaas" & pid[$rank]=$!
  (( rank++ ))
done

rank=0
for node in 'cat $NODEFILE'; do
  wait ${pid[$rank]}
  rank++
done

rm $NODEFILE

prun # todo: whatever the name is

#todo: delete tmp files