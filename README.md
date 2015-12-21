### Description

id3 mass tagger is a tool for manipulating id3 and id3v2 tags in multiple files. It can generate tag fields from the filename and other variables, and/or rename files, using an intuitive syntax. id3 currently supports old-style ID3v1 tags, Lyrics3v2, as well as the more complex ID3v2 format. This means its use is limited to audio files which use these formats, i.e. MP3.

### Examples

```sh
id3 -a "Stallman" -t "Free Software Song" fs_song.mp3"
```
Add a simple tag to a file.
```sh
id3 mysong.mp3
id3 -v mysong.mp3
```
List tag info contained in file (short-form and detailed)
```sh
id3 --genre "psych rock" *.mp3
```
Sets genre to "Psychedelic Rock" in all mp3's
```sh
id3 --artist "TAFKAT" --track "%1" -title "%+2" "*. *.mp3"
```
Update tag fields similar to this;
>id3 -a "TAFKAT" -n "01" -t "My Song"  "01. my_song.mp3"<br/>
>id3 -a "TAFKAT" -n "02" -t "Untitled" "02. untitled.mp3"

```sh
id3 -m "%n. %+t.mp3"
```
Shorthand notation for the above, using the -m option.
```sh
id3 -f "%a - %t.mp3" blaet.mp3
```
Rename file to a standard "Artist - Title" format.
```sh
id3 -g "alt rock" -a "The Author" -l %1 -n %2 -t %3 "Author - */(*) *.mp3"
```
Process multiple directories at once.
```sh
id3 -2 -wTPE2 "%{TXXX:ALBUM ARTIST}" file.mp3
```
Copy the ID3v2 album artist field (as used by Foobar2000), if any, to the field used by iTunes
```sh
id3 -1 -3 -d *.mp3
```
Removes all ID3v1 and Lyrics3 tags from all mp3's
```sh
id3 -2 -1 -u "*.mp3"
```
Copy ID3v2 tag (if any) to ID3v1 tag in all files.
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
id3 -2 -rAPIC -rGEOB -s 0 *.mp3
```
Removes embedded images, binary blobs, and padding from all mp3's.
```sh
id3 -2 -rAPIC -rGEOB -s 0 -R "Music/*.mp3"
```
As above, but works recursively on all mp3's in the Music folder
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

ID3v2.4 at the start of a file can be read, but will be converted to the universally recognized ID3v2.3 when written. This may actually be a feature rather
than a limitation; e.g. running `id3 -2u` is a quick way to convert tags to a form your portable player may recognize.

On Windows, support for non-ASCII characters (e.g. Russian or Korean characters) is dependent on your system language settings. A Unicode version is on the TODO list.

### Getting it

Version  | Release date | Source | Pre-built binaries
-------- | ------------ | ------ | ------
0.80     | 21 Dec 2015  | [tarball](https://github.com/squell/id3/releases/download/0.80/id3-0.80.tar.gz)/[zip](https://github.com/squell/id3/releases/download/0.80/id3-080s.zip) | [Windows](https://github.com/squell/id3/releases/download/0.80/id3-080w.zip)<br> Debian/Ubuntu: [amd64](https://github.com/squell/id3/releases/download/0.80/id3mtag_0.80-1_amd64.deb)/[i386](https://github.com/squell/id3/releases/download/0.80/id3mtag_0.80-1_i386.deb)
0.79     | 30 Jan 2015  | [tarball](https://github.com/squell/id3/releases/download/0.79/id3-0.79.tar.gz)/[zip](https://github.com/squell/id3/releases/download/0.79/id3-079s.zip) | [Windows](https://github.com/squell/id3/releases/download/0.79/id3-079w.zip)<br> Debian/Ubuntu: [amd64](https://github.com/squell/id3/releases/download/0.79/id3mtag_0.79-1_amd64.deb)/[i386](https://github.com/squell/id3/releases/download/0.79/id3mtag_0.79-1_i386.deb)
0.78     | 21 Mar 2006  | [tarball](https://github.com/squell/id3/releases/download/0.78/id3-0.78.tar.gz)/[zip](https://github.com/squell/id3/releases/download/0.78/id3-078s.zip) | [Windows](https://github.com/squell/id3/releases/download/0.78/id3-078w.zip)

id3 mass tagger may also be a available on your system by default:

**Arch Linux**: `pacman -S id3`

**FreeBSD**: `pkg install id3mtag`

You can also find release notes, tarballs, and binaries [here](https://github.com/squell/id3/releases/latest/).

### Developer notes

Like many small projects, this started as something hacked together to scratch a need. Besides its obvious purpose, this has also become an exercise in writing a fully portable program, trying out C++ features on various compilers, etc.

If you like this program, you can help by trying to build and test it on an uncommon system, or becoming a package maintainer for some Linux distribution. Please contact me if you do so.

### Copyright

&copy; Marc Schoolderman 2003-2015. All rights reserved.

This program may be used freely, and you are welcome to redistribute it under certain conditions.

For the actual licensing conditions you should read the file [COPYING](https://raw.githubusercontent.com/squell/id3/master/COPYING), which should be accompanying the files you receive.

These files are distributed in the hope that they will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
accompanying file [COPYING](https://raw.githubusercontent.com/squell/id3/master/COPYING) for more details.
