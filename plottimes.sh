#!/bin/sh
PGM="`basename $0`"
USAGE="$PGM: usage: $PGM [-p <n>]"
FILTER=false
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
  *)  echo "$USAGE" >&2
      exit 1
      ;;
  esac
done

$FILTER && echo "set key left"
cat <<'EOF'
set terminal postscript eps
set ylabel 'time (secs for 1000 steps)'
set xlabel 'particles'
set key box
plot \
     '-' with linespoints title 'BPF with naive resampling', \
     '-' with linespoints title 'BPF with naive resampling and presort', \
     '-' with linespoints title 'BPF with heap-based resampling and presort', \
     '-' with linespoints title 'BPF with heap-based resampling', \
     '-' with linespoints title 'BPF with optimal resampling', \
     '-' with linespoints title 'BPF with regular resampling'
EOF
for a in naive naivesort logmsort logm optimal regular
do
  if $FILTER
  then
    awk "\$1<=$FILTERVAL{print \$0;}" <$a.plot
  else
    cat $a.plot
  fi
  echo 'e'
done
#echo 'pause -1'
