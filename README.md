# Furigana4Subtitles

A free & open-source tool for Japanese learners who watch anime with Japanese subtitles.

**Furigana4Subtitles** automatically converts .srt subtitles into .ass format, with hiragana readings (ふりがな) displayed above kanji.

It works seamlessly with **VLC** (uses a rendering technique compatible with VLC, unlike ruby annotations).

![Alt text](furigana4subtitles.png)

## Prerequisites
### Video tutorial
If you prefer a video format, [you can follow all the steps on YouTube.](https://youtu.be/DEWwYSAeoWE).

### Windows
For Windows, please install [WSL2](https://learn.microsoft.com/en-us/windows/wsl/install) and follow GNU/Linux installation steps. 

Make sure you have installed WSL version 2.

Once you have installed WSL2 with a GNU/Linux distribution (Ubuntu is recommended), run all of the following steps in the WSL2 terminal.

### macOS
I don't have a Mac so don't hesitate to contribute to the project.

### GNU/Linux

```bash
sudo apt update
sudo apt install build-essential git mecab libmecab-dev mecab-ipadic-utf8
```

## Clone the project
```bash
git clone https://github.com/remisimaer/furigana4subtitles.git
cd furigana4subtitles/
```

The following steps should be executed inside the furigana4subtitles folder.

## Build

```bash
make
```

This produces two executables:
- `furigana4subtitles` : command-line version
- `furigana4subtitles-cli` : interactive menu version

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
include/                # Headers
src/
  ├── utils.c           # File operations, config
  ├── srt.c             # SRT parser
  ├── ass.c             # ASS generator
  ├── mecab_helpers.c   # MeCab integration, furigana extraction
  └── cli.c             # Interactive CLI logic
main.c                  # Command-line entry point
main_cli.c              # Interactive entry point
obj/                    # Compiled object files (not committed)
```

## Roadmap

If the project becomes popular:

- A GUI version will be developed for Windows, GNU/Linux, and macOS. Raylib is a candidate, but suggestions for other lightweight, cross-platform solutions are welcome.
- A one-click installer will be developed for Windows, GNU/Linux, and macOS.

## License

[GNU General Public License v3.0 or later](LICENSE)

## Author

Rémi SIMAER : <rsimaer@gmail.com>
