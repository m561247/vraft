CXX := g++
CXXFLAGS := -g -Wall -std=c++14
INCLUDES := -Isrc/raft -Isrc/seda -Isrc/util -Isrc/test -Ithird_party/spdlog.v1.13.0/include -Ithird_party/cxxopts.v3.2.0/include -Ithird_party/googletest.v1.14.0/googletest/include -Ithird_party/leveldb.1.22/include -Ithird_party/libuv.v1.40.0/include -Ithird_party/nlohmann_json.v3.10.0/single_include
LDFLAGS := third_party/libuv.v1.40.0/.libs/libuv.a third_party/googletest.v1.14.0/build/lib/libgtest.a third_party/googletest.v1.14.0/build/lib/libgtest_main.a third_party/leveldb.1.22/build/libleveldb.a -pthread -ldl


# common src
SRC_DIRS := src/raft src/seda src/util
COMMON_SRCS := $(wildcard $(SRC_DIRS:=/*.cc))

# main src
VRAFT_SERVER_SRCS := src/main/vraft_server.cc $(COMMON_SRCS)
RLOG_TOOL_SRCS := src/main/rlog_tool.cc $(COMMON_SRCS)
REMU_SRCS := src/main/remu.cc $(COMMON_SRCS)

# example src
ECHO_SERVER_SRCS := src/example/echo_server.cc $(COMMON_SRCS)
ECHO_CLIENT_SRCS := src/example/echo_client.cc $(COMMON_SRCS)

# test src
LOGGER_TEST_SRCS := src/test/logger_test.cc $(COMMON_SRCS)
PING_TEST_SRCS := src/test/ping_test.cc $(COMMON_SRCS)
RAFT_LOG_TEST_SRCS := src/test/raft_log_test.cc $(COMMON_SRCS)
SOLID_DATA_TEST_SRCS := src/test/solid_data_test.cc $(COMMON_SRCS)
UTIL_TEST_SRCS := src/test/util_test.cc $(COMMON_SRCS)
JSON_TEST_SRCS := src/test/json_test.cc $(COMMON_SRCS)
REQUEST_VOTE_TEST_SRCS := src/test/request_vote_test.cc $(COMMON_SRCS)
REQUEST_VOTE_REPLY_TEST_SRCS := src/test/request_vote_reply_test.cc $(COMMON_SRCS)
APPEND_ENTRIES_TEST_SRCS := src/test/append_entries_test.cc $(COMMON_SRCS)
CODING_TEST_SRCS := src/test/coding_test.cc $(COMMON_SRCS)
APPEND_ENTRIES_REPLY_TEST_SRCS := src/test/append_entries_reply_test.cc $(COMMON_SRCS)
TRACER_TEST_SRCS := src/test/tracer_test.cc $(COMMON_SRCS)
RAFT_TEST_SRCS := src/test/raft_test.cc $(COMMON_SRCS)
TPL_TEST_SRCS := src/test/tpl_test.cc $(COMMON_SRCS)
REMU_ELECT_TEST_SRCS := src/test/remu_elect_test.cc $(COMMON_SRCS)

# remu test src

# generate .o
# main
VRAFT_SERVER_OBJECTS := $(VRAFT_SERVER_SRCS:.cc=.o)
RLOG_TOOL_OBJECTS := $(RLOG_TOOL_SRCS:.cc=.o)
REMU_OBJECTS := $(REMU_SRCS:.cc=.o)

# example
ECHO_SERVER_OBJECTS := $(ECHO_SERVER_SRCS:.cc=.o)
ECHO_CLIENT_OBJECTS := $(ECHO_CLIENT_SRCS:.cc=.o)

# test
LOGGER_TEST_OBJECTS := $(LOGGER_TEST_SRCS:.cc=.o)
PING_TEST_OBJECTS := $(PING_TEST_SRCS:.cc=.o)
RAFT_LOG_TEST_OBJECTS := $(RAFT_LOG_TEST_SRCS:.cc=.o)
SOLID_DATA_TEST_OBJECTS := $(SOLID_DATA_TEST_SRCS:.cc=.o)
UTIL_TEST_OBJECTS := $(UTIL_TEST_SRCS:.cc=.o)
JSON_TEST_OBJECTS := $(JSON_TEST_SRCS:.cc=.o)
REQUEST_VOTE_TEST_OBJECTS := $(REQUEST_VOTE_TEST_SRCS:.cc=.o)
REQUEST_VOTE_REPLY_TEST_OBJECTS := $(REQUEST_VOTE_REPLY_TEST_SRCS:.cc=.o)
APPEND_ENTRIES_TEST_OBJECTS := $(APPEND_ENTRIES_TEST_SRCS:.cc=.o)
CODING_TEST_OBJECTS := $(CODING_TEST_SRCS:.cc=.o)
APPEND_ENTRIES_REPLY_TEST_OBJECTS := $(APPEND_ENTRIES_REPLY_TEST_SRCS:.cc=.o)
TRACER_TEST_OBJECTS := $(TRACER_TEST_SRCS:.cc=.o)
RAFT_TEST_OBJECTS := $(RAFT_TEST_SRCS:.cc=.o)
TPL_TEST_OBJECTS := $(TPL_TEST_SRCS:.cc=.o)
REMU_ELECT_TEST_OBJECTS := $(REMU_ELECT_TEST_SRCS:.cc=.o)


# generate exe
MAIN := vraft_server rlog_tool remu 
EXAMPLE := echo_server echo_client 
TEST := logger_test ping_test raft_log_test solid_data_test util_test json_test request_vote_test request_vote_reply_test append_entries_test coding_test append_entries_reply_test tracer_test raft_test tpl_test remu_elect_test


# compile
all: $(MAIN) $(EXAMPLE) $(TEST)
main: $(MAIN) 
example: $(EXAMPLE)
test: $(TEST)


# .cc -> .o
%.o: %.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -o $@


# exe rules
# main
vraft_server: $(VRAFT_SERVER_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/main/$@

rlog_tool: $(RLOG_TOOL_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/main/$@

remu: $(REMU_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/main/$@

# example
echo_server: $(ECHO_SERVER_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/example/$@

echo_client: $(ECHO_CLIENT_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/example/$@

# test
logger_test: $(LOGGER_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

ping_test: $(PING_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

raft_log_test: $(RAFT_LOG_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

solid_data_test: $(SOLID_DATA_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

util_test: $(UTIL_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

json_test: $(JSON_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

request_vote_test: $(REQUEST_VOTE_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@	

request_vote_reply_test: $(REQUEST_VOTE_REPLY_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@		

append_entries_test: $(APPEND_ENTRIES_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@	

coding_test: $(CODING_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@	

append_entries_reply_test: $(APPEND_ENTRIES_REPLY_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@	

tracer_test: $(TRACER_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

raft_test: $(RAFT_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

tpl_test: $(TPL_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

remu_elect_test: $(REMU_ELECT_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

# clean
clean:
	find output/main/ -type f ! -name '*.sh' | xargs rm -f
	find output/example/ -type f ! -name '*.sh' | xargs rm -f
	find output/test/ -type f ! -name '*.sh' | xargs rm -f
	find ./src/ -name "*.o" | xargs rm -f


# format
format:
	clang-format --style=Google -i `find ./src -type f \( -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.cpp" \)`


.PHONY: all clean