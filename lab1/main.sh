#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Неверное число аргументов: $#"
    exit 1
fi

directory="$1"

if [ ! -d "$directory" ]; then
    echo "Указанный путь не является каталогом"
    exit 1
fi

echo "Содержимое каталога $directory:"
for dir in "$directory"/*; do
    if [ -d "$dir" ]; then
        subdirectories=$(find "$dir" -mindepth 1 -maxdepth 1 -type d | wc -l)
        if [ "$subdirectories" -gt 0 ]; then
            echo "$(basename "$dir")"
        fi
    fi
done
