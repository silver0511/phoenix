#!/usr/bin/env bash
filelist=`ls ./`
rootfbs_t=`pwd`

javafbs=$rootfbs_t"/javafbs"
rm -rd $javafbs
mkdir $javafbs

hfbs=$rootfbs_t"/hfbs"
rm -rd $hfbs
mkdir $hfbs

jsfbs=$rootfbs_t"/jsfbs"
rm -rf $jsfbs
mkdir $jsfbs

gofbs=$rootfbs_t"/gofbs"
rm -rf $gofbs
mkdir $gofbs

pyfbs=$rootfbs_t"/pyfbs"
rm -rf $pyfbs
mkdir $pyfbs

cd fbs
filelist=`ls ./`

for file in $filelist
do 
    temp=$file
    file=$file"/fbs"
    cd $file
    flatc -j -b -o $javafbs ./*
    flatc -g -b -o $gofbs ./*
    flatc -p -b -o $pyfbs ./*

    cpp_path_temp=$hfbs"/"$temp
    mkdir $cpp_path_temp
    flatc -c -b -o $cpp_path_temp ./*

    js_path_temp=$jsfbs"/"$temp
    mkdir $js_path_temp
    flatc -s -b -o $js_path_temp ./*

    cd ..
    cd ..
done
