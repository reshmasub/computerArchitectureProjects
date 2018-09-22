#!/bin/bash

#1024, 2048 4096, 8192
for i in {0..5}
do
  for j in {0..3}
  do
    for k in {1..6}
    do
      let line=(2**$k)
      let a=(2**$j)
      let cs=(2**$i)*1024
      echo "./main $cs $line $a"
    done
  done
done
