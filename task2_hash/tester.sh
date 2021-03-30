#!/bin/bash 


for i in {1..16};
do  

    input=$(./hash $i |  sed -n 1p)
    hash=$(./hash $i |  sed -n 2p)
    expected=$( echo -n "$input" | xxd -r -ps | openssl sha384 | sed 's/(stdin)= //g')
    echo "test $i" 
    echo "input $input" 
    echo "OUTPUT                                EXPECTED"
    result=$(diff <(echo "$hash") <(echo "$expected"))
    echo $result    
done
