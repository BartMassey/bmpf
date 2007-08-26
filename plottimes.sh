#!/bin/sh
PGM="`basename $0`"
USAGE="$PGM: usage: $PGM [-x] [-p <n>] [-f <n> <n>] [<dir>]"
X=false
FILTER=false
DIR=""
while [ $# -gt 0 ]
do
  case $1 in
  -p) FILTER=true
      FILTERMIN=0
      FILTERMAX="$2"
      shift 2
      if echo "$FILTERMAX" | awk "/^[1-9][0-9]*$/{exit(1);}"
      then
        echo "$USAGE" >&2
	exit 1
      fi
      ;;
  -f) FILTER=true
      FILTERMIN="$2"
      FILTERMAX="$3"
      shift 3
      if echo "$FILTERMIN" | awk "/^[1-9][0-9]*$/{exit(1);}" ||
         echo "$FILTERMAX" | awk "/^[1-9][0-9]*$/{exit(1);}"
      then
        echo "$USAGE" >&2
	exit 1
      fi
      ;;
  -x) X=true
      shift
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

$X || echo 'set terminal postscript eps'
$FILTER && echo "set key left"
cat <<'EOF'
set ylabel 'time (secs for 1000 steps)'
set xlabel 'particles'
set key box
set style data linespoints
plot \
EOF
reverse algorithms | 
while read a t
do
     echo "'-' title '$t'"
done | sed -e 's/$/, \\/' -e '$s/, \\$//'
reverse algorithms |
while read a t
do
  if $FILTER
  then
    awk "\$1>=$FILTERMIN&&\$1<=$FILTERMAX{print \$0;}" <"$DIR"/$a.plot
  else
    cat "$DIR"/$a.plot
  fi
  echo 'e'
done
$X && echo 'pause -1'
