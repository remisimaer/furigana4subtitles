# Furigana4Subtitles

![Alt text](furigana4subtitles.png)

Convert Japanese subtitle files .srt to .ass format with furigana (reading aids) displayed above kanji characters.

## Prerequisites

### Windows
For Windows, please install [WSL2](https://learn.microsoft.com/en-us/windows/wsl/install) and follow GNU/Linux installation steps. 

Make sure you have installed WSL version 2.

### macOS
I don't have a Mac so don't hesitate to contribute to the project.

### GNU/Linux & Windows with WSL2

#### Install MeCab
```bash
sudo apt update
sudo apt install mecab libmecab-dev mecab-ipadic-utf8
```

## Build

```bash
make
```

This produces two executables:
- `furigana4subtitles` — command-line version
- `furigana4subtitles-cli` — interactive menu version

Clean build artifacts:
```bash
make clean
```

## Usage

### Command-line version

```bash
./furigana4subtitles <files or folders...>
```

> **Note:** Use quotes around paths containing spaces or special characters.

| Action | Command |
|--------|---------|
| Single file | `./furigana4subtitles subtitle.srt` |
| Multiple files | `./furigana4subtitles ep01.srt ep02.srt ep03.srt` |
| With spaces | `./furigana4subtitles "my subtitle.srt"` |
| Folder (recursive) | `./furigana4subtitles ./subs/` |
| Mix | `./furigana4subtitles ./folder/ "special.srt"` |

### Interactive version

```bash
./furigana4subtitles-cli
```

```
  ╔══════════════════════════════════════════════════════════════╗
  ║                           MAIN MENU                          ║
  ║              .srt → .ass with furigana (ふりがな)            ║
  ╠══════════════════════════════════════════════════════════════╣
  ║   [1] Convert subtitles                                      ║
  ║   [2] Set font size (current:  52px)                         ║
  ║   [q] Quit                                                   ║
  ╚══════════════════════════════════════════════════════════════╝
```

**Features:**
- Convert subtitles .srt files and/or subtitles .srt contained in folders
- Adjustable font size (16-120px) with proportional furigana scaling
- Press `q` at any step to go back

## Output

Generated `.ass` files are placed alongside the input files:
```
/path/to/subtitle.srt → /path/to/subtitle.ass
```

## Project Structure

```
include/             # Headers
src/
  ├── utils.c        # File operations, config
  ├── srt.c          # SRT parser
  ├── ass.c          # ASS generator
  ├── mecab_helpers.c # MeCab integration, furigana extraction
  └── cli.c          # Interactive CLI logic
main.c               # Command-line entry point
main_cli.c           # Interactive entry point
obj/                 # Compiled object files (not committed)
```

## License

[GNU General Public License v3.0 or later](LICENSE)

## Author

Rémi SIMAER — <rsimaer@gmail.com>
