# Canti 完整构建教程

本教程从零开始，详细说明如何拉取依赖、运行、打包整个项目。

## 前提条件

### 必需软件

1. **Go 1.20 或更高版本**
   - 下载：https://golang.org/dl/
   - 验证安装：`go version`

2. **Git**
   - 下载：https://git-scm.com/
   - 验证安装：`git --version`

### 可选软件（用于优化）

3. **UPX**（压缩二进制文件）
   - Ubuntu/Debian: `sudo apt install upx`
   - macOS: `brew install upx`
   - Windows: 从 https://upx.github.io/ 下载

4. **mingw-w64**（Windows GUI交叉编译）
   - Ubuntu/Debian: `sudo apt install mingw-w64`
   - macOS: `brew install mingw-w64`

## 第一步：获取源码

### 方法1：使用Git克隆（推荐）

```bash
# 克隆仓库
git clone https://github.com/lensferno/canti.git

# 进入项目目录
cd canti
```

### 方法2：下载ZIP

1. 访问 https://github.com/lensferno/canti
2. 点击绿色的 "Code" 按钮
3. 选择 "Download ZIP"
4. 解压下载的文件
5. 进入解压后的目录

## 第二步：拉取依赖

```bash
# 进入项目目录（如果还没进入）
cd canti

# 下载所有依赖
go mod download

# 验证依赖
go mod verify
```

**输出示例：**
```
go: downloading github.com/urfave/cli/v2 v2.27.1
go: downloading github.com/go-resty/resty/v2 v2.11.0
...（下载所有依赖）
all modules verified
```

**如果遇到网络问题（在中国大陆）：**
```bash
# 设置Go模块代理
export GOPROXY=https://goproxy.cn,direct

# 或者使用阿里云代理
export GOPROXY=https://mirrors.aliyun.com/goproxy/,direct

# 然后重新下载
go mod download
```

## 第三步：构建基础版本

### 3.1 构建CLI版本（包含Web UI）

**Linux/macOS：**
```bash
# 构建当前平台版本
go build -o canti .

# 测试运行
./canti --help
```

**Windows：**
```bash
# 构建Windows版本
go build -o canti.exe .

# 测试运行
canti.exe --help
```

**输出示例：**
```
 ██████╗ █████╗ ███╗   ██╗████████╗██╗
██╔════╝██╔══██╗████╗  ██║╚══██╔══╝██║
██║     ███████║██╔██╗ ██║   ██║   ██║
...

命令:
   install, i     将canti安装为服务...
   login, l       登录校园网
   webui, web, w  启动Web界面
   gui, g         启动图形界面
   ...
```

### 3.2 测试Web UI

```bash
# 启动Web服务器
./canti webui --port 8080

# 在浏览器中打开
# http://localhost:8080
```

按 `Ctrl+C` 停止服务器。

### 3.3 构建GUI版本（仅Windows）

**在Windows上：**
```bash
# 构建GUI版本
go build -tags gui -ldflags "-H=windowsgui" -o canti-gui.exe .

# 运行GUI
canti-gui.exe
```

**在Linux上交叉编译Windows GUI：**
```bash
# 需要先安装mingw-w64
CGO_ENABLED=1 GOOS=windows GOARCH=amd64 CC=x86_64-w64-mingw32-gcc \
  go build -tags gui -ldflags "-H=windowsgui" -o canti-gui.exe .
```

## 第四步：一键构建所有版本

### 使用build-all.sh脚本

```bash
# 添加执行权限
chmod +x build-all.sh

# 执行构建脚本
./build-all.sh
```

**构建过程：**
```
==========================================
开始构建 Canti 多版本
==========================================

构建 CLI 版本（包含Web UI）...
==========================================
构建 OpenWrt MIPS...
✓ OpenWrt MIPS 完成
构建 OpenWrt ARM...
✓ OpenWrt ARM7 完成
构建 Linux AMD64...
✓ Linux AMD64 完成
构建 Windows AMD64...
✓ Windows AMD64 完成
构建 Windows 386...
✓ Windows 386 完成

==========================================
压缩二进制文件...
==========================================
使用 UPX 压缩...
...

==========================================
构建完成！
==========================================
```

### 构建产物

所有构建文件位于 `build/` 目录：

```bash
ls -lh build/
```

**输出：**
```
canti-linux-amd64              # Linux 64位版本
canti-linux-amd64.tar.gz       # 压缩包

canti-windows-amd64.exe        # Windows 64位版本
canti-windows-amd64.exe.tar.gz # 压缩包

canti-windows-386.exe          # Windows 32位版本
canti-windows-386.exe.tar.gz   # 压缩包

canti-openwrt-mipsle           # OpenWrt MIPS版本
canti-openwrt-mipsle.tar.gz    # 压缩包

canti-openwrt-arm7             # OpenWrt ARM版本
canti-openwrt-arm7.tar.gz      # 压缩包
```

