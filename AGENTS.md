# AGENTS.md

## 默认沟通

- 默认使用简体中文沟通。
- 先给明确判断，再说明必要依据和下一步。
- 修改前先列计划并等待用户确认；用户回复“确认 / 执行 / ok / 1”等表示同意执行。

## 仓库来源关系

- 本仓库：`sunnyhmz7010/telegram-bot-api-file-streaming`
- 上游仓库：`lappland22233/tgtc`
- 上游分支：`beta`
- 上游对应目录：`telegram-bot-api/`
- 上游源码目录：`telegram-bot-api/telegram-bot-api/`
- 本仓库源码目录：`server/`

本仓库不是 GitHub 元数据意义上的标准 fork，`parent/source` 为空，更像是从上游 `telegram-bot-api/` 子目录抽取出的独立仓库。因此后续同步不要直接使用 GitHub fork compare 作为唯一依据。

## 上游同步规则

- 同步目标是上游 `beta` 分支里的 `telegram-bot-api/` 子项目。
- 上游源码 `telegram-bot-api/telegram-bot-api/*` 映射到本仓库 `server/*`。
- 移植源码后，把 include 路径从：
  - `#include "telegram-bot-api/..."`
  - 改为 `#include "server/..."`
- `telegram-bot-api/CMakeLists.txt` 需要手工合并到本仓库根目录 `CMakeLists.txt`：
  - 保留本仓库的 `server/` 路径。
  - 只补充新增源码、头文件、测试目标和必要 include/link 配置。
  - 不要把路径改回 `telegram-bot-api/`。
- 保留本仓库包装层，不要被上游覆盖：
  - `Dockerfile`
  - `docker-entrypoint.sh`
  - `.dockerignore`
  - `.github/`
  - `README.md`
  - `SECURITY.md`
  - `LICENSE`
- `td/` 目录在 2026-08-14 检查时与上游 tree 一致。后续同步时如果上游 `td/` 变更，再单独判断是否同步。

## 本次同步记录

检查日期：2026-08-14

已同步到的上游提交：

- `077c7bd9ee80a7c3c33185354c6dfd76918b843c`
- 提交信息：`feat(telegram-bot-api): 添加工作目录清理管理器`
- 上游链接：`https://github.com/lappland22233/tgtc/commit/077c7bd9ee80a7c3c33185354c6dfd76918b843c`

本次同步涉及的上游后续提交包括：

- `859b7c676f4af449a66c96c82507f4e0efb35f8d`：无缓存模式支持文件实时回源直通。
- `8144efbae49d929303809f4f34fa7cdf0baf4c21`：添加环境变量配置，并新增 `FileStreamCore.h`、`TestMain.cpp`、`WorkdirCleanupManager.*`。
- `f511edea42f0447eacfa16e8f01a4c35f2d8b04c`：体检仅获取元数据，避免预载文件内容。
- `077c7bd9ee80a7c3c33185354c6dfd76918b843c`：接入工作目录清理管理器构建配置。

本次新增文件：

- `server/FileStreamCore.h`
- `server/TestMain.cpp`
- `server/WorkdirCleanupManager.cpp`
- `server/WorkdirCleanupManager.h`
- `server/WorkdirCleanupManager.test.cpp`

本次修改范围：

- `CMakeLists.txt`
- `server/*.cpp`
- `server/*.h`
- `server/*.test.cpp`

## 后续检查上游更新的推荐命令

```powershell
git remote add upstream https://github.com/lappland22233/tgtc.git
git fetch upstream beta
git log --oneline --decorate --max-count=20 upstream/beta -- telegram-bot-api
```

如果 `upstream` 已存在，跳过 `git remote add upstream ...`。

比较上游源码目录与本仓库 `server/` 时，先考虑路径映射差异：

```powershell
git diff --name-status HEAD..upstream/beta -- telegram-bot-api/telegram-bot-api telegram-bot-api/CMakeLists.txt
```

实际移植时优先按文件内容迁移，不要直接把上游 `telegram-bot-api/` 目录复制到仓库根目录。

## 验证策略

- 用户已明确表示：本地不需要构建测试，GitHub 会自动运行构建。
- 后续同步后至少做本地静态检查：
  - `git diff --check`
  - 确认 `server/` 下没有残留 `#include "telegram-bot-api/..."`
  - 确认 `CMakeLists.txt` 引用的新增文件都存在
- 不要因为本地没有 `cmake` 或 `docker` 而安装依赖；需要安装或升级依赖前先征求用户确认。

