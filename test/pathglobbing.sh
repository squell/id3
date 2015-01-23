#! /bin/sh

# test the internal -R globbing against 'find'

test `whoami` != root || exit 1

iter=0
while [ $iter -lt 10 ]; do
	files="$(date +%N | md5sum | tr -d '[:digit:][:space:]-' | sed 'y:abcdef:./?*es:')*"

	if [ "$files" = "/${files#*/}" ]; then  # begins with /
		files="/usr${files}"
		base="/usr"
	else
		files="/home/${files}"
		base="/home"
	fi

	# id3 -R implements a slight superset of find
	files=$(echo "$files" | sed 's:/\+:/:g') # remove double slashes
	files=$(echo "$files" | sed 's:/\.\+::g') # remove "." and ".."

	for f in $files; do
		if [ "$f" != "$files" ]; then
			iter=$(expr $iter + 1)
			echo "$files"
			find "$base" -path "$files" 2> /dev/null | sort > /tmp/test1
			./id3 -R -q "%_p%_f" "$files" 2> /dev/null | sort > /tmp/test2
			diff -q /tmp/test1 /tmp/test2 || exit 1
		fi
		break
	done
done
