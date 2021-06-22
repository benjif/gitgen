# gitgen - static webpage generator for git

## Installation

```
# For optional syntax highlighting
GG_COLOR=TRUE
sudo make install-color

# For optional markdown rendering
GG_MARKDOWN=TRUE

make && sudo make install
```

## Usage

### Generate files for a repository

```bash
# This will put everything into public/
./gitgen repo <repo path> [--max-commits <max>] [--max-filesize <max>] [--max-diff-lines <max>]
```

### Generate an index file

```bash
./gitgen index <repo path>...
```

## Syntax Highlighting and Markdown Rendering

Syntax highlighting requires [GNU source-highlight](https://www.gnu.org/software/src-highlite/) and markdown rendering requires [md4c](https://github.com/mity/md4c). Note that syntax highlighting currently slows generation by around ~2x.

## Dependencies

* [libgit2](https://libgit2.org/)
* [libsource-highlight](https://www.gnu.org/software/src-highlite/) (optional, for highlighting)
* [md4c](https://github.com/mity/md4c) (optional, for markdown rendering)

## Other Projects

* [stagit](https://codemadness.org/stagit.html) is another static page generator for git.
* [cgit](https://git.zx2c4.com/cgit/about/) is a CGI web interface for git.
