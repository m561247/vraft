cat remu.log | grep -E "state_begin|event_recv|event_send|state_end" | awk '{print; if (NR % 4 == 0) print ""}' > remu.log.sm
cat remu.log | grep "tick:" > remu.log.tick
