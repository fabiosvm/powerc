#!/usr/bin/env bash

n=0
fail=0

for f in examples/*.pwc
do
  echo "$f"
  build/powerc $f
  n=$((n+1))
  if [ $? -ne 0 ]; then
    fail=$((fail+1))
  fi
  echo "----------------------"
done

echo "$n file(s) tested, $fail failed"
