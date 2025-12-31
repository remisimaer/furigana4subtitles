# Furigana4Subtitles

Convert Japanese subtitle files .srt to .ass format with furigana (reading aids) displayed above kanji characters.

## Prerequisites

### Linux (Debian/Ubuntu)
#### Install Mecab
```bash
sudo apt update
sudo apt install mecab libmecab-dev mecab-ipadic-utf8
```

#### Install Raylib
[Follow the official Raylib documentation](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux) 

### Windows
TODO


## Usage 
### Linux
#### Compile project
```bash
make
```

#### Convert single file
```bash
./furigana4subtitles "subtitle.srt"
```

#### Convert multiple files
```bash
./furigana4subtitles "file1.srt" "file2.srt" "file3.srt"
```

### Windows
TODO

## Output

The program generates `.ass` files with the same name as the input files:
- `subtitle.srt` → `subtitle.ass`

## License

GNU General Public License v3.0 or later

## Author

Rémi SIMAER <rsimaer@gmail.com>