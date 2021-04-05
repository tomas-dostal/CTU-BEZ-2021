#!/bin/bash 

IMG_FOLDER="testData"
IMG_PREFIX="img"

failed=0
for i in {1..3};
do
    echo "########## test $i ecb ############## "

    ENCRYPTED="$IMG_FOLDER/$IMG_PREFIX$i.tga"
    DECRYPTED="$IMG_FOLDER/$IMG_PREFIX$i.tga_ecb.tga"

    ./block "e" "ecb" "$ENCRYPTED"
    ./block "d" "ecb" "$DECRYPTED"

    result=$(diff "$ENCRYPTED" "$IMG_FOLDER/$IMG_PREFIX$i.tga")

    if [ $? -eq 0 ]
    then
        echo "OK"
        #rm "$ENCRYPTED"_ecb.tga
        #rm "$ENCRYPTED"_ecb.tga_dec.tga
    else
        echo $result    
        failed=$((failed+1))
    fi


done
for i in {1..3};
do
    echo "########## test $i cbc ############## "

    ENCRYPTED="$IMG_FOLDER/$IMG_PREFIX$i.tga"
    DECRYPTED="$IMG_FOLDER/$IMG_PREFIX$i.tga_cbc.tga"

    ./block "e" "cbc" "$ENCRYPTED"
    ./block "d" "cbc" "$DECRYPTED"

    result=$(diff "$ENCRYPTED" "$IMG_FOLDER/$IMG_PREFIX$i.tga")

    if [ $? -eq 0 ]
    then
        echo "OK"
        #rm "$ENCRYPTED"_cbc.tga
        #rm "$ENCRYPTED"_cbc.tga_dec.tga
    else
        echo $result
        failed=$((failed+1))
    fi

done

echo ""
echo "Failed: $failed"
