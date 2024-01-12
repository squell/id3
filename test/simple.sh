#! /bin/sh

# basic functionality

set -e

test `whoami` != root

TESTP=/tmp/id3-test
FILE=$TESTP/nULL

mkdir -p  /tmp/id3-test
rm -f $FILE ${FILE}2
cat /dev/null > ${FILE}
cat /dev/null > ${FILE}2

set -v
# wildcard
./id3 -a "%+1" "/tmp/id3-test/*"
./id3 -q %a $FILE | grep -q '^Null$'
test -z "`(./id3 -q %p%f "*/"; echo */) | sort | uniq -d`"
! ./id3 -q %p%f -X "*/"
./id3 --artist "" -- "$FILE"
./id3 -q %a $FILE | grep -q '^<empty>$'
cp "$FILE" "$TESTP"/'['
./id3 -q %a "$TESTP/\[" | grep -q '^<empty>$'
rm "$TESTP"/'['

# -m match
./id3 -a "" $FILE
./id3 -m "/tmp/id3-test/%aU*L"
./id3 -q %a $FILE | grep -q '^n$'
./id3 -m "/tmp/id3-test/%+a"
./id3 -q %a $FILE | grep -q '^Null$'

# tag info
./id3 -t %a -a Art01st -n 01 -c 01 $FILE
./id3 -q "%_t:%_a:%_n:%_c" $FILE | grep -q '^Null:Art01st:1:01$'
./id3 -g "psy ro" $FILE
./id3 -q %g $FILE | grep -q '^Psychedelic Rock$'
./id3 -g "42" $FILE
./id3 -q %g $FILE | grep -q '^Bass$'
./id3 -y "2007" $FILE
./id3 -q %y $FILE | grep -q '^2007$'
./id3 -l "ThisISAnAlbum" $FILE
./id3 -q "%*{l}" $FILE | grep -q '^This I S An Album$'
./id3 -q '%x %x %X' "$TESTP/*" | tr '\n' ':' | grep -q '1 1 1:2 2 2'
./id3 -q '%x %x %X'  $TESTP/*  | tr '\n' ':' | grep -q '1 1 1:1 1 2'
./id3 -q '%p%f' $FILE | grep -q "$FILE"

# modifiers
./id3 -q "%_|  Art01st   ThisISAnAlbum  (01/7) |?" /dev/null | grep -q '  Art01st   ThisISAnAlbum  (01/7) $'
./id3 -q "%|  Art01st   ThisISAnAlbum  (01/7) |?" /dev/null | grep -q 'Art01st ThisISAnAlbum (01/7)$'
./id3 -q "%-|  Art01st   ThisISAnAlbum  (01/7) |?" /dev/null | grep -q 'art01st thisisanalbum (01/7)$'
./id3 -q "%*|  Art01st   ThisISAnAlbum  (01/7) |?" /dev/null | grep -q 'Art01st This I S An Album (01/7)$'
./id3 -q "%+|  Art01st   ThisISAnAlbum  (01/7) |?" /dev/null | grep -q 'Art01st Thisisanalbum (01/7)$'
./id3 -q "%+#|  Art01st   ThisISAnAlbum  (01/7) |?" /dev/null | grep -q 'Art1st Thisisanalbum (1/7)$'
./id3 -q "%#+|  Art01st   ThisISAnAlbum  (01/7) |?" /dev/null | grep -q 'Art1st Thisisanalbum (1/7)$'
./id3 -q "%##|  Art01st   ThisISAnAlbum  (01/7) |?" /dev/null | grep -q 'Art01st ThisISAnAlbum (01/07)$'
./id3 -q "%###|  Art01st   ThisISAnAlbum  (01/7) |?" /dev/null | grep -q 'Art001st ThisISAnAlbum (001/007)$'
./id3 -q "%###|Art0st|?" /dev/null | grep -q 'Art000st$'
./id3 -q "%###|Art\%%%\|st|?" /dev/null | grep -q 'Art%%|st$'

./id3 -q "\\\\" /dev/null | grep -q '^\\$'

./id3 -a '%a ' $FILE
./id3 -1 -q "%_t:%_a:%_n:%_c" $FILE | grep -q '^Null:Art01st :1:01$'
cat /dev/null > ${FILE}2
./id3 -1 -D $FILE -cant %_c %_a %_n %_t ${FILE}2
./id3 -1 -d $FILE
test -z $(cat $FILE)
./id3 -1 -q "%_t:%_a:%_n:%_c" ${FILE}2 | grep -q '^Null:Art01st :1:01$'
./id3 -1u ${FILE}2
./id3 -1 -q "%_t:%_a:%_n:%_c" ${FILE}2 | grep -q '^Null:Art01st:1:01$'

# file rename
rm -f $FILE
./id3 -f `basename $FILE` ${FILE}2

# file mod. preserve
sleep 1
cp $FILE /tmp/test1
./id3 -M -1u $FILE
test /tmp/test1 -nt $FILE

# id3v2 (incomplete)
./id3 -2 -wCOMM:descr unalterable $FILE
./id3 -1 -2u $FILE
./id3 -2 -q "%_t:%_a:%_n:%_c" $FILE | grep -q '^Null:Art01st:1:01$'
./id3 -2 -q "%{COMM:descr}" $FILE | grep -q '^unalterable$'
./id3 -2 -rTALB -wTPE2 "%{TALB}" $FILE
./id3 -2 -q "%l:%a" $FILE | grep -q '^<empty>:Art01st$'
./id3 -2rTPE1 $FILE
./id3 -2 -q "%|%g|l:%a" $FILE | grep -q '^Bass:ThisISAnAlbum$'
./id3 -2 -wTXXX:descr tag $FILE
./id3 -2 -wCOMM:extra tag $FILE
./id3 -2 -wWXXX:$ dollar $FILE
./id3 -2 -wWXXX:€ euro $FILE
./id3 -2 -wWPAY "http://website" $FILE
./id3 -2 -wUSER::ita "dati per usarlo" $FILE
./id3 -2 -wPCNT 42 $FILE
./id3 -2 -wPOPM:"john@doe" 127 $FILE
./id3 -2 -wPOPM:"jane@roe" 42:918273645 $FILE
cat /dev/null > ${FILE}2
./id3 -2 -D $FILE ${FILE}2
./id3 -2d $FILE
test $(cat $FILE | wc -c) -eq 128
./id3 -2 -q "%{TXXX:descr}" ${FILE}2 | grep -q '^tag$'
./id3 -2 -q "%{COMM:extra}" ${FILE}2 | grep -q '^tag$'
./id3 -2 -q "%{WXXX:$}" ${FILE}2 | grep -q '^dollar$'
./id3 -2 -q "%{WXXX:€}" ${FILE}2 | grep -q '^euro$'
./id3 -2 -q "%{WPAY}" ${FILE}2 | grep -q '^http://website$'
./id3 -2 -q "%{USER::ita}" ${FILE}2 | grep -q '^dati per usarlo$'
./id3 -2 -q "%{USER}" ${FILE}2 | grep -q '^dati per usarlo$'
./id3 -2 -q "%{PCNT}" ${FILE}2 | grep -q '^42$'
./id3 -2 -q "%{POPM:john@doe}" ${FILE}2 | grep -q '^127:0$'
./id3 -2 -q "%{POPM:jane@roe}" ${FILE}2 | grep -q '^42:918273645$'
cp -f ${FILE}2 $FILE
./id3 -2a "a loooooooooooooooooooooooooooooonger artist name" $FILE
test $(cat $FILE | wc -c) -eq $(cat ${FILE}2 | wc -c)
./id3 -2s0 $FILE
size=$(cat $FILE | wc -c)
test $(cat $FILE | wc -c) -lt $(cat ${FILE}2 | wc -c)
./id3 -2s5000 $FILE
test $(cat $FILE | wc -c) -eq 5000
./id3 -2s0 $FILE
test $size -eq $(cat $FILE | wc -c)
rm -rf ${FILE}2
ln -s ${FILE} ${FILE}2
./id3 -2 -s 20000 ${FILE}2
test $(cat ${FILE}2 | wc -c) -eq 20000
for x in `seq 150`; do
	./id3 -2 -wTXXX:$x "test$x" "${FILE}2"
	n=$((RANDOM % 100 + 12))
	test "$n" -gt "$x" || ./id3 -2 -q "%{TXXX:$n}" "${FILE}2" | grep -q "^test$n"
done

# subtag - maintag interference
./id3 -123da "0artist" $FILE
./id3 -123a "%?" $FILE
./id3 -1q "%a" $FILE | grep -q '^0artist$'
./id3 -2q "%a" $FILE | grep -q '^0artist$'
./id3 -3q "%a" $FILE | grep -q '^0artist$'
./id3 -2wTPE1 "2artist" $FILE
./id3 -2q "%a" $FILE | grep -q '^2artist$'
./id3 -1q "%a" $FILE | grep -q '^0artist$'
./id3 -3wEAR "3artist" $FILE
./id3 -3q "%a" $FILE | grep -q '^3artist$'
./id3 -1q "%a" $FILE | grep -q '^3artist$' # updated by Lyrics3
./id3 -1d -l "0album" $FILE
./id3 -2q "%a:%l" $FILE | grep -q '^2artist:<empty>$'
./id3 -3q "%a:%l" $FILE | grep -q '^3artist:0album$'
./id3 -1q "%a:%l" $FILE | grep -q '^<empty>:0album$'
./id3 -1a "0artist" $FILE
./id3 -3q "%a" $FILE | grep -q '^0artist$'  # note: lyrics3 contains '3artist', but id3v1 overrides since their prefixes don't match
./id3 -23d -l "1album" $FILE
./id3 -1q "%a:%l" $FILE | grep -q '^0artist:1album$'
./id3 -2q "%a:%l" $FILE | grep -q '^<empty>:1album$'
./id3 -3q "%a:%l" $FILE | grep -q '^0artist:1album$'
./id3 -23a dummy $FILE
./id3 -2 -rTALB $FILE
./id3 -2q "%a:%l" $FILE | grep -q '^dummy:<empty>$'
./id3 -3 -l 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAZ' $FILE
./id3 -3 -rEAL $FILE
./id3 -3q "%a:%l" $FILE | grep -q '^dummy:AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA$'
./id3 -1d "$FILE"
test ! -z "$(cat $FILE)"
./id3 -3 --remove=EAR $FILE
./id3 -2 --remove=TPE1 $FILE
test -z "$(cat $FILE)"

# -E switch
./id3 -123a "dummy" $FILE
./id3 -1E -a "9artist" $FILE
./id3 -1q "%a" $FILE | grep -q '^9artist$'
./id3 -2E -a "8artist" $FILE
./id3 -2q "%a" $FILE | grep -q '^8artist$'
./id3 -3E -a "7artist" $FILE
./id3 -3q "%a" $FILE | grep -q '^7artist$'
./id3 -123d $FILE
./id3 -1E -a "9artist" $FILE
./id3 -1q "%a" $FILE | grep -q '^<empty>$'
./id3 -2E -a "8artist" $FILE
./id3 -2q "%a" $FILE | grep -q '^<empty>$'
./id3 -3E -a "7artist" $FILE
./id3 -3q "%a" $FILE | grep -q '^<empty>$'
./id3 -1  -t "1title" $FILE
./id3 -3E -t "2title" $FILE
./id3 -1q "%t" $FILE | grep -q '^1title$'
./id3 -3q "%t" $FILE | grep -q '^<empty>$'

# all okay
