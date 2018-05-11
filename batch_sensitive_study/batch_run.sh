#!/bin/bash

cd batch_sensitive_study/sensitive_study_0
sh run_0.sh
cd ../..
for i in `seq 10`
do
    echo $i
    cd batch_sensitive_study/sensitive_study_$i
    sh run_$i.sh
    cd ../..
done
