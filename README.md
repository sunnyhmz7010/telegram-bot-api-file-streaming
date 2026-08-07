<div align="center">
  <!-- <img src="assets/logo.png" alt="telegram-bot-api-file-streaming Logo" width="120" /> -->
  <h1>telegram-bot-api-file-streaming</h1>
  <p>为 Telegram Bot API 增加文件流式下载端点。</p>
</div>

<p align="center">
  <a href="https://github.com/sunnyhmz7010/telegram-bot-api-file-streaming/releases"><img src="https://img.shields.io/github/v/release/sunnyhmz7010/telegram-bot-api-file-streaming?label=Release&color=3b82f6" alt="Release" /></a>
  <a href="https://github.com/sunnyhmz7010/telegram-bot-api-file-streaming/blob/main/LICENSE"><img src="https://img.shields.io/github/license/sunnyhmz7010/telegram-bot-api-file-streaming?color=10b981" alt="License" /></a>
  <a href="https://github.com/sunnyhmz7010/telegram-bot-api-file-streaming/actions/workflows/docker.yml"><img src="https://img.shields.io/github/actions/workflow/status/sunnyhmz7010/telegram-bot-api-file-streaming/docker.yml?branch=main&label=Docker" alt="Docker" /></a>
</p>

---

## ✨ 为什么做这个项目

官方 Telegram Bot API Server 的标准文件下载路径适合常规场景，但在冷文件或大文件下载时，调用方通常需要等待本地文件准备完成后再开始传输。本项目在官方实现基础上新增独立的流式文件端点，让服务端可以在 TDLib 持续下载文件时，同步把已经连续可读的字节发送给 HTTP 客户端。

## 🚀 核心能力

- 新增 `/stream/file/bot<TOKEN>/<FILE_ID>` 流式文件下载端点。
- 使用 Telegram Bot API 对象中的 `file_id`，不要求先拿到 `file_path`。
- 默认关闭，通过 `--enable-file-streaming` 显式启用。
- 保持标准 Bot API 方法行为不变，不修改 `getFile`。
- Docker 镜像默认启用文件流式下载端点并监听 `8081`。
- GitHub Actions 仅手动触发，且只构建 `linux/amd64` 镜像。

## ⚡ 快速开始

### 📋 前置要求

- Docker 24 或更新版本
- Telegram `api_id` 与 `api_hash`，可从 `https://my.telegram.org` 获取
- BotFather 发放的 Bot Token

### 📦 安装与运行

```bash
docker build -t telegram-bot-api-file-streaming .
docker run --rm -p 8081:8081 \
  -e TELEGRAM_API_ID=<API_ID> \
  -e TELEGRAM_API_HASH=<API_HASH> \
  telegram-bot-api-file-streaming
```

## 📖 使用说明

启动后，请用 Bot API 返回的 `file_id` 请求流式端点：

```bash
curl --fail --output file.bin \
  "http://127.0.0.1:8081/stream/file/bot<BOT_TOKEN>/<URL_ENCODED_FILE_ID>"
```

如 TDLib 尚不知道历史文件的精确大小，可以由可信后端补充文件大小请求头：

```bash
curl --fail \
  --header "X-Telegram-File-Size: 10485760" \
  --output file.bin \
  "http://127.0.0.1:8081/stream/file/bot<BOT_TOKEN>/<URL_ENCODED_FILE_ID>"
```

完整调用约束、错误码和客户端示例见 [流式文件端点调用指南.md](./流式文件端点调用指南.md)。

## 🧠 功能细节

流式端点会为单个文件启动一次 TDLib 标准整文件下载，并从 TDLib 本地缓存文件中读取连续可用前缀输出给 HTTP 客户端。该实现不调用 `readFilePart`，避免把 HTTP 分块转换成高频 Telegram DC 分片请求。

当前版本只支持完整顺序 `GET` 响应，不支持 `Range`、断点续传、`HEAD` 或多区间响应。调用方必须校验 `Content-Length` 与实际接收字节数，连接提前关闭时应丢弃临时文件并重试。

## 🧱 技术栈

- C++：Telegram Bot API Server 与 TDLib
- CMake：源码构建
- Docker：Alpine 多阶段镜像构建
- GitHub Actions：手动触发 amd64 镜像构建
- 目标平台：`linux/amd64`

## 🗂️ 项目结构

```text
telegram-bot-api-file-streaming/
├── server/                         # Bot API Server 与文件流式端点实现
├── td/                             # TDLib 源码
├── .github/workflows/docker.yml    # 手动触发的 amd64 Docker 镜像构建
├── Dockerfile                      # Alpine 多阶段构建镜像
├── CMakeLists.txt                  # CMake 构建入口
├── 流式文件端点调用指南.md          # 流式端点集成说明
├── LICENSE                         # Boost Software License 1.0
└── SECURITY.md                     # 安全报告方式
```

## 👨‍💻 本地开发

### 🧰 环境

本地开发推荐使用 Linux 或 WSL2。需要安装 C++ 编译器、CMake、OpenSSL、zlib、Python 3、gperf 和 Docker。

### ⚙️ 命令

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
docker build -t telegram-bot-api-file-streaming .
```

## 🔐 安全报告

如果发现安全问题，请不要公开披露细节。请优先参考仓库中的 [SECURITY.md](./SECURITY.md) 提交安全报告。

## 📄 许可证

本项目基于 [Boost Software License 1.0](./LICENSE) 开源，保留上游 Telegram Bot API Server 与 TDLib 的许可证声明。

<div align="center">
  <sub>Built with ❤️ by Sunny</sub>
</div>
