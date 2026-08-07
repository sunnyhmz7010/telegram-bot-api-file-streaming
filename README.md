<div align="center">
  <!-- <img src="assets/logo.png" alt="telegram-bot-api-file-streaming Logo" width="120" /> -->
  <h1>telegram-bot-api-file-streaming</h1>
  <p>为 Telegram Bot API 增加大文件实时流式传输能力</p>
  <p>本项目完全参考自 <a href="https://github.com/lappland22233/tgtc/tree/beta">tgtc</a> 作者的开源仓库及其在 <a href="https://www.nodeseek.com/post-840065-1">NodeSeek</a> 发布的帖子</p>
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
- Docker 镜像通过 `TELEGRAM_FILE_STREAMING=1` 环境变量启用文件流式端点并监听 `8081`。
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
      - TELEGRAM_FILE_STREAMING=1
    volumes:
      - telegram-bot-api-data:/var/lib/telegram-bot-api

volumes:
  telegram-bot-api-data:
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
  -e TELEGRAM_FILE_STREAMING=1 \
  -v telegram-bot-api-data:/var/lib/telegram-bot-api \
  ghcr.io/sunnyhmz7010/telegram-bot-api-file-streaming:latest
```

### 🛠️ 自行构建镜像

如果想自己构建而不是使用预构建镜像：

```bash
git clone https://github.com/sunnyhmz7010/telegram-bot-api-file-streaming.git
cd telegram-bot-api-file-streaming
docker build -t telegram-bot-api-file-streaming .
docker run -d \
  --name telegram-bot-api-file-streaming \
  --restart unless-stopped \
  -p 8081:8081 \
  -e TELEGRAM_API_ID=<API_ID> \
  -e TELEGRAM_API_HASH=<API_HASH> \
  -e TELEGRAM_FILE_STREAMING=1 \
  -v telegram-bot-api-data:/var/lib/telegram-bot-api \
  telegram-bot-api-file-streaming
```

如果用 Docker Compose，把 `compose.yaml` 里的 `image: ghcr.io/...` 换成 `build: .`，然后 `docker compose up -d --build`。

## 📖 使用说明

### ⚙️ 环境变量

容器通过环境变量配置服务，入口脚本会将其转换为 `telegram-bot-api` 启动参数：

| 环境变量 | 必填 | 默认值 | 说明 |
| --- | --- | --- | --- |
| `TELEGRAM_API_ID` | 是 | - | Telegram 应用 `api_id`，可改为 `TELEGRAM_API_ID_FILE` 从文件读取 |
| `TELEGRAM_API_HASH` | 是 | - | Telegram 应用 `api_hash`，可改为 `TELEGRAM_API_HASH_FILE` 从文件读取 |
| `TELEGRAM_WORK_DIR` | 否 | `/var/lib/telegram-bot-api` | 工作目录，建议挂载卷持久化 |
| `TELEGRAM_TEMP_DIR` | 否 | `/tmp/telegram-bot-api` | 临时目录 |
| `TELEGRAM_HTTP_PORT` | 否 | `8081` | HTTP 监听端口 |
| `TELEGRAM_STAT` | 否 | 关闭 | 设为 `1` 启用统计端点（监听 `8082`） |
| `TELEGRAM_FILE_STREAMING` | 否 | 关闭 | 设为 `1` 启用流式文件端点 |
| `TELEGRAM_FILE_STREAM_CHUNK_SIZE` | 否 | `262144` | 单次读取和发送的最大字节数 |
| `TELEGRAM_FILE_STREAM_MAX_CONNECTIONS` | 否 | `100` | 全局同时活动的流式响应数量上限 |
| `TELEGRAM_FILE_STREAM_FIRST_BYTE_TIMEOUT` | 否 | `30` | 首个数据块发送前最长等待时间（秒） |
| `TELEGRAM_FILE_STREAM_IDLE_TIMEOUT` | 否 | `60` | 两个数据块之间允许的最长停滞时间（秒） |
| `TELEGRAM_FILE_STREAM_WRITE_HIGH_WATERMARK` | 否 | `1048576` | 每个连接单次进入受控写入队列的数据上限 |
| `TELEGRAM_LOG_FILE` | 否 | - | 日志文件路径，默认输出到 stdout/stderr |
| `TELEGRAM_FILTER` | 否 | - | 允许 `bot_user_id % modulo == remainder` 的机器人 |
| `TELEGRAM_MAX_WEBHOOK_CONNECTIONS` | 否 | - | 每个机器人默认最大 webhook 连接数 |
| `TELEGRAM_VERBOSITY` | 否 | - | 日志详细程度 |
| `TELEGRAM_MAX_CONNECTIONS` | 否 | - | 最大打开文件描述符数量 |
| `TELEGRAM_PROXY` | 否 | - | 出站 webhook 请求的 HTTP 代理，格式 `http://host:port` |
| `TELEGRAM_HTTP_IP_ADDRESS` | 否 | - | HTTP 监听地址，IPv6 使用 `[::]` |

