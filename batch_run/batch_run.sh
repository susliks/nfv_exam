#!/bin/bash

for i in `seq 19`
do
    echo $i
    cd batch_nfv_exam/nfv_exam_$i
    sh run_$i.sh
    cd ../..
done
