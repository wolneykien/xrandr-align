#!/bin/sh -efu

dir="${0%/*}"
ppid=$(ps -p $$ -o ppid= | sed -e 's/[[:space:]]//g')
export ALIGN_TMPDIR="${TMPDIR%/}/xrandr-align.$ppid"
for i in `seq 0 9`; do
	[ -d "$ALIGN_TMPDIR" ] && break
	sleep 0.1
done
if ! [ -d "$ALIGN_TMPDIR" ]; then
	echo "The directory $ALIGN_TMPDIR doesn't exist" >&2
	exit 1
fi
flock "$ALIGN_TMPDIR" run-parts "$dir/post-align.d"
ret=$?
rm -rf "$ALIGN_TMPDIR"
exit $ret