`_FILE` 变体用于从 Docker Secret 等文件读取敏感值，如 `TELEGRAM_API_ID_FILE=/run/secrets/api_id`。

启用统计端点时同步映射端口 `-p 8082:8082`，然后访问 `http://<HOST>:8082` 查看服务统计。

### ✨ 功能说明

本项目新增了独立的流式文件下载端点，可在 TDLib 尚未完成整文件下载时，将已经连续下载完成的字节按顺序发送给调用方，从而降低冷文件的首字节等待时间。

- 默认关闭，必须通过启动参数显式启用。
- 不修改标准 Bot API `getFile` 方法的行为。
- 接口正常结束时保证响应体字节数等于文件精确大小。
- 文件始终从偏移量 `0` 开始顺序输出，不重复、不跳字节。
- 同一文件的多个调用方共享 TDLib 下载任务，但各自维护独立的输出进度。
- 当前版本仅支持完整文件 `GET`，不支持 `Range`、断点续传、`HEAD` 或多区间请求。

### 🚀 启动服务

最简启动命令：

```powershell
.\telegram-bot-api.exe `
  --api-id=<API_ID> `
  --api-hash=<API_HASH> `
  --enable-file-streaming
```

默认监听端口为 `8081`。如需修改：

```powershell
.\telegram-bot-api.exe `
  --api-id=<API_ID> `
  --api-hash=<API_HASH> `
  --http-port=8081 `
  --enable-file-streaming
```

`API_ID` 和 `API_HASH` 是 Telegram 应用凭据，不是 Bot Token，可从 `https://my.telegram.org` 获取。

完整配置示例：

```powershell
.\telegram-bot-api.exe `
  --api-id=<API_ID> `
  --api-hash=<API_HASH> `
  --http-port=8081 `
  --enable-file-streaming `
  --file-stream-chunk-size=262144 `
  --file-stream-max-connections=100 `
  --file-stream-first-byte-timeout=30 `
  --file-stream-idle-timeout=60 `
  --file-stream-write-high-watermark=1048576
