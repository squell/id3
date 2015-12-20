


                     
 _ _  __ _ _ __  ___ 
| ' \/ _` | '  \/ -_)
|_||_\__,_|_|_|_\___|
                     
       id3 - a command line mass tagger

                          _    
 ____  _ _ _  ___ _ __ __(_)___
(_-< || | ' \/ _ \ '_ (_-< (_-<
/__/\_, |_||_\___/ .__/__/_/__/
    |__/         |_|           
       id3 [-v] [-m] [-123] [-d] [-t title] [-a artist] [-l album]
	 [-n tracknr] [-y year] [-g genre] [-c comment]
	 [-f template] [-q format] [-D file] [-R] [-M]
	 [-E] [-s size] [-u] [-rFRAME] [-wFRAME data] "filespec"

    _                _      _   _          
 __| |___ ___ __ _ _(_)_ __| |_(_)___ _ _  
/ _` / -_|_-</ _| '_| | '_ \  _| / _ \ ' \ 
\__,_\___/__/\__|_| |_| .__/\__|_\___/_||_|
                      |_|                  
       id3  mass tagger is a tool for listing and manipulating ID3 and Lyrics3
       tags in multiple files. It can generate tag fields  from  the  filename
       and other variables, and/or rename files, using an intuitive syntax.

       id3  currently supports old-style ID3v1 tags, Lyrics3v2, as well as the
       more complex ID3v2 format.  This means its  use	is  limited  to  audio
       files which use these formats, i.e. MPEG-1 Layer III.

          _   _             
 ___ _ __| |_(_)___ _ _  ___
/ _ \ '_ \  _| / _ \ ' \(_-<
\___/ .__/\__|_\___/_||_/__/
    |_|                     
       Order of the options is only important where specified.

       filespec ...
	      specifies the file(s) to be affected by the command.

	      If  you use wildcards, it is necessary to enclose the specifica-
	      tion in double quotes ("") for pattern matching to work.

       -h, --help
	      show command line help

       -V, --version
	      display version number and copyright

       -v, --verbose
	      be verbose

       -1     process/add ID3v1 tags.

       -2     process/add ID3v2 tags.

       -3     process/add Lyrics3 tags.

	      If more than one tag type is specified, they  are  all  written,
	      and the left-most one that is found is used for variable substi-
	      tution. If no tag type is specified, id3 will by default attempt
	      to read ID3v2, Lyrics3 and ID3v1 tags in that order, only modify
	      existing ID3v2 and Lyrics3 tags, and modify/add ID3v1 tags.

       -R, --recursive
	      searches recursively; When this is  enabled,  `*'  wildcards  in
	      filespec will also match against directory separators. Normally,
	      this is not the case.

       -M, --keep-time
	      preserve last modification time of files operated on

       --     force the following argument to be interpreted  as  a  filename.
	      Use this for files that start with a `-' (dash).

   Modifying operations
       The default operation of id3 is to list the tags found. By using any of
       the following options, this behaviour is inhibited.

       -d, --delete
	      do not re-use existing tag data. If no new  tag  information  is
	      specified  in  conjunction  with	this option, all selected tags
	      will be removed.

       -t title,     --title title
       -a artist,    --artist artist
       -l album,     --album album
       -n tracknr,   --track tracknr
       -y year,      --year year
       -g genre,     --genre genre
       -c comment,   --comment comment

	      add/replace the specified fields in all selected tags  with  the
	      values  given.  Field  values are scanned for substitution vari-
	      ables, see SUBSTITUTION below. If a  field  value  is  a	single
	      variable, and its substitution fails, no operation is performed.

	      Genres  can  be specified either directly or with their assigned
	      number, regardless of tag format.

       -f filename-template, --rename filename-template
	      rename files encountered according  to  filename-template.   The
	      argument	is  scanned for substitution variables. An empty vari-
	      able will by default get replaced  with  "Unknown".   Trying  to
	      rename to an already existing file will cause an error.

       -q format, --query format
	      for each file encountered, format will get scanned for substitu-
	      tion variables, and  written  to	standard  output.  Using  this
	      option will block any attempt to modify files.

       -m, --match
	      match  mode; interpret any substitution variables (see SUBSTITU-
	      TION below) found in a filespec as a wildcard, and set the  cor-
	      responding tag field to the matched portion of the filename.

	      Thus,    `id3   -m "%a - %t.mp3"'   is   short-hand   for   `id3
	      -a %1 -t %2 "* - *.mp3"'.

       -D filename, --duplicate filename
	      duplicate and copy the tags found  in  filename  to  the	target
	      files.  The  tag in each target file is replaced after any vari-
	      ables have been read, but before any fields are updated.

	      Any original tag values not explicitly written to  the  new  tag
	      (for  example,  by using -u) are lost. If filename does not have
	      any tags, this option is identical to the -d option.

       The following options only apply on  the  tag  most  recently  selected
       before them, and only have meaning where relevant.

       -E, --if-exists
	      only  write  a  tag of the most recently selected type if a file
	      already contains it; do not add new ones.

       -u, --update
	      update all standard tag fields by copying them from  the	source
	      tag. This is similar to writing `-talnygc %t %a %l %n %y %g %c',
	      but only operates on the most recent tag. It is possible for the
	      source and destination tag to be the same.

       -s size, --size size
	      try to write a new tag using exactly size bytes, adding / remov-
	      ing padding as necessary. The resulting tag will have no padding
	      if size is smaller than the actual size necessary.

       -rFRAME, --remove=FRAME
	      remove  occurrences  of frames named FRAME from the tag. Consult
	      the format  documentation  for  valid  FRAME  names.   For  text
	      frames, it is equivalent to `-wFRAME ""'.

       -wFRAME data, --frame=FRAME data
	      add / update a frame named FRAME with data in the  tag.  data is
	      scanned for substitution variables. Again,  consult  the	format
	      documentation. See COMPATIBILITY for more information.

       Short-form  options can be stacked in a single argument for more conve-
       nience.	For example, running

	  id3 -2d -alt "Artist" "Album" "Title" *.mp3

       is equivalent to:

	  id3 -2 -d -a "Artist" -l "Album" -t "Title" *.mp3


         _       _   _ _        _   _          
 ____  _| |__ __| |_(_) |_ _  _| |_(_)___ _ _  
(_-< || | '_ (_-<  _| |  _| || |  _| / _ \ ' \ 
/__/\_,_|_.__/__/\__|_|\__|\_,_|\__|_\___/_||_|
                                               
       id3 can perform "printf-like" substitution on the values prior to writ-
       ing  them  to a tag. Note that if you want to use pattern matching, you
       HAVE TO enclose the wildcard specification on the command line in  dou-
       ble quotes to prevent your shell from expanding any wildcards.

       \c     escape  sequence. \n, \r, \t, \v, \f, \b, \a, get replaced as in
	      C, any other character will be stripped of any special  meaning.
	      E.g., \n becomes the newline character, \\ a single backslash.

       %<modifiers>N
	      where  N	<-  [0..9]  replaced with the portion of the file path
	      matching the nth `*' (asterisk) wildcard in the file  specifica-
	      tion. 0 is taken to mean 10.

       %<modifiers>c
	      where c <- [a..z]
	      replaced by values according to the following table:

	      %t title
	      %a artist
	      %l album title
	      %n track number
	      %y year
	      %g genre
	      %c comment field
	      %f file name (without path)
	      %p path to filename
	      %x auto-increasing counter
	      %X file counter

	      Values get read (where applicable) from the source tag, which is
	      the left-most tag selected on the command line, and reflect  the
	      state  of  the  file  before any modifications were made. If the
	      source value is not available,  the  variable  fails.   "%_p%_f"
	      combines to the raw full path and file name. The "%x" value gets
	      increased every time it has been	substituted  inside  the  same
	      directory,  and  is  intended for auto-numbering. "%X" increases
	      for every file processed.

       %<modifiers>{FRAME}
	      replaced by the content of  the  FRAME  frame  in  the  selected
	      source  tag; any frame writeable with the -w option can be used;
	      see COMPATIBILITY for more information.

       %%     replaced with a single "%", equivalent to \%

       %|text||alt text||...|?
	      substituted by the first text that was completely successful, or
	      fails  as  empty,  see  fall-backs below. This can be used as an
	      all-or-nothing substitution. A lone "%?" always fails.

   Available <modifiers> (optional):
       + (plus sign)
	      Capitalize the substituted value

       - (minus sign)
	      Convert all characters to lowercase

       _ (underscore)
	      Use the  raw  value  of  the  variable.  Normally,  substitution
	      replaces any underscores with spaces, and condenses empty white-
	      space.

       * (asterisk)
	      Split the variable into separate words by looking at  the  capi-
	      talization.

       # (hash or pound sign)
	      Attempt  to  fit	numeric  values in the substituted string to a
	      desired width, by removing or adding  leading  zeros.   Multiple
	      hash  signs  can	be  stacked  to indicate the desired width. If
	      there are no numeric values, this modifier has no effect.

       |fall-back|
	      If substitution for a variable fails, attempt fall-back instead.
	      fall-back  itself  may  be  empty  or  contain  other  variables
	      (including other fall-backs). If	fall-back  contains  variables
	      that  fail,  the	fall-back  fails and will not be used. If more
	      than one fall-back is provided, successive fall-backs are  tried
	      until one succeeds.

                         _        
 _____ ____ _ _ __  _ __| |___ ___
/ -_) \ / _` | '  \| '_ \ / -_|_-<
\___/_\_\__,_|_|_|_| .__/_\___/__/
                   |_|            
       Here are some examples of using id3 :

       id3 -a "Stallman" -t "Free Software Song" fs_song.mp3"
	      Add a simple tag to a file.

       id3 muzak.mp3
	      List tag information in a file.

       id3 -d *.mp3
	      Removes all ID3v1 tags from all mp3's.

       id3 -2 -1u fs_song.mp3
	      Copy ID3v2 tag to ID3v1 tag in selected file.

       id3 -D source.mp3 -1 -2 dest.mp3
	      Duplicate ID3v1 and ID3v2 tags of source.mp3

       id3 -a "TAFKAT" -n "%1" -t "%+2" "*. *.mp3"
	      Update tag fields similar to this;
		-a "TAFKAT" -n "01" -t "My Song"  "01. my_song.mp3"
		-a "TAFKAT" -n "02" -t "Untitled" "02. untitled.mp3"

       id3 -2 -f "%a - %t.mp3" blaet.mp3
	      Rename file to a standard format, using ID3v2 values.

       id3 -a %t -t %a "*.mp3"
	      Swap artist and title fields in all mp3's.

       id3 -2 -rAPIC -s 0 *.mp3
	      Removes embedded images and padding from all mp3's.

       id3 -2d -u *.mp3
	      Rewrite ID3v2 tag while keeping only the basic fields.

       id3 -2 -wUSLT "foo, bar0alala!0 blaet.mp3
	      Adds an ID3v2 lyric frame to blaet.mp3.

       id3 -v -g alt-rock -alnt "The Author" %1 %2 %3 "Author - */(*) *.mp3"
	      Process multiple directories at once.

       id3 -v -g alt-rock -a "The Author" -m "Author - %l/(%n) %t.mp3"
	      Shorthand for the previous example.

       id3 -2 -c "Was: %_f" -f "%|Nobody|a - %|Untitled (%x)|t.mp3" "*.mp3"
	      Rename with missing values replaced. Saves previous filename in the comments.

       id3 -2 -q "%|%{TPE2}||%{TXXX:ALBUM ARTIST}|?"
	      Tries to print the "album artist" using two possible ID3v2 frames.

       id3 -2 -q "%| %a - %|Untitled|t || %t || %1 |?" "*.mp3"
	      Generate a simple list of songs.

          _          
 _ _  ___| |_ ___ ___
| ' \/ _ \  _/ -_|_-<
|_||_\___/\__\___/__/
                     
       The  internal  pattern matching emulates the normal pattern matching of
       "sh". It supports ?, * and [].

       A shell pattern will never match a forward slash ("/") or a  dot  (".")
       beginning a filename. Wildcards can be used for directories as well (to
       arbitrary depths), in which case a search will be performed.

       In an ambiguous situation, the pattern matcher will  always  resolve  a
       "*"  wildcard to the shortest possible sequence of tokens. This differs
       from the behavior of regular expressions,  however  it  tends  to  make
       sense in the context of filenames.

       Do NOT add ID3 tags to files for which it does not make sense, i.e, add
       them only to MP3 files. In particular, do not add  ID3v2  tags  to  Ogg
       files, since ID3v2 tags start at the beginning of the file.

                         _   _ _    _ _ _ _        
 __ ___ _ __  _ __  __ _| |_(_) |__(_) (_) |_ _  _ 
/ _/ _ \ '  \| '_ \/ _` |  _| | '_ \ | | |  _| || |
\__\___/_|_|_| .__/\__,_|\__|_|_.__/_|_|_|\__|\_, |
             |_|                              |__/ 
       id3 has a built-in genre list of 148 genres. If you pass the -g parame-
       ter a string instead of a number when using ID3v1, id3  tries  to  find
       the  specified  genre  in  this	list, and selects the closest possible
       match (if any). For the genre numbers and exact spelling,  see  id3v1.c
       in  the	source distribution. An empty or invalid genre is assigned the
       number 0.

       The ID3v1 format only supports to the ISO-8859-1 (Latin 1) encoding. If
       you need other Unicode characters, you need to use ID3v2 tags.

       When  using  -2,  id3  will  write ID3v2.3 by default, unless a file is
       already tagged with the older ID3v2.2. id3 can read ID3v2.4  tags,  but
       these will be converted to ID3v2.3 when modified.

       Furthermore, with ID3v2 tags, the -wFRAME option and %{FRAME} substitu-
       tion only support the following ID3v2.2 (3 letter)/ID3v2.3  (4  letter)
       frames: T??/T??? (text), W??/W??? (links), COM/COMM (comment), IPL/IPLS
       (involved  people), ULT/USLT (lyrics), CNT/PCNT (numeric play  counter)
       and USER (tos, v2.3 only).  Attempts to write ID3v2.2 frames to ID3v2.3
       or vice versa will be ignored.

       Several ID3v2 frames can be  specialized  with  additional  descriptors
       (TXXX,  WXXX,  COMM,  USLT).  These  can  be  read or written using the
       extended syntax -wFRAME:descriptor and  %{FRAME:descriptor}.   Descrip-
       tors  are  case	sensitive and may contain whitespace.  For frames that
       are language-specific (COMM, USLT), the form  FRAME:descriptor:xxx  may
       also  be  used,	where  xxx  is a three letter ISO-639-2 language code.
       Which descriptors are meaningful is application-specific.

       id3 does not support unnecessary ID3v2 features	such  as  compression,
       encryption, or embedding binary data (including image files).

           _   _            
 __ _ _  _| |_| |_  ___ _ _ 
/ _` | || |  _| ' \/ _ \ '_|
\__,_|\_,_|\__|_||_\___/_|  
                            
       Written by Marc R. Schoolderman <squell@alumina.nl>.

                      _      _   _   
 __ ___ _ __ _  _ _ _(_)__ _| |_| |_ 
/ _/ _ \ '_ \ || | '_| / _` | ' \  _|
\__\___/ .__/\_, |_| |_\__, |_||_\__|
       |_|   |__/      |___/         
       This  is free software; see the source for copying conditions. There is
       NO warranty; not even for MERCHANTABILITY or FITNESS FOR  A  PARTICULAR
       PURPOSE.

                    _         
 ___ ___ ___   __ _| |___ ___ 
(_-</ -_) -_) / _` | (_-</ _ \
/__/\___\___| \__,_|_/__/\___/
                              
       Program homepage: https://squell.github.io/id3



