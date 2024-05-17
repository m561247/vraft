# 声明编译器
CXX := g++
# 基础编译参数
CXXFLAGS := -g -Wall -std=c++14

INCLUDES := -Isrc/raft -Isrc/seda -Isrc/util -Isrc/test -Ithird_party/spdlog.v1.13.0/include -Ithird_party/cxxopts.v3.2.0/include -Ithird_party/googletest.v1.14.0/googletest/include -Ithird_party/leveldb.1.22/include -Ithird_party/libuv.v1.40.0/include -Ithird_party/nlohmann_json.v3.10.0/single_include

# 链接参数
LDFLAGS := third_party/libuv.v1.40.0/.libs/libuv.a third_party/googletest.v1.14.0/build/lib/libgtest.a third_party/googletest.v1.14.0/build/lib/libgtest_main.a third_party/leveldb.1.22/build/libleveldb.a -pthread -ldl

# 定位源代码文件夹
SRC_DIRS := src/raft src/seda src/util

# 找出所有的源代码文件
COMMON_SRCS := $(wildcard $(SRC_DIRS:=/*.cc))
VRAFT_SERVER_SRCS := src/main/vraft_server.cc $(COMMON_SRCS)
RAFT_LOG_TOOL_SRCS := src/main/raft_log_tool.cc $(COMMON_SRCS)
ECHO_SERVER_SRCS := src/main/echo_server.cc $(COMMON_SRCS)
ECHO_CLIENT_SRCS := src/main/echo_client.cc $(COMMON_SRCS)

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

# 生成所有.o文件的列表
VRAFT_SERVER_OBJECTS := $(VRAFT_SERVER_SRCS:.cc=.o)
RAFT_LOG_TOOL_OBJECTS := $(RAFT_LOG_TOOL_SRCS:.cc=.o)
ECHO_SERVER_OBJECTS := $(ECHO_SERVER_SRCS:.cc=.o)
ECHO_CLIENT_OBJECTS := $(ECHO_CLIENT_SRCS:.cc=.o)

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

# 可执行文件列表
EXE := vraft_server raft_log_tool echo_server echo_client 
TEST := logger_test ping_test raft_log_test solid_data_test util_test json_test request_vote_test request_vote_reply_test append_entries_test coding_test append_entries_reply_test

# 默认目标
all: $(EXE) $(TEST)

# 模式规则，用于从.cc文件编译成.o文件
%.o: %.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -o $@

# 每个可执行文件的构建规则
vraft_server: $(VRAFT_SERVER_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@

raft_log_tool: $(RAFT_LOG_TOOL_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@

echo_server: $(ECHO_SERVER_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@

echo_client: $(ECHO_CLIENT_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@

logger_test: $(LOGGER_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@

ping_test: $(PING_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@

raft_log_test: $(RAFT_LOG_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@

solid_data_test: $(SOLID_DATA_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@

util_test: $(UTIL_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@

json_test: $(JSON_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@

request_vote_test: $(REQUEST_VOTE_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@	

request_vote_reply_test: $(REQUEST_VOTE_REPLY_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@		

append_entries_test: $(APPEND_ENTRIES_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@	

coding_test: $(CODING_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@	

append_entries_reply_test: $(APPEND_ENTRIES_REPLY_TEST_OBJECTS)
	$(CXX) $(INCLUDES) $(CXXFLAGS) $^ $(LDFLAGS) -o ./output/$@	

# 清理规则
clean:
	rm -f output/vraft_server output/raft_log_tool output/echo_server output/echo_client
	rm -f output/*_test
	find ./src/ -name "*.o" | xargs rm -f

format:
	clang-format --style=Google -i `find ./src -type f \( -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.cpp" \)`

.PHONY: all clean