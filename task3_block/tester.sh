#!/bin/bash 

IMG_FOLDER="testData"
IMG_PREFIX="img"

failed=0
for i in {1..3};
do
    echo "########## test $i ecb ############## "

    ORIGINAL="$IMG_FOLDER/$IMG_PREFIX$i"
    ENCRYPTED="$IMG_FOLDER/$IMG_PREFIX$i"_ecb_e.tga

    ./block "e" "ecb" "$ORIGINAL".tga
    ./block "d" "ecb" "$ENCRYPTED"

    SUFFIX="_ecb_e_ecb_d.tga"
    result=$(diff "$ORIGINAL".tga "$IMG_FOLDER/$IMG_PREFIX$i$SUFFIX")

    if [ $? -eq 0 ]
    then
        echo "OK"
        rm "$ORIGINAL"_ecb_e.tga
        rm "$ORIGINAL"_ecb_e_ecb_d.tga
    else
        echo $result    
        failed=$((failed+1))
    fi


done
for i in {1..3};
do
    echo "########## test $i cbc ############## "

    ORIGINAL="$IMG_FOLDER/$IMG_PREFIX$i"
    ENCRYPTED="$IMG_FOLDER/$IMG_PREFIX$i"_cbc_e.tga

    ./block "e" "cbc" "$ORIGINAL".tga
    ./block "d" "cbc" "$ENCRYPTED"

    SUFFIX="_cbc_e_cbc_d.tga"
    result=$(diff "$ORIGINAL".tga "$IMG_FOLDER/$IMG_PREFIX$i$SUFFIX")

    if [ $? -eq 0 ]
    then
        echo "OK"
        rm "$ORIGINAL"_cbc_e.tga
        rm "$ORIGINAL"_cbc_e_cbc_d.tga
    else
        echo $result
        failed=$((failed+1))
    fi

done

for i in {1..4};
do
    echo "########## test $i bad files ############## "

    ORIGINAL="$IMG_FOLDER/$IMG_PREFIX$i"

    ./block "e" "cbc" "$ORIGINAL"bad.tga
    RET=$?
    if [ $RET -eq 0 ]
    then
        echo "Failed"
        failed=$((failed+1))

    else
       echo ""
       echo "Return code: $RET"
       echo "OK"
    fi

done


echo ""
echo "Failed: $failed"
