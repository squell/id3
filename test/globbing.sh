#! /bin/sh

# test the internal globbing against the shell

test `whoami` != root || exit 1

randstr() {
	head -c4 /dev/urandom | shasum | head -c16 | tr -d '[:digit:]'
}

iter=0
while [ $iter -lt 50 ]; do
	files="$(randstr | sed 'y:abcdef:./?*es:')*"
	files="${files%.}"
	files="${files%.}"
	if [ "$files" = "/${files#*/}" ]; then  # begins with /
		files="${files#*/}"
		base="."
	else
		files="/usr/${files}"
		base="/usr"
	fi
	for f in $files; do
		if [ "$f" != "$files" ]; then
			iter=$(expr $iter + 1)
			echo "$files"

# id3 will never match "." ".." against wildcards intended to match files;
# so we need to filter those; also, filter anything containing /proc/ or /dev/

			find $files -prune 2> /dev/null | grep -v '\(^\|\/\)[.]\{1,2\}$' | grep -v -e "/proc/" -e "/dev/" | sort > /tmp/test1
			./id3 -0 -q "%_p%_f" "$files" 2> /dev/null | grep -v -e "/proc/" -e "/dev/" | sort > /tmp/test2
			diff -q /tmp/test1 /tmp/test2 || exit 1
		fi
		break
	done
done
