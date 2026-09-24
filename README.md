# Operating System Labs

本仓库用于 uCore on RISC-V64 操作系统实验。当前只准备实验环境和实验一代码，尚未填写实验答案。

## 目录

```text
.
|-- lab1/
|   |-- codes/lab1/       # 实验一代码
|   |-- references/       # 环境和工具链参考文档
|   `-- lab1.md           # 实验报告模板
|-- labcodes -> lab1/codes
|-- tools/riscv/bin/      # 本机工具链命令入口（不提交二进制）
|-- imports/              # 最初收到的代码备份
`-- env.sh                # 当前终端加载实验环境
```

`labcodes` 是兼容课程要求的快捷入口，因此以下两个路径指向同一份代码：

- `labcodes/lab1`
- `lab1/codes/lab1`

## 使用

```bash
cd /Users/akobayashi/OSLab
source ./env.sh
cd labcodes/lab1

riscv64-unknown-elf-gcc --version
qemu-system-riscv64 --version
make
make qemu
```

退出 QEMU：先按 `Ctrl+A`，松开后再按 `X`。

实验代码来自 [`nkgongxl/ucoreonrv`](https://github.com/nkgongxl/ucoreonrv) 的 `code_practice` 分支，固定来源提交为 `67be3f2da866e6e3da43b02c9805499178da6376`。

本机使用 Homebrew 的 Apple Silicon 原生工具：GCC 16.2.0、Binutils 2.47、GDB 17.2、QEMU 11.1.1。课程代码针对旧版 SiFive 工具链，因此只做了两处不涉及实验答案的兼容调整：将旧 CSR 名 `sbadaddr` 改为标准名 `stval`，并让新版 QEMU 通过 `-kernel bin/kernel` 启动内核。

## GitHub

首次创建远端仓库前，先登录 GitHub CLI：

```bash
gh auth login -h github.com -p https -w
```

登录后，在仓库根目录创建远端并首次推送：

```bash
gh repo create Operating_System --private --source=. --remote=origin --push
```

小组仓库可在 GitHub 网页的 **Settings -> Collaborators** 中添加组员。不要提交 API Key、访问令牌、工具链二进制、`bin/` 或 `obj/` 构建产物。
