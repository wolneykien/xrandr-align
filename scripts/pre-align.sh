#!/bin/sh -efu

dir="${0%/*}"
ppid=$(ps -p $$ -o ppid= | sed -e 's/[[:space:]]//g')
export ALIGN_TMPDIR="${TMPDIR%/}/xrandr-align.$ppid"
mkdir "$ALIGN_TMPDIR"
flock "$ALIGN_TMPDIR" run-parts "$dir/pre-align.d"
ret=$?
if [ $ret -ne 0 ]; then
	rm -rf "$ALIGN_TMPDIR"
	exit $ret
fi
