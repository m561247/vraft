#include "parser.h"

#include "common.h"
#include "util.h"

namespace vectordb {

const std::string example_cmdstr(VectordbCmd cmd) {
  // char str_buf[1024];
  std::string cmd_str;
  switch (cmd) {
    case kCmdCreateTable: {
      cmd_str.append(
          "create table --name=test-table --partition_num=10 --replica_num=3 "
          "--dim=10");
    }

    default:
      return cmd_str;
  }
}

std::string CmdStr(VectordbCmd cmd) {
  switch (cmd) {
    case kCmdError:
      return "kCmdError";
    case kCmdHelp:
      return "kCmdHelp";
    case kCmdVersion:
      return "kCmdVersion";
    case kCmdQuit:
      return "kCmdQuit";
    case kCmdMeta:
      return "kMeta";
    case kCmdGet:
      return "kCmdGet";
    case kCmdDelete:
      return "kCmdDelete";
    case kCmdGetKNN:
      return "kCmdGetKNN";
    case kCmdCreateTable:
      return "kCmdCreateTable";
    case kCmdBuildIndex:
      return "kCmdBuildIndex";
    case kDescTable:
      return "kDescTable";
    case kDescPartition:
      return "kDescPartition";
    case kDescDescReplica:
      return "kDescDescReplica";
    case kShowTables:
      return "kShowTables";
    case kShowPartitions:
      return "kShowPartitions";
    case kShowReplicas:
      return "kShowReplicas";
    default:
      return "kCmdError";
  }
}

std::string MergeString(const std::vector<std::string> &strs) {
  std::string str;
  for (auto &s : strs) {
    str.append(s).append(" ");
  }
  return str;
}

VectordbCmd GetCmd(const std::string &cmd_line, int *argc, char ***argv) {
  VectordbCmd ret_cmd = kCmdError;
  size_t del_count = 0;
  std::string cmd_line2;
  std::vector<std::string> result;
  vraft::Split(cmd_line, ' ', result);
  if (result.size() == 0) {
    return kCmdError;
  }

  if (result[0] == "create") {
    del_count = 2;
    ret_cmd = kCmdCreateTable;
    std::string cmd_arg2 = "table";

    if (result[1] == cmd_arg2 && result.size() > del_count) {
      result.erase(result.begin(), result.begin() + del_count);
      cmd_line2.append(CmdStr(ret_cmd)).append(" ").append(MergeString(result));
      vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
      return ret_cmd;
    }

  } else if (result[0] == "build") {
    del_count = 2;
    ret_cmd = kCmdBuildIndex;
    std::string cmd_arg2 = "index";

    if (result[1] == cmd_arg2 && result.size() > del_count) {
      result.erase(result.begin(), result.begin() + del_count);
      cmd_line2.append(CmdStr(ret_cmd)).append(" ").append(MergeString(result));
      vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
      return ret_cmd;
    }

  } else if (result[0] == "desc") {
    del_count = 2;
    if (result.size() == 3) {
      if (result[1] == "table") {
        ret_cmd = kDescTable;

        result.erase(result.begin(), result.begin() + del_count);
        cmd_line2.append(CmdStr(ret_cmd))
            .append(" ")
            .append(DESC_NAME)
            .append(result[2]);
        vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
        return ret_cmd;

      } else if (result[1] == "partition") {
        ret_cmd = kDescPartition;

        result.erase(result.begin(), result.begin() + del_count);
        cmd_line2.append(CmdStr(ret_cmd))
            .append(" ")
            .append(DESC_NAME)
            .append(result[2]);
        vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
        return ret_cmd;

      } else if (result[1] == "replica") {
        ret_cmd = kDescDescReplica;

        result.erase(result.begin(), result.begin() + del_count);
        cmd_line2.append(CmdStr(ret_cmd))
            .append(" ")
            .append(DESC_NAME)
            .append(result[2]);
        vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
        return ret_cmd;

      } else {
        return kCmdError;
      }
    }

  } else if (result[0] == "show") {
    if (result.size() == 2) {
      if (result[1] == "tables") {
        ret_cmd = kShowTables;
        cmd_line2.append(CmdStr(ret_cmd));
        vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
        return ret_cmd;
      } else if (result[1] == "partitions") {
        ret_cmd = kShowPartitions;
        cmd_line2.append(CmdStr(ret_cmd));
        vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
        return ret_cmd;
      } else if (result[1] == "replicas") {
        ret_cmd = kShowReplicas;
        cmd_line2.append(CmdStr(ret_cmd));
        vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
        return ret_cmd;
      }
    }

  } else if (result[0] == "help") {
    if (result.size() == 1) {
      ret_cmd = kCmdHelp;
      cmd_line2.append(CmdStr(ret_cmd));
      vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
      return ret_cmd;
    }

  } else if (result[0] == "version") {
    if (result.size() == 1) {
      ret_cmd = kCmdVersion;
      cmd_line2.append(CmdStr(ret_cmd));
      vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
      return ret_cmd;
    }

  } else if (result[0] == "quit") {
    if (result.size() == 1) {
      ret_cmd = kCmdQuit;
      cmd_line2.append(CmdStr(ret_cmd));
      vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
      return ret_cmd;
    }

  } else if (result[0] == "meta") {
    if (result.size() == 1) {
      ret_cmd = kCmdMeta;
      cmd_line2.append(CmdStr(ret_cmd));
      vraft::ConvertStringToArgcArgv(cmd_line2, argc, argv);
      return ret_cmd;
    }

  } else if (result[0] == "put") {
  } else if (result[0] == "get") {
  } else if (result[0] == "del") {
  } else if (result[0] == "getknn") {
  }

  return kCmdError;
}

nlohmann::json Parser::ToJson() {
  nlohmann::json j;
  j["cmd_line"] = cmd_line_;
  j["cmd"] = CmdStr(cmd_);
  for (int32_t i = 0; i < argc_; ++i) {
    j["argvs"][i] = argv_[i];
  }

  j["name"] = name();
  j["partition_num"] = partition_num();
  j["replica_num"] = replica_num();
  j["dim"] = dim();

  return j;
}

nlohmann::json Parser::ToJsonTiny() { return ToJson(); }

std::string Parser::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["parser"] = ToJsonTiny();
  } else {
    j["parser"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

void Parser::Parse() {
  cmd_ = GetCmd(cmd_line_, &argc_, &argv_);

  if (cmd_ != kCmdError) {
    options_ = std::make_shared<cxxopts::Options>(std::string(argv_[0]));
    options_->add_options()("name", "name",
                            cxxopts::value<std::string>()->default_value(""))(
        "partition_num", "partition_num",
        cxxopts::value<int32_t>()->default_value("0"))(
        "replica_num", "replica_num",
        cxxopts::value<int32_t>()->default_value("0"))(
        "dim", "dim", cxxopts::value<int32_t>()->default_value("0"))

        ;

    parse_result_ = std::make_shared<cxxopts::ParseResult>();
    *parse_result_ = options_->parse(argc_, argv_);

    if (parse_result_->count("name")) {
      name_ = (*parse_result_)["name"].as<std::string>();
    }

    if (parse_result_->count("partition_num")) {
      partition_num_ = (*parse_result_)["partition_num"].as<std::int32_t>();
    }

    if (parse_result_->count("replica_num")) {
      replica_num_ = (*parse_result_)["replica_num"].as<std::int32_t>();
    }

    if (parse_result_->count("dim")) {
      dim_ = (*parse_result_)["dim"].as<std::int32_t>();
    }
  }
}

Parser::Parser(const std::string &cmd_line)
    : argc_(0),
      argv_(nullptr),
      cmd_line_(cmd_line),
      name_(""),
      partition_num_(0),
      replica_num_(0),
      dim_(0) {
  Parse();
}

Parser::~Parser() { vraft::FreeArgv(argc_, argv_); }

}  // namespace vectordb
