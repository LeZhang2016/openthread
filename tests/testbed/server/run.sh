#/bin/bash!
export DC_MODE_3=1

for ((i=0; i<5; ++i))  
do
    python test_throughput.py
done
