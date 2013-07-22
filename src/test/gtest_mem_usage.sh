#!/bin/bash

HELP_NOTIFICATION="use -h argument for detailed information"

function usage {
cat << END
$(basename "$0") -e gtest_exec [-h] [-f \"filter[ filter...]\"] -- script to calculate peak memory usage of google unit tests

where:
	-e  path to google unit tests executable file
	-h  show this help text
	-f  filters for separate tests measurments
END
}

TEST_EXEC=""
TESTS_STR=""
while getopts ':he:f:' option; do
	case "$option" in
		h) usage
			exit
			;;
		e) TEST_EXEC=$OPTARG
			;;
		f) TESTS_STR=$OPTARG
			;;
		:) echo >&2 "missing argument for -$OPTARG"
			echo >&2 "$HELP_NOTIFICATION"
			exit 1
			;;
		\?) echo >&2 "illegal option: -$OPTARG"
			echo >&2 "$HELP_NOTIFICATION"
			exit 1
			;;
	esac
done

if [ -z "$TEST_EXEC" ]
then
	echo test executable should be provided
	echo $HELP_NOTIFICATION
	exit 1
fi

PROFILER_COMMAND="valgrind --tool=massif --pages-as-heap=yes --massif-out-file=massif.out"

TESTS=()
for TEST in $TESTS_STR
do
	TESTS+=($TEST)
done


MEM_USAGE=()

if [ -z "$TESTS_STR" ]
then
		$PROFILER_COMMAND $TEST_EXEC
		MEM_USAGE=`cat massif.out | grep mem_heap_B | sed -e 's/mem_heap_B=\(.*\)/\1/' | sort -g | tail -n 1`
		printf "\nPeak memory usage: $MEM_USAGE \n"
else
	for TEST in ${TESTS[@]}
	do
		$PROFILER_COMMAND $TEST_EXEC --gtest_filter=$TEST
		MEM_USAGE+=(`cat massif.out | grep mem_heap_B | sed -e 's/mem_heap_B=\(.*\)/\1/' | sort -g | tail -n 1`)
	done

	echo
	echo Peak memory usage per test:
	for i in $(seq 0 $((${#MEM_USAGE[@]}-1)))
	do
		echo ${TESTS[$i]} memory usage: ${MEM_USAGE[$i]}
	done
fi

# vim: set noet ts=4 sw=4:
