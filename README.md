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

**高级：使用构建脚本构建多平台版本**

项目提供了 `build.sh` 脚本，可以一次性构建Windows和Linux的amd64/386版本。需要先安装 `upx` 工具用于压缩二进制文件。

```bash
chmod +x build.sh
./build.sh
```

构建后的文件将在 `build/` 目录下，支持的平台包括：
- Windows (amd64/386)
- Linux (amd64/386)

# 使用

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

运行./canti [命令] --help获取相关参数