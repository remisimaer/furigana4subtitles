# Furigana4Subtitles

Convert Japanese subtitle files .srt to .ass format with furigana (reading aids) displayed above kanji characters.

## Prerequisites

### Windows
For Windows, please install [WSL2](https://learn.microsoft.com/en-us/windows/wsl/install) and follow GNU/Linux installation steps. Make sure you have installed WSL version 2.

### macOS
I don't have a Mac so don't hesitate to to the project.

### GNU/Linux & Windows
Adapt the commands to your WSL2 / distribution.

#### Install Mecab
```bash
sudo apt update
sudo apt install mecab libmecab-dev mecab-ipadic-utf8 pkg-config
```

## Usage 
#### Compile project
```bash
make
```

#### Convert single file
```bash
./furigana4subtitles "subtitle.srt"
```

#### Examples
| Action | Command |
|--------|---------|
| Multiple files | `./furigana4subtitles "file1.srt" "file2.srt" "file3.srt"` |
| All files from a folder | `./furigana4subtitles ./folder1/` |
| Multiple folders | `./furigana4subtitles ./folder1/ ./folder2/` |
| Mix folders and files | `./furigana4subtitles ./folder1/ "file1.srt" ./folder2/` |

## Output
The program generates `.ass` files in the same directory as the input files:
- `/path/to/subtitle.srt` → `/path/to/subtitle.ass`

## License
GNU General Public License v3.0 or later

## Author
Rémi SIMAER <rsimaer@gmail.com>
