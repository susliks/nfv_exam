#!/bin/bash

mkdir total_result
cp batch_nfv_exam/nfv_exam_0/data/final_result_0 total_result
for i in `seq 19`
do
    echo $i
    cp batch_nfv_exam/nfv_exam_$i/data/final_result_$i total_result
done
