#!/bin/bash

rm -rf /tmp/vraft_tools
mkdir -p /tmp/vraft_tools

cp analyze.sh /tmp/vraft_tools/
cp check_remu_meta.sh /tmp/vraft_tools/
cp check_remu_rlog.sh /tmp/vraft_tools/

cp ../output/main/meta_tool /tmp/vraft_tools/
cp ../output/main/rlog_tool /tmp/vraft_tools/
cp ../output/main/db_tool /tmp/vraft_tools/
