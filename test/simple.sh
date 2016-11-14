#! /bin/sh

# basic functionality

set -e

test `whoami` != root

FILE=/tmp/id3-test/nULL

mkdir -p  /tmp/id3-test
cat /dev/null > $FILE

set -v
# wildcard
./id3 -a "%+1" "/tmp/id3-test/*"
./id3 -q %a $FILE | grep -q '^Null$'

# -m match
./id3 -a "" $FILE
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
./id3 -q "%*l" $FILE | grep -q '^This I S An Album$'

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

./id3 -a '%a ' $FILE
./id3 -1 -q "%_t:%_a:%_n:%_c" $FILE | grep -q '^Null:Art01st :1:01$'
cat /dev/null > ${FILE}2
./id3 -1 -D $FILE -cant %_c %_a %_n %_t ${FILE}2
./id3 -1 -d $FILE
test -e $(cat $FILE)
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
cat /dev/null > ${FILE}2
./id3 -2 -D $FILE ${FILE}2
./id3 -2d $FILE
test $(cat $FILE | wc -c) -eq 128
./id3 -2 -q "%{TXXX:descr}" ${FILE}2 | grep -q '^tag$'
./id3 -2 -q "%{COMM:extra}" ${FILE}2 | grep -q '^tag$'
./id3 -2 -q "%{WXXX:$}" ${FILE}2 | grep -q '^dollar$'
./id3 -2 -q "%{WXXX:€}" ${FILE}2 | grep -q '^euro$'
cp -f ${FILE}2 $FILE
./id3 -2a "a loooooooooooooooooooooooooooooonger artist name" $FILE
test $(cat $FILE | wc -c) -eq $(cat ${FILE}2 | wc -c)
./id3 -2s0 $FILE
test $(cat $FILE | wc -c) -lt $(cat ${FILE}2 | wc -c)
./id3 -2s5000 $FILE
test $(cat $FILE | wc -c) -eq 5000

# subtag - maintag interference
./id3 -123a "0artist" $FILE
./id3 -2wTPE1 "2artist" $FILE
./id3 -2q "%a" $FILE | grep -q '^2artist$'
./id3 -3wEAR "3artist" $FILE
./id3 -3q "%a" $FILE | grep -q '^3artist$'

# all okay