```

配置参数：

| 参数 | 默认值 | 说明 |
| --- | ---: | --- |
| `--enable-file-streaming` | 关闭 | 启用流式文件端点。 |
| `--file-stream-chunk-size` | `262144` | 单次读取和发送的最大字节数，允许范围为 16 KiB～4 MiB。 |
| `--file-stream-max-connections` | `100` | 全局同时活动的流式响应数量上限。 |
| `--file-stream-first-byte-timeout` | `30` | 首个数据块发送前的最长等待时间，单位为秒。 |
| `--file-stream-idle-timeout` | `60` | 已开始传输后，两个数据块之间允许的最长停滞时间，单位为秒。 |
| `--file-stream-write-high-watermark` | `1048576` | 每个连接单次进入受控写入队列的数据上限。 |

### 🔑 获取 `file_id`

流式端点接收 Telegram Bot API 返回的 `file_id`，不是标准 `getFile` 返回的 `file_path`，也不是本地文件路径。

常见来源包括：

- `getUpdates` 返回消息中的 `document.file_id`；
- `message.video.file_id`；
- `message.audio.file_id`；
- `message.photo[n].file_id`；
- 其他 Bot API 对象中的 `file_id` 字段。

示例对象：

```json
{
  "document": {
    "file_name": "example.zip",
    "file_id": "BQACAgIAAxkBAA...",
    "file_unique_id": "AgAD...",
    "file_size": 10485760
  }
}
```

调用时应使用 `file_id`，不能使用 `file_unique_id`。

### 📡 请求格式

正式数据中心：

```text
GET /stream/file/bot<BOT_TOKEN>/<URL_ENCODED_FILE_ID>
```

完整 URL：

```text
http://<HOST>:<PORT>/stream/file/bot<BOT_TOKEN>/<URL_ENCODED_FILE_ID>
```

Telegram 测试数据中心：

```text
GET /stream/file/bot<BOT_TOKEN>/test/<URL_ENCODED_FILE_ID>
```

只有 Bot 和文件来自 Telegram 测试数据中心时才使用 `/test/` 路径。

参数要求：

| 参数 | 说明 |
| --- | --- |
| `BOT_TOKEN` | BotFather 发放的完整 Bot Token，例如 `123456789:AA...`。 |
| `URL_ENCODED_FILE_ID` | 对完整 `file_id` 做 URL 路径段编码后的结果。 |
| `X-Telegram-File-Size` | 可选请求头，十进制正整数，单位为字节。全新 TDLib 工作目录下载历史文件时，应由可信业务后端从上传记录中提供。 |

必须对 `file_id` 进行 URL 编码，尤其不要直接拼接包含 `/`、`+`、`=`、`%` 等特殊字符的值。

大小选择规则：TDLib 已知大小时使用并核对 TDLib 值；TDLib 未知时使用 `X-Telegram-File-Size`；两者不一致返回 `502`；两者都未知也返回 `502 Exact file size is unavailable`。不要让浏览器或公网调用方自由声明大小，端点应仅供业务后端通过内网访问。

Telegram 风控兼容机制：

每个文件只启动一次 TDLib 标准整文件下载：`downloadFile(file_id, 1, 0, 0, false)`。HTTP 流根据 `updateFile` 的连续可读前缀，从 TDLib 本地缓存文件使用本地 `pread` 分块输出；**不会调用 `readFilePart`，也不会把每个 HTTP 分块转换成额外的 MTProto `upload.getFile` 请求**。同一文件的并发 HTTP 消费者共享这一次下载，从而保持流式首字节和背压能力，同时避免非官方式高频 DC 分片访问触发服务端风控或重置连接。

运维验收应确认：源码及运行日志中没有流式路径的 `readFilePart` 调用；单个大文件在 Telegram 侧只表现为一次正常整文件下载。

### 💻 调用示例

PowerShell 下载到文件：

```powershell
$baseUrl = "http://127.0.0.1:8081"
$botToken = "<BOT_TOKEN>"
$fileId = "<FILE_ID>"
$encodedFileId = [System.Uri]::EscapeDataString($fileId)
$url = "$baseUrl/stream/file/bot$botToken/$encodedFileId"

curl.exe --fail --location --output ".\download.bin" $url
```

建议使用系统自带的 `curl.exe`，避免 Windows PowerShell 将 `curl` 解析为 `Invoke-WebRequest` 别名。

PowerShell 使用 `Invoke-WebRequest`：

```powershell
$baseUrl = "http://127.0.0.1:8081"
$botToken = "<BOT_TOKEN>"
$fileId = "<FILE_ID>"
$encodedFileId = [System.Uri]::EscapeDataString($fileId)
$url = "$baseUrl/stream/file/bot$botToken/$encodedFileId"

Invoke-WebRequest -Uri $url -Method Get -OutFile ".\download.bin"
```

JavaScript / Node.js：

```javascript
import { createWriteStream } from "node:fs";
import { pipeline } from "node:stream/promises";

