#!/bin/sh
PGM="`basename $0`"
USAGE="$PGM: usage: $PGM [-x] [-p <n>] [-f <n> <n>] [-d <dir>] [<algorithm> ...]"
TMPDIR=/tmp/plottimes-$$
#trap "rm -rf $TMPDIR" 0
mkdir $TMPDIR
X=false
FILTER=false
DIR="bench"
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
  -d) DIR=$2
      shift 2
      ;;
  -*) echo "$USAGE" >&2
      exit 1
      ;;
  *)  break
      ;;
  esac
done

if [ $# -eq 0 ]
then
  set `reverse algorithms | awk '{print $1;}'`
else
  for a in "$@"
  do
    if awk "\$1==\"$a\" {found = 1;}
	    END {exit(found);}" algorithms >/dev/null
    then
	echo "unknown algorithm $a" >&2
	exit 1
    fi
  done
fi

$X || echo 'set terminal postscript eps'
$FILTER && echo "set key left"
cat <<'EOF'
set ylabel 'time (secs for 1000 steps)'
set xlabel 'particles'
set key box
set style data linespoints
plot \
EOF
( cd "$DIR"
  for p in *.plot
  do
    if $FILTER
    then
      awk "\$1>=$FILTERMIN&&\$1<=$FILTERMAX{print \$0;}" <$p
    else
      cat $p
    fi > $TMPDIR/$p
    [ -s $TMPDIR/$p ] || rm -f $TMPDIR/$p
  done
)
for a in "$@"
do
  if [ -f $TMPDIR/$a.plot ]
  then
    awk "\$1==\"$a\"" algorithms | (
      read aa t
      echo "'-' title '$t'"
    )
  fi
done | sed -e 's/$/, \\/' -e '$s/, \\$//'
for a in "$@"
do
  if [ -f $TMPDIR/$a.plot ]
  then
      cat $TMPDIR/$a.plot
      echo 'e'
  fi
done
$X && echo 'pause -1'
