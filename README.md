# 在线判题系统 Online Judge

一个 C++ 实现的在线判题系统，分两个进程：

- `compile_server`：判题机。收到 JSON 里的代码与限制，编译、运行、判定结果，返回 JSON。
- `oj_server`：业务服务器。管理题库、渲染页面、把提交分发给判题机。

## 判题流程

```text
用户提交代码
   │  POST /Compile_Run   {"code":"...","input":"...","limit_cpu":1,"limit_mem":32768}
   ▼
compile_server
   ├─ 写入临时源码文件 <时间戳>_<原子自增>.cpp
   ├─ fork + execlp g++  ──► 编译错误重定向到 .compile_err
   ├─ fork + execl 可执行文件
   │    ├─ setrlimit(RLIMIT_CPU)  限制 CPU 时间
   │    ├─ setrlimit(RLIMIT_AS)   限制地址空间
   │    └─ dup2 把 stdin/stdout/stderr 重定向到独立临时文件
   └─ waitpid 取 status，按低 7 位解析终止信号
   ▼
{"status":0,"reason":"编译运行成功","stdout":"...","stderr":"..."}
```

## 结果码约定

`status` 的取值就是子进程的终止信号编号（`status & 0x7F`）：

| status | 含义 |
|---|---|
| `0` | 正常运行结束 |
| `-1` | 提交的代码为空 |
| `-2` | 系统内部错误（fork / 打开文件失败等） |
| `-3` | 编译错误，详情在 `.compile_err` 文件 |
| `SIGABRT` (6) | 内存超过限制 |
| `SIGXCPU` (24) | CPU 时间超限 |
| `SIGFPE` (8) | 浮点异常 |

## 目录

```text
comm/            日志、路径与文件工具、字符串切分
compile_server/  判题机：compiler / runner / compile_run / http 入口
oj_server/       业务层：model（题库）/ view（ctemplate 渲染）/ control（调度）
oj_server/conf/  判题机列表
```

## 构建

依赖：`g++`（C++11）、`jsoncpp`、`ctemplate`、`boost`（仅用 `boost::split`）。

```bash
# 判题机
g++ -std=c++11 -o compile_server/compile_server compile_server/compile_server.cpp -ljsoncpp -lpthread
./compile_server/compile_server 8081

# 提交一次
curl -X POST http://127.0.0.1:8081/Compile_Run \
  -H 'Content-Type: application/json' \
  -d '{"code":"#include <iostream>\nint main(){std::cout<<1+1;}", "input":"", "limit_cpu":1, "limit_mem":32768}'
```

## 当前状态

判题核心（`compile_server`）已完成并可运行：编译、运行、资源限制、信号判定、结果 JSON 化
都已打通。

业务层（`oj_server`）还在开发中：题库加载与页面渲染已实现，判题机调度
（`LoadBlance::SmartChoice`）与 `judge` 接口尚未完成，因此 `oj_server` 目前还不能完整编译。
这部分补齐后再接入 HTTP 服务与题目数据目录。
