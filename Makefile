ASAN ?= no

ifeq ($(ASAN),yes)
	SANITIZE_FLAGS = -fsanitize=address
else
	SANITIZE_FLAGS =
endif

CXX := g++

CXXFLAGS := -g -Wall -std=c++14 
CXXFLAGS += $(SANITIZE_FLAGS)

INCLUDES := -Isrc/raft -Isrc/seda -Isrc/util -Isrc/test -Isrc/example
INCLUDES += -Ithird_party/spdlog/include 
INCLUDES += -Ithird_party/cxxopts/include 
INCLUDES += -Ithird_party/googletest/googletest/include 
INCLUDES += -Ithird_party/leveldb/include 
INCLUDES += -Ithird_party/libuv/include 
INCLUDES += -Ithird_party/nlohmann_json/single_include

LDFLAGS := third_party/libuv/.libs/libuv.a 
LDFLAGS += third_party/leveldb/build/libleveldb.a 
LDFLAGS += third_party/googletest/build/lib/libgtest.a 
LDFLAGS += third_party/googletest/build/lib/libgtest_main.a 
LDFLAGS += -pthread -ldl


# common src
SRC_DIRS := src/raft src/seda src/util
COMMON_SRCS := $(wildcard $(SRC_DIRS:=/*.cc))

# main src
VRAFT_SERVER_SRCS := src/main/vraft_server.cc $(COMMON_SRCS)
RLOG_TOOL_SRCS := src/main/rlog_tool.cc $(COMMON_SRCS)
META_TOOL_SRCS := src/main/meta_tool.cc $(COMMON_SRCS)
DB_TOOL_SRCS := src/main/db_tool.cc $(COMMON_SRCS)
REMU_SRCS := src/main/remu_main.cc $(COMMON_SRCS)

# example src
ECHO_SERVER_SRCS := src/example/echo_server.cc $(COMMON_SRCS)
ECHO_CLIENT_SRCS := src/example/echo_client.cc $(COMMON_SRCS)

# test src
LOGGER_TEST_SRCS := src/test/logger_test.cc $(COMMON_SRCS)
PING_TEST_SRCS := src/test/ping_test.cc $(COMMON_SRCS)
PING_REPLY_TEST_SRCS := src/test/ping_reply_test.cc $(COMMON_SRCS)
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
REMU_ELECT2_TEST_SRCS := src/test/remu_elect2_test.cc $(COMMON_SRCS)
REMU_PROPOSE_TEST_SRCS := src/test/remu_propose_test.cc $(COMMON_SRCS)
REMU_SM_TEST_SRCS := src/test/remu_sm_test.cc $(COMMON_SRCS)
REMU_SM2_TEST_SRCS := src/test/remu_sm2_test.cc $(COMMON_SRCS)
HOSTPORT_TEST_SRCS := src/test/hostport_test.cc $(COMMON_SRCS)
BUFFER_TEST_SRCS := src/test/buffer_test.cc $(COMMON_SRCS)
EVENTLOOP_TEST_SRCS := src/test/eventloop_test.cc $(COMMON_SRCS)
TIMER_TEST_SRCS := src/test/timer_test.cc $(COMMON_SRCS)
ACCEPTOR_TEST_SRCS := src/test/acceptor_test.cc $(COMMON_SRCS)
TCP_CONNECTION_TEST_SRCS := src/test/tcp_connection_test.cc $(COMMON_SRCS)
TCP_SERVER_TEST_SRCS := src/test/tcp_server_test.cc $(COMMON_SRCS)
CONNECTOR_TEST_SRCS := src/test/connector_test.cc $(COMMON_SRCS)
TCP_CLIENT_TEST_SRCS := src/test/tcp_client_test.cc $(COMMON_SRCS)
RAFT_SERVER_TEST_SRCS := src/test/raft_server_test.cc $(COMMON_SRCS)
COUNT_DOWN_TEST_SRCS := src/test/count_down_test.cc $(COMMON_SRCS)
WORK_THREAD_TEST_SRCS := src/test/work_thread_test.cc $(COMMON_SRCS)
LOOP_THREAD_TEST_SRCS := src/test/loop_thread_test.cc $(COMMON_SRCS)

# remu test src

# generate .o
# main
VRAFT_SERVER_OBJECTS := $(VRAFT_SERVER_SRCS:.cc=.o)
RLOG_TOOL_OBJECTS := $(RLOG_TOOL_SRCS:.cc=.o)
META_TOOL_OBJECTS := $(META_TOOL_SRCS:.cc=.o)
DB_TOOL_OBJECTS := $(DB_TOOL_SRCS:.cc=.o)
REMU_OBJECTS := $(REMU_SRCS:.cc=.o)

# example
ECHO_SERVER_OBJECTS := $(ECHO_SERVER_SRCS:.cc=.o)
ECHO_CLIENT_OBJECTS := $(ECHO_CLIENT_SRCS:.cc=.o)

# test
LOGGER_TEST_OBJECTS := $(LOGGER_TEST_SRCS:.cc=.o)
PING_TEST_OBJECTS := $(PING_TEST_SRCS:.cc=.o)
PING_REPLY_TEST_OBJECTS := $(PING_REPLY_TEST_SRCS:.cc=.o)
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
REMU_ELECT2_TEST_OBJECTS := $(REMU_ELECT2_TEST_SRCS:.cc=.o)
REMU_PROPOSE_TEST_OBJECTS := $(REMU_PROPOSE_TEST_SRCS:.cc=.o)
REMU_SM_TEST_OBJECTS := $(REMU_SM_TEST_SRCS:.cc=.o)
REMU_SM2_TEST_OBJECTS := $(REMU_SM2_TEST_SRCS:.cc=.o)
HOSTPORT_TEST_OBJECTS := $(HOSTPORT_TEST_SRCS:.cc=.o)
BUFFER_TEST_OBJECTS := $(BUFFER_TEST_SRCS:.cc=.o)
EVENTLOOP_TEST_OBJECTS := $(EVENTLOOP_TEST_SRCS:.cc=.o)
TIMER_TEST_OBJECTS := $(TIMER_TEST_SRCS:.cc=.o)
ACCEPTOR_TEST_OBJECTS := $(ACCEPTOR_TEST_SRCS:.cc=.o)
TCP_CONNECTION_TEST_OBJECTS := $(TCP_CONNECTION_TEST_SRCS:.cc=.o)
TCP_SERVER_TEST_OBJECTS := $(TCP_SERVER_TEST_SRCS:.cc=.o)
CONNECTOR_TEST_OBJECTS := $(CONNECTOR_TEST_SRCS:.cc=.o)
TCP_CLIENT_TEST_OBJECTS := $(TCP_CLIENT_TEST_SRCS:.cc=.o)
RAFT_SERVER_TEST_OBJECTS := $(RAFT_SERVER_TEST_SRCS:.cc=.o)
COUNT_DOWN_TEST_OBJECTS := $(COUNT_DOWN_TEST_SRCS:.cc=.o)
WORK_THREAD_TEST_OBJECTS := $(WORK_THREAD_TEST_SRCS:.cc=.o)
LOOP_THREAD_TEST_OBJECTS := $(LOOP_THREAD_TEST_SRCS:.cc=.o)


# generate exe
MAIN := vraft_server rlog_tool meta_tool db_tool remu 
EXAMPLE := echo_server echo_client 

TEST := tpl_test
TEST += logger_test 
TEST += ping_test 
TEST += ping_reply_test 
TEST += raft_log_test 
TEST += solid_data_test 
TEST += util_test 
TEST += json_test 
TEST += request_vote_test 
TEST += request_vote_reply_test 
TEST += append_entries_test 
TEST += coding_test 
TEST += append_entries_reply_test 
TEST += tracer_test 
TEST += raft_test  
TEST += remu_elect_test
TEST += remu_elect2_test
TEST += remu_propose_test 
TEST += remu_sm_test 
TEST += remu_sm2_test
TEST += hostport_test
TEST += buffer_test
TEST += eventloop_test
TEST += timer_test
TEST += acceptor_test
TEST += tcp_connection_test
TEST += tcp_server_test
TEST += connector_test
TEST += tcp_client_test
TEST += raft_server_test
TEST += count_down_test
TEST += work_thread_test
TEST += loop_thread_test


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

meta_tool: $(META_TOOL_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/main/$@

db_tool: $(DB_TOOL_OBJECTS)
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

ping_reply_test: $(PING_REPLY_TEST_OBJECTS)
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

remu_elect2_test: $(REMU_ELECT2_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

remu_propose_test: $(REMU_PROPOSE_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

remu_sm_test: $(REMU_SM_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

remu_sm2_test: $(REMU_SM2_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

hostport_test: $(HOSTPORT_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

buffer_test: $(BUFFER_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

eventloop_test: $(EVENTLOOP_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

timer_test: $(TIMER_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

acceptor_test: $(ACCEPTOR_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

tcp_connection_test: $(TCP_CONNECTION_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

tcp_server_test: $(TCP_SERVER_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

connector_test: $(CONNECTOR_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

tcp_client_test: $(TCP_CLIENT_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

raft_server_test: $(RAFT_SERVER_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

count_down_test: $(COUNT_DOWN_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

work_thread_test: $(WORK_THREAD_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/test/$@

loop_thread_test: $(LOOP_THREAD_TEST_OBJECTS)
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