## 第五步：测试构建的程序

### 测试Linux/macOS版本

```bash
# 运行
./build/canti-linux-amd64 --help

# 测试Web UI
./build/canti-linux-amd64 webui --port 8080
```

### 测试Windows版本（在Windows上）

```bash
# 测试命令行
.\build\canti-windows-amd64.exe --help

# 测试Web UI
.\build\canti-windows-amd64.exe webui --port 8080

# 测试GUI版本（如果构建了）
.\build\canti-gui-windows-amd64.exe
```

### 测试OpenWrt版本

```bash
# 可以在Linux上测试（模拟）
./build/canti-openwrt-mipsle --help

# 注意：实际运行需要在OpenWrt路由器上
```

## 第六步：打包发布

### 创建发布压缩包

```bash
cd build/

# 所有文件已经自动打包为 .tar.gz
ls -lh *.tar.gz
```

### 生成校验和

```bash
cd build/

# 生成SHA256校验和
sha256sum canti-* > checksums.txt

# 查看校验和文件
cat checksums.txt
```

### 准备发布说明

创建 `RELEASE_NOTES.md`：

```markdown
# Canti v0.0.1 发布

## 新特性

- ✅ CLI命令行界面
- ✅ Web UI浏览器界面
- ✅ Windows GUI图形界面
- ✅ 支持多平台

## 下载

### Windows用户
- **图形界面版**: canti-gui-windows-amd64.exe
- **命令行+Web版**: canti-windows-amd64.exe

### OpenWrt用户
- **MIPS架构**: canti-openwrt-mipsle
- **ARM架构**: canti-openwrt-arm7

### Linux用户
- **AMD64**: canti-linux-amd64

## 使用方法

详见 [USAGE_GUIDE.md](USAGE_GUIDE.md)
```

## 常见问题排查

### 问题1：依赖下载失败

**错误：**
```
go: github.com/xxx: Get "https://proxy.golang.org/...": dial tcp: i/o timeout
```

**解决：**
```bash
# 使用国内代理
export GOPROXY=https://goproxy.cn,direct
go mod download
```

### 问题2：构建失败 - 缺少包

**错误：**
```
package xxx is not in GOROOT
```

**解决：**
```bash
# 清理模块缓存并重新下载
go clean -modcache
go mod download
go mod tidy
```

### 问题3：UPX压缩失败

**错误：**
```
upx: command not found
```

**解决：**
这不是致命错误，构建脚本会跳过压缩。
要安装UPX：
- Ubuntu/Debian: `sudo apt install upx`
- macOS: `brew install upx`
- Windows: 从 https://upx.github.io/ 下载

### 问题4：GUI构建失败

**错误：**
```
# fyne.io/fyne/v2: CGO is required
```

**解决：**
GUI版本需要CGO和C编译器：
- Windows: 安装 TDM-GCC 或 MinGW
- Linux: `sudo apt install build-essential`
- macOS: 安装 Xcode Command Line Tools

### 问题5：权限被拒绝

**错误：**
```
bash: ./build-all.sh: Permission denied
```

**解决：**
```bash
chmod +x build-all.sh
./build-all.sh
```

## 完整构建流程总结

```bash
# 1. 克隆项目
git clone https://github.com/lensferno/canti.git
cd canti

# 2. 拉取依赖
go mod download

# 3. 快速测试构建
go build -o canti .
./canti --help

# 4. 测试Web UI
./canti webui --port 8080

# 5. 一键构建所有版本
chmod +x build-all.sh
./build-all.sh

# 6. 查看构建产物
ls -lh build/

# 7. 测试构建的程序
./build/canti-linux-amd64 --help
./build/canti-linux-amd64 webui

# 8. 生成校验和
cd build/
sha256sum canti-* > checksums.txt

# 完成！
```

## 开发模式快速测试

如果你只是想快速测试开发版本：

```bash
# 直接运行（不构建）
go run . --help
go run . login -u 学号 -p 密码
go run . webui

# GUI版本（Windows）
go run -tags gui . gui
```

## 持续集成/自动化

如果想设置CI/CD自动构建：

```yaml
# .github/workflows/build.yml 示例
name: Build
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - uses: actions/setup-go@v4
        with:
          go-version: '1.20'
      - run: go mod download
      - run: ./build-all.sh
      - uses: actions/upload-artifact@v3
        with:
          name: binaries
          path: build/
```

## 下一步

- 阅读 [README.md](README.md) 了解使用方法
- 阅读 [PACKAGING.md](PACKAGING.md) 了解详细打包说明
- 阅读 [USAGE_GUIDE.md](USAGE_GUIDE.md) 了解用户指南

## 获取帮助

- 项目主页：https://github.com/lensferno/canti
- 提交Issue：https://github.com/lensferno/canti/issues
