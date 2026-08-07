<div align="center">
  <!-- <img src="assets/logo.png" alt="telegram-bot-api-file-streaming Logo" width="120" /> -->
  <h1>telegram-bot-api-file-streaming</h1>
  <p>为 Telegram Bot API 增加大文件实时流式传输能力。</p>
</div>

<p align="center">
  <a href="https://github.com/sunnyhmz7010/telegram-bot-api-file-streaming/blob/main/LICENSE"><img src="https://img.shields.io/github/license/sunnyhmz7010/telegram-bot-api-file-streaming?color=10b981" alt="License" /></a>
</p>

---

## ✨ 为什么做这个项目

把 Telegram 作为文件存储后端的项目不少，但真实落地时通常会遇到两个问题。

第一，官方 Bot API 对文件下载存在大小限制，超过限制的大文件无法直接通过官方接口下载。要承载大文件，通常需要部署自建 Telegram Bot API Server 或其他代理服务。

第二，传统自建 API 代理在处理大文件时，往往需要先把 Telegram 文件完整下载到本地，再把结果返回给业务后端。GB 级文件会带来很长的首次等待时间，用户不能立即开始下载，代理侧也会承担额外的落盘压力。

本项目基于 Telegram Bot API Server 做增强，新增实时流式文件端点，让代理可以边从 Telegram 获取文件数据，边向下游 HTTP 客户端输出文件内容。

## 🚀 核心能力

- 新增 `/stream/file/bot<TOKEN>/<FILE_ID>` 流式文件下载端点。
- 使用 Telegram Bot API 对象中的 `file_id`，不要求先获取 `file_path`。
- 支持边下载边转发，降低大文件首字节等待时间。
- 减少“完整下载完成后再返回”的阻塞流程和额外磁盘占用。
- 默认关闭，通过 `--enable-file-streaming` 显式启用。
- 不修改标准 Bot API 接口行为，理论上兼容官方已有接口。
- Docker 镜像默认启用文件流式端点并监听 `8081`。
- GitHub Actions 仅手动触发，且只构建 `linux/amd64` 镜像。

## ⚡ 快速开始

### 📋 前置要求

- Docker 24 或更新版本
- Telegram `api_id` 与 `api_hash`，可从 `https://my.telegram.org` 获取
- BotFather 发放的 Bot Token
- 一台 `linux/amd64` 环境或可构建 amd64 镜像的机器

### 📦 Docker Compose（推荐）

新建 `compose.yaml`，写入以下内容：

```yaml
services:
  telegram-bot-api-file-streaming:
    image: ghcr.io/sunnyhmz7010/telegram-bot-api-file-streaming:latest
    container_name: telegram-bot-api-file-streaming
    restart: unless-stopped
    ports:
      - "8081:8081"
    environment:
      - TELEGRAM_API_ID=<API_ID>
      - TELEGRAM_API_HASH=<API_HASH>
```

然后启动：

```bash
docker compose up -d
```

查看日志：

```bash
docker compose logs -f
```

### 🖥️ 命令行方式

```bash
docker run -d \
  --name telegram-bot-api-file-streaming \
  --restart unless-stopped \
  -p 8081:8081 \
  -e TELEGRAM_API_ID=<API_ID> \
  -e TELEGRAM_API_HASH=<API_HASH> \
  ghcr.io/sunnyhmz7010/telegram-bot-api-file-streaming:latest
```

### 🛠️ 自行构建镜像

如果想自己构建而不是使用预构建镜像：

```bash
git clone https://github.com/sunnyhmz7010/telegram-bot-api-file-streaming.git
cd telegram-bot-api-file-streaming
docker build -t telegram-bot-api-file-streaming .
docker run --rm -p 8081:8081 \
  -e TELEGRAM_API_ID=<API_ID> \
  -e TELEGRAM_API_HASH=<API_HASH> \
  telegram-bot-api-file-streaming
```

如果用 Docker Compose，把 `compose.yaml` 里的 `image: ghcr.io/...` 换成 `build: .`，然后 `docker compose up -d --build`。

首次源码编译耗时较长，普通环境可能需要 30 分钟以上。GitHub Actions 已配置为手动触发构建，避免无意义地占用 CI 时间。

## 📖 使用说明

启动服务后，使用消息、文件对象或其他 Bot API 返回的 `file_id` 请求流式端点：

```bash
curl --fail --output file.bin \
  "http://127.0.0.1:8081/stream/file/bot<BOT_TOKEN>/<URL_ENCODED_FILE_ID>"
```

如果文件来自 Telegram 测试数据中心，请使用测试路径：

```bash
curl --fail --output file.bin \
  "http://127.0.0.1:8081/stream/file/bot<BOT_TOKEN>/test/<URL_ENCODED_FILE_ID>"
```

`file_id` 必须做 URL 路径段编码，不能直接拼接包含 `/`、`+`、`=`、`%` 等特殊字符的原始值。

如 TDLib 尚不知道历史文件的精确大小，可以由可信业务后端补充文件大小请求头：

```bash
curl --fail \
  --header "X-Telegram-File-Size: 10485760" \
  --output file.bin \
  "http://127.0.0.1:8081/stream/file/bot<BOT_TOKEN>/<URL_ENCODED_FILE_ID>"
```

完整调用约束、错误码、客户端示例和反向代理注意事项见 [流式文件端点调用指南.md](./流式文件端点调用指南.md)。

## 🧠 功能细节

传统流程通常是：

```text
Telegram 服务器
        |
        v
自建 API 代理
        |
        v
完整下载文件
        |
        v
返回文件路径或本地文件
        |
        v
网页后端
```

本项目新增的流式流程是：

```text
Telegram 服务器
        |
        v
增强版 API 代理
        |
        v
实时文件流
        |
        v
网页后端
        |
        v
用户客户端
```

流式端点会为单个文件启动一次 TDLib 标准整文件下载：`downloadFile(file_id, 1, 0, 0, false)`。HTTP 响应读取 TDLib 本地缓存中已经连续可用的前缀数据，并按顺序输出给下游客户端。

该实现不会调用 `readFilePart`，也不会把 HTTP 分块转换成高频 Telegram DC 分片请求。多个 HTTP 消费者请求同一文件时，可以共享同一次 TDLib 下载任务。

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

本项目基于 [Boost Software License 1.0](./LICENSE) 开源。

<div align="center">
  <sub>Built with ❤️ by Sunny</sub>
</div>
