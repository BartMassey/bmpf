#!/bin/sh
PGM="`basename $0`"
USAGE="$PGM: usage: $PGM [-p <n>] [<dir>]"
FILTER=false
DIR=""
while [ $# -gt 0 ]
do
  case $1 in
  -p) FILTER=true
      FILTERVAL="$2"
      shift 2
      if echo "$FILTERVAL" | awk "/^[1-9][0-9]*$/{exit(1);}"
      then
        echo "$USAGE" >&2
	exit 1
      fi
      ;;
  *)  break
      ;;
  esac
done
case $# in
  0) ;;
  1) DIR="$1" ;;
  *) echo "$USAGE" >&2
     exit 1
     ;;
esac

$FILTER && echo "set key left"
cat <<'EOF'
set terminal postscript eps
set ylabel 'time (secs for 1000 steps)'
set xlabel 'particles'
set key box
set style data linespoints
plot \
EOF
while read a t
do
     echo "'-' title '$t'"
done <algorithms | sed -e 's/$/, \\/' -e '$s/, \\$//'
while read a t
do
  if $FILTER
  then
    awk "\$1<=$FILTERVAL{print \$0;}" <"$DIR"/$a.plot
  else
    cat "$DIR"/$a.plot
  fi
  echo 'e'
done <algorithms
#echo 'pause -1'
