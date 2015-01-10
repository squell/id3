### Description

id3 mass tagger is a tool for manipulating id3 and id3v2 tags in multiple files. It can generate tag fields from the filename and other variables, and/or rename files, using an intuitive syntax. id3 currently supports the old-style ID3 format with track-number extension (ID3 v1.1), as well as the more complicated ID3v2 (ID3 v2.2.0, v2.3.0) format. This also means its use should be limited to audio files which use these formats (i.e., MP3).

### Examples

```sh
id3 -a "Stallman" -t "Free Software Song" fs_song.mp3"
```
Add a simple tag to a file.
```sh
id3 -d *.mp3
```
Removes all ID3v1 tags from all mp3's
```sh
id3 -g "psych rock" *.mp3
```
Sets genre to "Psychedelic Rock" all mp3's
id3 -2 -1 -u "*.mp3"
```
Copy ID3v2 tag to ID3v1 tag in all files.
id3 -a "TAFKAT" -n "%1" -t "%+2" "*. *.mp3"
```
Update tag fields similar to this;
>id3 -a "TAFKAT" -n "01" -t "My Song"  "01. my_song.mp3"<br/>
>id3 -a "TAFKAT" -n "02" -t "Untitled" "02. untitled.mp3"

```sh
id3 -2 -f "%a - %t.mp3" blaet.mp3
```
Rename file to a standard "Artist - Title" format, using ID3v2 values.
```sh
id3 -g "alt rock" -a "The Author" -l %1 -n %2 -t %3 "Author - */(*) *.mp3"
```
Process multiple directories at once.
```sh
id3 -g "alt rock" -a "The Author" -m "Author - %l/(%n) %t.mp3"
```
Shorthand for the previous example.
```sh
id3 -a %t -t %a "*.mp3"
```
Swap artist and title fields in all mp3's.
```sh
id3 -D source.mp3 -1 -2 dest.mp3
```
Copy ID3v1 and ID3v2 tags of source.mp3 to dest.mp3
```sh
id3 -D source.mp3 -1u -2u dest.mp3
```
As above, but only replaces the non-standard or blank fields in dest.mp3 by data from source.mp3.
```sh
id3 -2 -rAPIC -s 0 *.mp3
```
Removes embedded images and padding from all mp3's.
```sh
id3 -2 -rAPIC -s 0 -R "*.mp3" "/my documents"
```
As above, but works recursively on all mp3's in the directory tree starting at /my documents
```sh
id3 -2 -q "%| %a - %|Untitled|t || %t || %1 |?" "*.mp3"
```
Generate a playlist, using the best possible text
```sh
id3 -2 -c "Was: %_f" -f "%a - %|Untitled (%#x)|t.mp3" "*.mp3"
```
Advanced rename. Saves previous filename in the comment field, and renames files without proper tags to;
>Unknown - Untitled (01).mp3<br/>
>Unknown - Untitled (02).mp3<br/>
>... etc

Fore more information, consult the [documentation](https://github.com/squell/id3/blob/master/README).
### Limitations

ID3v2.4 tags are not supported; because ID3v2.4 is a horrible 'standard'. Also, do not use this program to tag Ogg Vorbis or FLAC files.

### Getting it

You can find release tarballs, and binaries for Windows [here](https://github.com/squell/id3/releases/latest/).

id3 mass tagger may also be a available on your system by default:

**Arch Linux**: `pacman -S id3`

**FreeBSD**: `pkg install id3mtag`

### Copyright

&copy; Marc Schoolderman 2003-2006. All rights reserved.

This program may be used freely, and you are welcome to redistribute it under certain conditions.

For the actual licensing conditions you should read the file [COPYING](https://raw.githubusercontent.com/squell/id3/master/COPYING), which should be accompanying the files you receive.

These files are distributed in the hope that they will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
accompanying file [COPYING](https://raw.githubusercontent.com/squell/id3/master/COPYING) for more details.
