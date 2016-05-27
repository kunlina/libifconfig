Automatically apply FreeBSD style(9) to source files

Please use textproc/uncrustify with the configuration file uncrustify_freebsd.cfg in this directory. (found at: https://github.com/freebsd/pkg/blob/master/freebsd.cfg)
#Example usage
You should cd into the project root directory.

This will replace existing files with the corrected files, but keep a backup:
uncrustify --replace -c tools/uncrustify_freebsd.cfg \*.c \*.h

If doing a style(9) pass on existing, unmodified files:
uncrustify --replace --no-backup -c tools/uncrustify_freebsd.cfg \*.c \*.h