const baseUrl = "http://127.0.0.1:8081";
const botToken = process.env.BOT_TOKEN;
const fileId = "<FILE_ID>";
const fileSize = 10485760;
const url = `${baseUrl}/stream/file/bot${botToken}/${encodeURIComponent(fileId)}`;

const response = await fetch(url, {
  headers: { "X-Telegram-File-Size": String(fileSize) }
});
if (!response.ok) {
  throw new Error(`下载失败：HTTP ${response.status} ${await response.text()}`);
}
if (!response.body) {
  throw new Error("响应体为空");
}

const expectedLength = Number(response.headers.get("content-length"));
let receivedLength = 0;
const counter = new TransformStream({
  transform(chunk, controller) {
    receivedLength += chunk.byteLength;
    controller.enqueue(chunk);
  }
});

await pipeline(
  response.body.pipeThrough(counter),
  createWriteStream("download.bin")
);

if (Number.isFinite(expectedLength) && receivedLength !== expectedLength) {
  throw new Error(`文件不完整：期望 ${expectedLength} 字节，实际 ${receivedLength} 字节`);
}
```

Python：

```python
import os
from urllib.parse import quote

import requests

base_url = "http://127.0.0.1:8081"
bot_token = os.environ["BOT_TOKEN"]
file_id = "<FILE_ID>"
url = f"{base_url}/stream/file/bot{bot_token}/{quote(file_id, safe='')}"

with requests.get(url, stream=True, timeout=(10, 120)) as response:
    response.raise_for_status()
    expected_length = int(response.headers["Content-Length"])
    received_length = 0

    with open("download.bin", "wb") as output:
        for chunk in response.iter_content(chunk_size=256 * 1024):
            if not chunk:
                continue
            output.write(chunk)
            received_length += len(chunk)

    if received_length != expected_length:
        raise RuntimeError(
            f"文件不完整：期望 {expected_length} 字节，实际 {received_length} 字节"
        )
```

安装依赖：

```powershell
python -m pip install requests
```

### ✅ 成功响应

成功请求返回：

```http
HTTP/1.1 200 OK
Content-Type: application/octet-stream
Content-Length: <文件精确字节数>
```

响应体是完整文件的原始二进制内容，不是 JSON，也没有 Bot API 的 `ok` 包装字段。

服务端只有在实际发送的字节数严格等于 `Content-Length` 时才会正常结束响应。

### 🔍 完整性校验

必须检查 `Content-Length`：

如果 Telegram 或 TDLib 在响应头发出后失败，HTTP 状态码无法再次修改。此时服务端会中断连接，调用方必须把以下情况视为下载失败：

- 实际收到的字节数小于 `Content-Length`；
- HTTP 客户端报告连接提前关闭；
- 输出流写入失败；
- 请求超时。

不要在发生上述情况时保留或使用不完整文件。推荐先写入临时文件，校验完成后再原子重命名：

```powershell
$tempFile = ".\download.bin.part"
$finalFile = ".\download.bin"

