# Canti

> 仍在施工中🚧
> 
> 有好的建议和意见欢迎在issue里提出~

全自动的wust武科大校园网认证客户端（当然，是第三方的）

支持使用web方式认证和pppoe拨号（尚未实现）

使用Go语言编写，支持多个平台使用

# 下载与安装

## 方式一：下载预编译版本（推荐）

前往 [Releases页面](https://github.com/lensferno/canti/releases) 下载对应平台的预编译版本。

## 方式二：从源码构建

### 前提条件

- 已安装 Go 1.20 或更高版本（[下载Go](https://golang.org/dl/)）

### 下载项目

**方法1：使用Git克隆（推荐）**
```bash
git clone https://github.com/lensferno/canti.git
cd canti
```

**方法2：下载ZIP压缩包**
1. 访问项目主页：https://github.com/lensferno/canti
2. 点击绿色的 "Code" 按钮
3. 选择 "Download ZIP"
4. 解压下载的 `canti-main.zip` 文件
5. 进入解压后的目录：
   ```bash
   cd canti-main
   ```

### 构建项目

**在Linux/macOS上：**
```bash
# 下载依赖
go mod download

# 构建可执行文件
go build -o canti .
```

**在Windows上：**
```bash
# 下载依赖
go mod download

# 构建可执行文件
go build -o canti.exe .
```

构建完成后，会在当前目录生成可执行文件（Linux/macOS为`canti`，Windows为`canti.exe`）

**高级：使用构建脚本构建多平台多版本**

项目提供了 `build-all.sh` 脚本，可以一次性构建所有平台和版本：

```bash
chmod +x build-all.sh
./build-all.sh
```

构建后的文件将在 `build/` 目录下，支持的版本包括：
- **CLI版本**（命令行 + Web界面）：
  - Windows (amd64/386)
  - Linux (amd64)
  - OpenWrt (MIPS/ARM) - 路由器版本
- **GUI版本**（图形界面）：
  - Windows (amd64) - 需要在Windows环境构建

注意：构建脚本会尝试使用 `upx` 压缩二进制文件（可选）

# 使用

## 命令行模式（CLI）

- 使用web方法认证，用户名（学号）为202100000000，密码为12450password：
  ``` bash
  ./canti login --username 202100000000 --password 12450password -m web
  ```

- 按照指定的配置进行登录认证：
  ``` bash
  ./canti login --config ./config.yml
  ```
  支持的文件格式有：JSON, TOML, YAML, HCL, .env

- 安装为开机自启的服务（用户信息同上文，需要管理员权限）：
  ``` bash
  ./canti install
  ```
	按照提示填写信息即可

- 查看状态：
  ``` bash
  ./canti status
  ```

- 登出：
  ``` bash
  ./canti logout
  ```

## Web界面模式（推荐用于OpenWrt）

启动Web服务器，通过浏览器进行操作：

```bash
./canti webui
# 或指定端口
./canti webui --port 8080
```

然后在浏览器中访问 `http://localhost:8080` 或 `http://路由器IP:8080`

Web界面特点：
- 📱 响应式设计，支持手机/平板访问
- 🎨 现代化UI，操作简单直观
- 🔄 实时状态查询
- ✅ 适合OpenWrt路由器部署

## 图形界面模式（Windows桌面）

如果构建了GUI版本（`canti-gui.exe`），可以直接双击运行，或：

```bash
./canti gui
```

图形界面提供：
- 🖥️ 原生Windows桌面体验
- 📝 用户友好的输入界面
- 📊 状态信息实时显示

## OpenWrt使用指南

1. 将编译好的 `canti-openwrt-*` 文件上传到路由器
2. 添加执行权限：`chmod +x canti-openwrt-*`
3. 启动Web界面：`./canti-openwrt-* webui --port 8080`
4. 在浏览器访问路由器IP:8080进行操作

或者使用命令行：
```bash
./canti-openwrt-* login -u 学号 -p 密码
```

运行 `./canti [命令] --help` 获取详细参数说明