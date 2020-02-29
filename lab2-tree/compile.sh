BINARYPATH=./bin
for C_SOURECE in ./*.c
do
    gcc $C_SOURECE -o $BINARYPATH/${C_SOURECE%.c}
done