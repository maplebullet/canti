# Canti 打包说明

本文档详细说明如何构建和打包Canti的各个版本。

## 快速开始

### 一键构建所有版本

```bash
chmod +x build-all.sh
./build-all.sh
```

这会在 `build/` 目录下生成所有平台的可执行文件和压缩包。

## 各版本详细说明

### 1. CLI版本（命令行 + Web UI）

**适用场景**：
- OpenWrt路由器
- Linux服务器
- Windows命令行使用

**构建方法**：

```bash
# Linux AMD64
CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -o canti-linux-amd64 .

# Windows AMD64
CGO_ENABLED=0 GOOS=windows GOARCH=amd64 go build -o canti-windows-amd64.exe .

# OpenWrt MIPS (路由器 - 小端)
CGO_ENABLED=0 GOOS=linux GOARCH=mipsle GOMIPS=softfloat go build -o canti-openwrt-mipsle .

# OpenWrt ARM (路由器 - ARMv7)
CGO_ENABLED=0 GOOS=linux GOARCH=arm GOARM=7 go build -o canti-openwrt-arm7 .
```

**功能**：
- ✅ 命令行登录/登出
- ✅ 配置文件支持
- ✅ 系统服务安装
- ✅ Web UI界面（内置HTTP服务器）

### 2. GUI版本（图形界面）

**适用场景**：
- Windows桌面用户
- 需要图形界面的场景

**构建方法**：

在 Windows 上：
```bash
go build -tags gui -ldflags "-H=windowsgui" -o canti-gui.exe .
```

在 Linux 上交叉编译（需要mingw-w64）：
```bash
CGO_ENABLED=1 GOOS=windows GOARCH=amd64 CC=x86_64-w64-mingw32-gcc \
  go build -tags gui -ldflags "-H=windowsgui" -o canti-gui.exe .
```

**注意事项**：
- GUI版本需要CGO（C编译器）
- Windows交叉编译需要安装mingw-w64
- 生成的exe文件较大（约30-40MB），但无需安装依赖

**功能**：
- ✅ 原生Windows界面
- ✅ 图形化登录/登出
- ✅ 状态显示
- ❌ 不支持系统服务（使用任务计划程序代替）

## 优化技巧

### 使用UPX压缩

UPX可以将二进制文件压缩50-70%：

```bash
# 安装UPX
# Ubuntu/Debian: sudo apt install upx
# macOS: brew install upx
# Windows: 从 https://upx.github.io/ 下载

# 压缩文件
upx -8 canti-linux-amd64
```

### 减小文件大小

```bash
# 使用-ldflags去除调试信息
go build -ldflags "-s -w" -o canti .

# 进一步优化（仅限特定平台）
go build -ldflags "-s -w -extldflags -static" -o canti .
```

## 版本对比

| 版本 | 大小 | CLI | Web UI | GUI | OpenWrt | Windows | Linux |
|------|------|-----|--------|-----|---------|---------|-------|
| CLI  | ~10MB | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ |
| GUI  | ~30MB | ✅ | ❌ | ✅ | ❌ | ✅ | ❌ |

## 发布流程

### 1. 构建所有版本

```bash
./build-all.sh
```

### 2. 测试关键功能

```bash
# 测试CLI
./build/canti-linux-amd64 --help

# 测试Web UI
./build/canti-linux-amd64 webui --port 8080
# 访问 http://localhost:8080

# 测试GUI（Windows）
./build/canti-gui-windows-amd64.exe
```

### 3. 创建发布包

```bash
cd build/

# 已经生成了tar.gz文件
ls -lh *.tar.gz

# 或手动打包
zip canti-windows-amd64.zip canti-windows-amd64.exe
zip canti-gui-windows-amd64.zip canti-gui-windows-amd64.exe
```

### 4. 生成校验和

```bash
cd build/
sha256sum canti-* > checksums.txt
```

### 5. 上传到GitHub Release

1. 在GitHub上创建新的Release
2. 上传所有的 `.tar.gz` 和 `.zip` 文件
3. 上传 `checksums.txt`
4. 编写Release说明

## OpenWrt专用说明

### 选择正确的架构

OpenWrt路由器有不同的CPU架构，需要选择对应的版本：

- **MIPS（小端）**：`canti-openwrt-mipsle`
  - 适用于大多数常见路由器（MT7621、AR71xx等）
- **ARM**：`canti-openwrt-arm7`
  - 适用于ARM架构路由器（如Raspberry Pi、某些高端路由器）

### 查看路由器架构

```bash
# 在OpenWrt路由器上执行
uname -m

# 输出示例：
# mips - 使用mipsle版本
# armv7l - 使用arm7版本
```

### 在OpenWrt上安装

```bash
# 1. 上传文件到路由器
scp canti-openwrt-mipsle root@192.168.1.1:/usr/bin/canti

# 2. 设置权限
ssh root@192.168.1.1
chmod +x /usr/bin/canti

# 3. 测试
/usr/bin/canti --help

# 4. 启动Web UI
/usr/bin/canti webui --port 8080 &
```

### 设置开机自启（OpenWrt）

创建 `/etc/init.d/canti`：

```bash
#!/bin/sh /etc/rc.common

START=99
STOP=10

start() {
    /usr/bin/canti webui --port 8080 &
}

stop() {
    killall canti
}
```

启用服务：
```bash
chmod +x /etc/init.d/canti
/etc/init.d/canti enable
/etc/init.d/canti start
```

## Windows GUI打包

### 创建安装程序（可选）

使用 [Inno Setup](https://jrsoftware.org/isinfo.php) 或 [NSIS](https://nsis.sourceforge.io/)：

1. 创建安装脚本
2. 包含 `canti-gui.exe`
3. 创建桌面快捷方式
4. 添加卸载程序

### 便携版

GUI版本本身就是独立可执行文件，可以直接分发：

```bash
# 创建便携版压缩包
zip canti-gui-portable.zip canti-gui.exe README.txt
```

## 常见问题

### Q: 为什么GUI版本这么大？

A: GUI版本包含了Fyne框架和所有依赖，但好处是不需要用户安装任何依赖。

### Q: OpenWrt版本能用GUI吗？

A: 不能。OpenWrt是嵌入式系统，没有图形显示。但可以使用Web UI在浏览器中操作。

### Q: 如何减小文件大小？

A: 使用UPX压缩、Go编译优化选项，或使用CLI版本（比GUI版本小3倍）。

### Q: 交叉编译失败怎么办？

A: 
- 确保安装了目标平台的工具链
- GUI版本需要CGO，确保设置了正确的CC编译器
- 尝试在目标平台上直接编译

## 开发建议

### 本地开发

```bash
# CLI开发
go run . login -u test -p test

# Web UI开发
go run . webui

# GUI开发（Windows）
go run -tags gui . gui
```

### 快速测试构建

```bash
# 只构建当前平台
go build -o canti .

# 测试特定平台
GOOS=linux GOARCH=amd64 go build -o canti-linux .
```

## 更多信息

- 主项目地址：https://github.com/lensferno/canti
- Fyne文档：https://docs.fyne.io/
- Go交叉编译：https://go.dev/doc/install/source#environment
