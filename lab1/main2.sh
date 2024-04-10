find "$1"  -maxdepth 1 -type d | sed '1d' | while read x; do
    DIGIT=$(find "$x" -type d | sed '1d' | wc -l)
        if [ $DIGIT -ne 0 ]; then
            echo "$x"
        fi
done
