# Warning
This project is in WORK IN PROGRESS!
IT IS NOT FUNCTIONAL!

# What does Furigana4Subtitles do?
It converts .str subtitle files into .ass files with furigana displayed above the text.
It works with major video players, including VLC.

# Prerequisites
```
sudo apt update
sudo apt install mecab libmecab-dev mecab-ipadic-utf8
```

# Compile
```
gcc main.c -o main -lmecab
```

## Run the program
### Linux
```
./main [argument: path to .str file]
```
### Windows
```
main.exe [argument: path to .str file]
```