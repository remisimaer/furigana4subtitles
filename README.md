# Furigana4Subtitles

Convert Japanese subtitle files .srt to .ass format with furigana (reading aids) displayed above kanji characters.

## Prerequisites

### Windows
For Windows, please install [WSL2](https://learn.microsoft.com/en-us/windows/wsl/install) and follow GNU/Linux installation steps.

### macOS
macOS users: I don't have a Mac so please contribute by adding the steps you followed to the README ;)

### GNU/Linux & Windows
Adapt the commands to your WSL2 / distribution.
#### Install Mecab
```bash
sudo apt update
sudo apt install mecab libmecab-dev mecab-ipadic-utf8 pkg-config
```

#### Install Raylib
[Follow the official Raylib documentation](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux) 


## Usage 
#### Compile project
```bash
make
```

#### Convert single file
```bash
./furigana4subtitles "subtitle.srt"
```

#### Convert multiple files one by one
```bash
./furigana4subtitles "file1.srt" "file2.srt" "file3.srt"
```

#### Recursive on a folder
```bash
./furigana4subtitles -R ./subs/
```

#### Recursive on multiple folders
```bash
./furigana4subtitles -R ./anime1/ ./anime2/
```

#### Mix of recursive on folder & files
```bash
./furigana4subtitles -R ./subs/ "file1.srt"
```
## Output
The program generates `.ass` files with the same name as the input files:
- `subtitle.srt` → `subtitle.ass`

## License
GNU General Public License v3.0 or later

## Author
Rémi SIMAER <rsimaer@gmail.com>
