#! /bin/sh

# test the internal matching against perl regexes

test `whoami` != root || exit 1

randstr() {
	head -c4 /dev/urandom | shasum | head -c16 | tr -d '[:digit:]'
}

iter=0
while [ $iter -lt 30 ]; do
	files="$(randstr | sed 'y:abcdef:.s?*e?:')*"
	files="/etc/${files}"
	stars=$(echo "$files" | tr -cd '*' | wc -c)
	test "$stars" -lt 4 && continue
	test -z "$(echo "$files" | tr -cd es)" && continue
	for f in $files; do
		if [ "$f" != "$files" ]; then
			iter=$(expr $iter + 1)
			echo "$files"
			regex=$(echo "$files" | sed 's:\.:[.]:g;s:?:.:g;s:\*:(.*?):g')
			ls -d $files | perl -pe "s#$regex"'$#\1:\2:\3:\4#' | sort > /tmp/test1
			./id3 -q "%_1:%_2:%_3:%_4" "$files" | sort > /tmp/test2
			diff -q /tmp/test1 /tmp/test2 || exit 1
		fi
		break
	done
done
