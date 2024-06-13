const fs = require('fs');
const readline = require('readline');

// 创建读取流
const readStream = fs.createReadStream('./remu.log.global');

// 创建写入流
const writeStream = fs.createWriteStream('./output.html', { flags: 'w+' });

// 使用 readline 处理逐行读取
const rl = readline.createInterface({
  input: readStream,
  crlfDelay: Infinity  // 此设置支持 Windows (CRLF) 和 UNIX (LF) 系统的换行模式
});

var state = "finish";
var temp_json_objs = new Object();
var last_json_objs = new Object();
var last_ready = false;

const td_str = "\t\t<td>";
const td_change_str = "\t\t<td class=\"change\">";
const td3_str = "\t\t<td colspan=\"3\">"
const td3_change_str = "\t\t<td class=\"change\" colspan=\"3\">"
const td_end_str = "</td>\n";

rl.on('line', (line) => {
  // console.log(line);  // 在控制台打印每一行
  // writeStream.write(line + '\n');  // 将每一行写入文件，添加换行符
  
  if (state == "finish") {
    if (line != "") {
        writeStream.write("<table>\n");

        writeStream.write("\t<tr>\n");
        writeStream.write("\t\t<td class=\"table-title\" colspan=\"5\">");
        writeStream.write(line);
        writeStream.write("</td>\n");
        writeStream.write("\t</tr>\n");

        writeStream.write("\t<tr>\n");
        writeStream.write("\t\t<td class=\"split\" colspan=\"5\">");
        writeStream.write("</td>\n");
        writeStream.write("\t</tr>\n");

        state = "data";
    }

  } else if (state == "data") {
    if (line != "") {
        var parts = line.split(' ');
        var unique_id = parts[0];
        var json_str = parts[2];
        var json_obj = JSON.parse(json_str);

        // generate one node
        // line 1
        writeStream.write("\t<tr>\n");

        writeStream.write("\t\t<td class=\"node\">");
        var keys = Object.keys(json_obj);
        raftid = keys[0];
        writeStream.write(raftid)
        writeStream.write(td_end_str);

        // save last one
        temp_json_objs[raftid] = json_obj[raftid];

        var raft_state = json_obj[raftid][0];
        if (last_ready && raft_state != last_json_objs[raftid][0]) {
            writeStream.write(td_change_str);
        } else {
            writeStream.write(td_str);
        }
        writeStream.write(raft_state)
        writeStream.write(td_end_str);

        writeStream.write(td3_str);
        var raft_ptr = json_obj[raftid][1][1];
        writeStream.write(raft_ptr);
        writeStream.write(td_end_str);

        writeStream.write("\t</tr>\n");

        // line 2
        writeStream.write("\t<tr>\n");

        var term = json_obj[raftid][1][0][0]["term"];
        writeStream.write(td_str);
        writeStream.write("\"term\":" + term);
        writeStream.write(td_end_str);

        writeStream.write(td_str);
        var vote = json_obj[raftid][1][0][0]["vote"];
        writeStream.write("\"vote\":" + vote);
        writeStream.write(td_end_str);

        writeStream.write(td3_str);
        var log = json_obj[raftid][1][0][1]["log"];
        writeStream.write("\"log\":" + JSON.stringify(log));
        writeStream.write(td_end_str);

        writeStream.write("\t</tr>\n");

        // line 3
        writeStream.write("\t<tr>\n");

        writeStream.write(td_str);
        var apply = json_obj[raftid][1][0][2]["apply"];
        writeStream.write("\"apply\":" + apply);
        writeStream.write(td_end_str);

        writeStream.write(td_str);
        var cmt = json_obj[raftid][1][0][2]["cmt"];
        writeStream.write("\"cmt\":" + cmt);
        writeStream.write(td_end_str);

        writeStream.write(td_str);
        var elect_ms = json_obj[raftid][1][0][2]["elect_ms"];
        writeStream.write("\"elect_ms\":" + JSON.stringify(elect_ms));
        writeStream.write(td_end_str);

        writeStream.write(td_str);
        var leader = json_obj[raftid][1][0][2]["leader"];
        writeStream.write("\"leader\":" + leader);
        writeStream.write(td_end_str);

        writeStream.write(td_str);
        var run = json_obj[raftid][1][0][2]["run"];
        writeStream.write("\"run\":" + run);
        writeStream.write(td_end_str);

        writeStream.write("\t</tr>\n");

        // line 4
        var peers = Object.keys(json_obj[raftid][1][0][3]);
        for (let i = 0; i < peers.length; i++) {
            var peer = peers[i];
            writeStream.write("\t<tr>\n");

            writeStream.write(td_str);
            writeStream.write(peer);
            writeStream.write(td_end_str);

            writeStream.write(td_str);
            var match = json_obj[raftid][1][0][3][peer][0]["match"];
            writeStream.write("\"match\":" + match);
            writeStream.write(td_end_str);

            writeStream.write(td_str);
            var next = json_obj[raftid][1][0][3][peer][0]["next"];
            writeStream.write("\"next\":" + next);
            writeStream.write(td_end_str);

            writeStream.write(td_str);
            var done = json_obj[raftid][1][0][3][peer][1]["done"];
            writeStream.write("\"done\":" + done);
            writeStream.write(td_end_str);

            writeStream.write(td_str);
            var grant = json_obj[raftid][1][0][3][peer][1]["grant"];
            writeStream.write("\"grant\":" + grant);
            writeStream.write(td_end_str);

            writeStream.write("\t</tr>\n");
        }

        // footer
        writeStream.write("\t<tr>\n");
        writeStream.write("\t\t<td class=\"split\" colspan=\"5\">");
        writeStream.write("</td>\n");
        writeStream.write("\t</tr>\n");

    } else {
        writeStream.write("</table>\n");
        state = "finish";

        last_json_objs = temp_json_objs;
        last_ready = true;

        temp_json_objs = {};
    }
  } 
});

rl.on('close', () => {
  //console.log('File reading and writing completed.');
  writeStream.end();  // 关闭写入流
});

// 错误处理
readStream.on('error', error => {
  console.error('Error reading the input file:', error);
});
writeStream.on('error', error => {
  console.error('Error writing to the output file:', error);
});