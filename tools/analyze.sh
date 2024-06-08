#!/bin/bash

dir="/tmp/remu_test_dir/log/"

cat ${dir}/remu.log | grep -E "state_change|event_start|state_begin|event_recv|event_send|event_timer|event_other|state_end" | awk '{if (last != $1) {if (NR != 1) print ""; last = $1} print}' > ${dir}/remu.log.sm
cat ${dir}/remu.log | grep "raft-tick:" > ${dir}/remu.log.tick
cat ${dir}/remu.log | grep "global-state:" | awk '{if (last != $1) {if (NR != 1) print ""; last = $1} print}' > ${dir}/remu.log.global

rm -rf ${dir}/temp
mkdir ${dir}/temp
cat ${dir}/remu.log.sm | grep state_change | awk '{print $7}' | sort | uniq > ${dir}/temp/ids
for id in `cat ${dir}/temp/ids`; do
    echo "generate log for ${id} ..."
    cat ${dir}/remu.log.sm | grep state_change | grep ${id} | awk '{print $1}' | sort | uniq > ${dir}/temp/keys.${id}
done

for file in `ls ${dir}/temp/keys.*`; do
    echo "analyzing ${file} ..."
    for key in `cat ${file}`; do
        cat ${dir}/remu.log.sm | grep ${key}  >> ${file}.sm.tmp
    done
    cat ${file}.sm.tmp | awk '{if (last != $1) {if (NR != 1) print ""; last = $1} print}' > ${file}.sm
    mv ${file}.sm ${dir}
done