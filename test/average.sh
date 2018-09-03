#!/bin/bash

if [ "$#" -lt 4 ]
then
    echo "<MSG>: "$0" <delimeter> <create> <output> <executable>"
    exit 1
fi

delimeter="$1";
create="$(echo "$2" | tr [:upper:] [:lower:])";
output="$3";
executable="$4"; shift 4;

if [ ! -x "$executable" ]
then
    echo "<ERR>: "$executable" is not an executable file"
    exit 2
fi

if [ "$create" = "yes" ]
then
    "$executable" "$@" > "$output"
fi

if [ ! -w "$output" ]
then
    echo "<ERR>: "$output" is not a writeable file"
    exit 3
fi

total=0
count=0
ratios=$(grep "$delimeter" "$output" | cut -f2 -d ":")

if [ -z "$ratios" ]
then
    echo "<ERR>: "$delimeter" does not appear anywhere inside "$output""
    exit 4
fi

for ratio in $ratios
do
    total=$(echo $total + $ratio | bc); ((count++));
done

echo -n "Average of \"${delimeter}\" flagged values read from ${output}: "
echo "scale = 2; $total / $count" | bc