curl.exe --fail --output $tempFile $url
if ($LASTEXITCODE -ne 0) {
  Remove-Item $tempFile -ErrorAction SilentlyContinue
  throw "下载失败"
}
Move-Item $tempFile $finalFile -Force
```

可选哈希校验：

如果业务侧保存了源文件哈希，可在下载完成后进行 SHA-256 校验：

```powershell
Get-FileHash ".\download.bin" -Algorithm SHA256
```

流式端点保证传输顺序和字节计数，但 Telegram Bot API 本身不会为该端点提供业务侧预期 SHA-256，因此端到端哈希需要由调用方自行保存和比对。

### ⚠️ 错误响应

在流式响应开始前发生的错误使用 Bot API 风格 JSON：

```json
{
  "ok": false,
  "error_code": 400,
  "description": "Bad Request: ..."
}
```

常见状态码：

| 状态码 | 含义 | 建议处理 |
| ---: | --- | --- |
| `400` | 路径、URL 编码或 `file_id` 不合法。 | 修正参数，不要直接重试同一请求。 |
| `401` | Bot Token 无效或认证失败。 | 检查 Token 和调用环境。 |
| `404` | 路由未启用、文件不可用或无法解析。 | 检查启动参数、`file_id` 和数据中心。 |
| `405` | 使用了非 `GET` 方法。 | 改为 `GET`。 |
| `413` | 文件大小超出当前平台可处理范围。 | 更换平台或限制文件大小。 |
| `429` | 活动流数量超过配置上限。 | 指数退避后重试。 |
| `502` | TDLib 或 Telegram 上游下载失败。 | 短暂退避后重试；持续失败时检查文件是否仍可访问。 |
| `503` | Bot API Client 正在关闭。 | 服务恢复后重试。 |
| `504` | 首字节等待或传输停滞超时。 | 检查网络后重试，可按需调整超时参数。 |

响应头发出后的错误不会返回第二个 JSON 错误体，而是直接终止连接。

### 🔄 推荐重试策略

建议仅对 `429`、`502`、`503`、`504` 和网络连接错误进行有限重试：

1. 第一次失败后等待 1 秒；
2. 第二次失败后等待 2 秒；
3. 第三次失败后等待 4 秒；
4. 加入少量随机抖动；
5. 每次重试都重新从完整文件起点下载。

当前版本不支持断点续传，因此重试时不能携带 `Range` 请求头。

### 🚪 反向代理注意事项

如果通过 Nginx、网关或 CDN 暴露端点，应关闭响应缓冲，否则代理可能先缓存大量数据再发送，抵消流式下载降低首字节时间的效果。

Nginx 示例：

```nginx
location /stream/file/bot {
    proxy_pass http://127.0.0.1:8081;
    proxy_http_version 1.1;
    proxy_buffering off;
    proxy_request_buffering off;
    proxy_read_timeout 120s;
    proxy_send_timeout 120s;
}
```

生产环境必须使用 HTTPS，并避免在访问日志、监控、错误页面或前端代码中泄露 Bot Token。由于 Token 位于 URL 路径中，应对代理访问日志进行脱敏或关闭该路径的详细日志。

### ❓ 常见问题

**请求返回 `404`**

确认启动命令包含：

```text
--enable-file-streaming
```

同时确认正式环境没有误用 `/test/` 路径。

**请求返回 `400 Invalid percent-encoding`**

说明 `file_id` 的 URL 编码不合法。应使用语言标准库的路径段编码函数，不要手工替换字符。

**下载立即成功但文件无法打开**

检查调用方是否把二进制响应当成文本或 JSON 处理。必须按二进制流写入文件。

**下载中途断开**

删除临时文件并重新请求。必须比较实际字节数和 `Content-Length`，不能把提前断开的文件当成完整文件。

**能否使用标准 `file_path`**

不能。该端点需要 Telegram Bot API 对象中的 `file_id`。标准下载路径 `/file/bot<TOKEN>/<file_path>` 与本项目新增的 `/stream/file/bot<TOKEN>/<file_id>` 流式路由语义不同。

**是否支持浏览器直接访问**

可以，但不建议将 Bot Token 暴露给浏览器端用户。更安全的方式是由业务后端调用该端点，再通过受控鉴权接口转发给客户端。

### 📋 上线检查清单

- [ ] 服务已使用 `--enable-file-streaming` 启动。
- [ ] Bot 已正确迁移到自建 Bot API 服务。
- [ ] 调用参数使用 `file_id`，不是 `file_unique_id` 或 `file_path`。
- [ ] `file_id` 使用标准 URL 编码函数编码。
- [ ] 调用方按二进制流处理响应。
- [ ] 调用方检查 `Content-Length` 与实际字节数。
- [ ] 下载先写入 `.part` 临时文件，成功后再重命名。
- [ ] 反向代理已关闭响应缓冲。
- [ ] 外部访问使用 HTTPS。
- [ ] 访问日志不会泄露 Bot Token。
- [ ] 仅对可恢复错误实施有限退避重试。

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
├── docker-entrypoint.sh            # 环境变量转启动参数的容器入口脚本
├── CMakeLists.txt                  # CMake 构建入口
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
