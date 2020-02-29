BINARYPATH=./bin
PUBLIC_LIB=./GeneralHashFunctions.c
for C_SOURECE in ./*.c
do
    if [ $C_SOURECE == $PUBLIC_LIB ]
    then
        continue
    fi

    echo $BINARYPATH/${C_SOURECE%.c}
    if [ $C_SOURECE == ./bf_search.c ]
    then
        gcc $C_SOURECE -o $BINARYPATH/${C_SOURECE%.c} -lm
    elif [[ $C_SOURECE =~ ^./hashtable.*c$ ]]
    then
        gcc $C_SOURECE $PUBLIC_LIB -o $BINARYPATH/${C_SOURECE%.c}
    else
        gcc $C_SOURECE -o $BINARYPATH/${C_SOURECE%.c}
    fi
done
