#! /bin/sh

set -e

test `whoami` != root

mkdir -p /tmp/id3-test
FILE=/tmp/id3-test/unicode
cat /dev/null > $FILE

echo testing euro in latin15
export LC_CTYPE=en_US.ISO8859-15
test `./id3 -q '\u20ac' /dev/null` = '�'
echo testing euro in utf8
export LC_CTYPE=en_US.UTF-8
test `./id3 -q '\u20ac' /dev/null` = '€'
echo testing higher unicode
test `./id3 -q '\U10330' /dev/null` = '𐌰' 

echo storing unicode in id3v2
./id3 -2 -a '\u20ac' -t '\U010330' $FILE
test `./id3 -2 -q '%a' $FILE` = '€'
test `./id3 -2 -q '%t' $FILE` = '𐌰' 

echo seems fine
