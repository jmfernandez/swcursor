# swcursor

![swcursor example screenshot](cursors/cursor-large.png)

This is a hack based fork of swcursor to replicate mouse overlays on linux.

This is a simple software cursor for X11 environments, it creates an
overlay window with an (large) image that follows the cursor
around. You will need to arrange for your normal cursor to be hidden,
or screen record using ffmpeg's kmsgrab which often does not capture
harware cursors at all.


## Installation and Usage

You will need to have working development libraries for gtk-3.0,
gdk and xlib.


```
$ make
$ ./swcursor

```

```
Usage: ./swcursor [-ir] [file...]

-i [file.png]
 load the png file as mouse cursor

-r [number]
set the refresh rate per seconds

Example:
./swcursor -i cursors/cursor-large.png -r 12
```

```
# A variety of cursors can be found in the cursors folder
# You can display any transparent PNG image you like for the cursor
$ ./swcursor cursors/cursor-large.png
```

For more details see
https://github.com/andykitchen/swcursor
