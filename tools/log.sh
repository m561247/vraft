cat remu.log | grep -E "state_begin|event_recv|event_send|event_timer|state_end" | awk '{if (last != $1) {if (NR != 1) print ""; last = $1} print}' > remu.log.sm
cat remu.log | grep "raft-tick:" > remu.log.rafttick
cat remu.log | grep "remu-tick:" | awk '{if (last != $1) {if (NR != 1) print ""; last = $1} print}' > remu.log.remutick
