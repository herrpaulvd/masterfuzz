#!/bin/bash -x

. variables.sh

workspace="build/test"
tempfile1="$workspace/temp1.txt"
tempfile2="$workspace/temp2.txt"
bincode="$workspace/code.bin"
source1="$workspace/source1.cpp"
source2="$workspace/source2.cpp"
prg="$workspace/prg"
compiler="ntox86-g++-8.3.0 -O2 -fPIE -Warray-bounds -Wdiv-by-zero \
    -Wshift-count-negative -Wshift-count-overflow -fstack-protector-strong \
    -D_FORTIFY_SOURCE=2 -Wclobbered -fwrapv -fwrapv-pointer -fno-strict-aliasing \
    -fno-delete-null-pointer-checks -fno-builtin"

generator="build/Generators/SimpleGenerator/SimpleGenerator 256"
decoder="build/Decoders/UB/DecoderUB 0"
checker="build/Checkers/EqualityChecker/EqualityChecker $tempfile1 $tempfile2"
fuzzer="build/SimpleFuzzer/SimpleFuzzer"

ninja -C build || exit 1
mkdir -p $workspace
rm $workspace/*
rm -r $workspace/*
time $fuzzer "$generator" "$decoder" "$checker" "$bincode" "$source1" "$source2" "$prg" "$compiler"

export SCP=
export SSH=
export REMOTE=
