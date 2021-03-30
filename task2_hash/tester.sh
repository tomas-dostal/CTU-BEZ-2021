#!/bin/bash 

echo "OUTPUT                                EXPECTED"
failed=0
for i in {1..16};
do  
    input=$(./hash $i |  sed -n 1p)
    hash=$(./hash $i |  sed -n 2p)
    expected=$( echo -n "$input" | xxd -r -ps | openssl sha384 | sed 's/(stdin)= //g')
    echo "test $i" 
    echo "input $input" 
    result=$(diff <(echo "$hash") <(echo "$expected"))
    if [ $? -eq 0 ]
    then
        echo "OK"
    else
        echo $result    
        failed=$((failed+1))
    fi

done

echo ""
echo "Failed: $failed"
