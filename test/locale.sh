#! /bin/sh

set -e

test `whoami` != root

mkdir -p /tmp/id3-test
FILE=/tmp/id3-test/unicode
cat /dev/null > $FILE

set -v
# testing euro in latin15
export LC_CTYPE=en_US.ISO8859-15
test `./id3 -q '\u20ac' /dev/null` = '�'
# testing euro in utf8
export LC_CTYPE=en_US.UTF-8
test `./id3 -q '\u20ac' /dev/null` = '€'
echo testing higher unicode
test `./id3 -q '\U10330' /dev/null` = '𐌰' 

# reading uncode from ID3v2
export LC_CTYPE=en_US.ISO8859-15
test "`./id3 -q '%{TXXX:latin1}' test/encoding.tag `"  = "a latin1 string with � sign"
test "`./id3 -q '%{TXXX:utf16}' test/encoding.tag`"    = "a utf16 string with � sign (le bom)"
test "`./id3 -q '%{TXXX:utf16bom}' test/encoding.tag`" = "a utf16 string with � sign (be bom)"
test "`./id3 -q '%{TXXX:utf16be}' test/encoding.tag`"  = "a utf16be string with � sign"
test "`./id3 -q '%{TXXX:utf8}' test/encoding.tag`"     = "a utf8 string with � sign"
export LC_CTYPE=en_US.UTF-8

# storing unicode in id3v2
./id3 -2 -a '\u20ac' -t '\U010330' $FILE
test `./id3 -2 -q '%a' $FILE` = '€'
test `./id3 -2 -q '%t' $FILE` = '𐌰' 

# storing unicode in id3v2's complex frame
./id3 -2 -c '\u20ac' $FILE
test `./id3 -2 -q '%c' $FILE` = '€'
./id3 -2 -wTXXX:test '\u20ac' $FILE
test `./id3 -2 -q '%{TXXX:test}' $FILE` = '€'
./id3 -2 -wTXXX:€u greece $FILE
test `./id3 -2 -q '%{TXXX:€u}' $FILE` = 'greece'
./id3 -2 -wTXXX:un€u 'unic\U00010330de' $FILE
test `./id3 -2 -q '%{TXXX:un€u}' $FILE` = 'unic𐌰de' 

# storing unicode in filename
rm -f /tmp/id3-test/euro*
./id3 -2 -f "euro%aeuro" $FILE
test `./id3 -2 -q '%a' /tmp/id3-test/euro€euro` = '€'
test `./id3 -2 -q '%a' /tmp/id3-test/euro*euro` = '€'
test `./id3 -2 -q '%a' "/tmp/id3-test/euro?euro"` = '€'
test `./id3 -2 -q '%1' "/tmp/id3-test/euro*euro"` = '€'

# seems fine
