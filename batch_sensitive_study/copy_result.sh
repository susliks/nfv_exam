#!/bin/bash

mkdir total_result
cp batch_sensitive_study/sensitive_study_0/data/final_result_sensitive_0 total_result
for i in `seq 10`
do
    echo $i
    cp batch_sensitive_study/sensitive_study_$i/data/final_result_sensitive_$i total_result
done
