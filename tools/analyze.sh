#!/bin/bash

dir="/tmp/remu_test_dir/log/"

cat ${dir}/remu.log | grep -E "state_change|event_start|state_begin|event_recv|event_send|event_timer|event_other|state_end" | awk '{if (last != $1) {if (NR != 1) print ""; last = $1} print}' > ${dir}/remu.log.sm
cat ${dir}/remu.log | grep "raft-tick:" > ${dir}/remu.log.rafttick
cat ${dir}/remu.log | grep "remu-tick:" | awk '{if (last != $1) {if (NR != 1) print ""; last = $1} print}' > ${dir}/remu.log.remutick
