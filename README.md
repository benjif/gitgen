# gitgen - static HTML generator for git repositories

## Building

### Without syntax highlighting

```
make && sudo make install
```

### With syntax highlighting

```
COLOR=TRUE make
sudo make install-color && sudo make install
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

## Syntax Highlighting

Syntax highlighting requires [GNU source-highlight](https://www.gnu.org/software/src-highlite/). Note that syntax highlighting currently slows generation by around ~2x.

## Dependencies

* libgit2
* (optional) libsource-highlight and source-highlight

## Other Projects

* [stagit](https://codemadness.org/stagit.html) is another static page generator for git.
* [cgit](https://git.zx2c4.com/cgit/about/) is a CGI web interface for git.
