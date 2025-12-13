# Canti 使用指南

## 快速开始

### Windows用户

1. **下载预编译版本**（推荐）
   - 从 [Releases页面](https://github.com/lensferno/canti/releases) 下载
   - CLI版本：`canti-windows-amd64.exe`
   - GUI版本：`canti-gui-windows-amd64.exe`

2. **使用方法**
   
   **GUI图形界面（推荐）**：
   ```bash
   # 双击运行或命令行启动
   canti-gui.exe
   ```
   - ✅ 图形界面，操作简单
   - ✅ 双击即可运行
   - ✅ 无需命令行知识

   **Web界面**：
   ```bash
   canti.exe webui
   ```
   然后在浏览器打开 http://localhost:8080
   
   **命令行**：
   ```bash
   canti.exe login -u 学号 -p 密码
   ```

### OpenWrt用户（路由器）

1. **确定路由器架构**
   ```bash
   # SSH登录路由器后执行
   uname -m
   ```
   - 显示 `mips` → 下载 `canti-openwrt-mipsle`
   - 显示 `armv7l` → 下载 `canti-openwrt-arm7`

2. **上传到路由器**
   ```bash
   # 方法1：使用SCP
   scp canti-openwrt-mipsle root@192.168.1.1:/usr/bin/canti
   
   # 方法2：使用WinSCP等图形工具
   ```

3. **设置权限并运行**
   ```bash
   chmod +x /usr/bin/canti
   
   # 启动Web界面（推荐）
   /usr/bin/canti webui --port 8080 &
   ```

4. **浏览器访问**
   - 打开 http://路由器IP:8080
   - 例如：http://192.168.1.1:8080

5. **设置开机自启**
   ```bash
   # 创建启动脚本
   cat > /etc/init.d/canti << 'EOF'
   #!/bin/sh /etc/rc.common
   START=99
   start() {
       /usr/bin/canti webui --port 8080 &
   }
   stop() {
       killall canti
   }
   EOF
   
   # 设置权限并启用
   chmod +x /etc/init.d/canti
   /etc/init.d/canti enable
   /etc/init.d/canti start
   ```

### Linux用户

```bash
# 下载
wget https://github.com/.../canti-linux-amd64.tar.gz
tar -xzf canti-linux-amd64.tar.gz

# 运行
chmod +x canti-linux-amd64

# Web界面
./canti-linux-amd64 webui

# 命令行
./canti-linux-amd64 login -u 学号 -p 密码
```

## 三种界面对比

| 特性 | CLI命令行 | Web界面 | GUI图形界面 |
|------|----------|---------|------------|
| 适用平台 | 全平台 | 全平台 | 仅Windows |
| 易用性 | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| 远程访问 | ❌ | ✅ | ❌ |
| OpenWrt | ✅ | ✅ | ❌ |
| 推荐场景 | 脚本/自动化 | 路由器/服务器 | Windows桌面 |

## 功能详解

### 登录认证

**CLI方式**：
```bash
canti login -u 202100000000 -p 12450password
```

**使用配置文件**：
```bash
# 创建 config.yml
cat > config.yml << EOF
username: "202100000000"
password: "12450password"
method: "web"
reconnect: true
EOF

# 使用配置
canti login --config config.yml
```

**Web/GUI界面**：
- 输入学号和密码
- 点击"登录"按钮

### 查看状态

**CLI**：
```bash
canti status
```

**Web/GUI**：
- 点击"查看状态"按钮

### 登出

**CLI**：
```bash
canti logout
```

**Web/GUI**：
- 点击"登出"按钮

### 开机自启（Windows）

**方法1：任务计划程序**
1. Win+R 输入 `taskschd.msc`
2. 创建基本任务
3. 触发器：系统启动时
4. 操作：启动程序 `canti.exe webui`

**方法2：使用canti内置功能**
```bash
# 以管理员身份运行
canti.exe install
```

### 开机自启（Linux）

**systemd方式**：
```bash
# 创建服务文件
sudo cat > /etc/systemd/system/canti.service << EOF
[Unit]
Description=Canti Campus Network Authentication
After=network.target

[Service]
Type=simple
User=your-username
ExecStart=/path/to/canti webui --port 8080
Restart=always

[Install]
WantedBy=multi-user.target
EOF

# 启用服务
sudo systemctl daemon-reload
sudo systemctl enable canti
sudo systemctl start canti
```

## 常见问题

### Q: Web界面无法访问？

A: 检查以下几点：
1. 防火墙是否允许端口（默认8080）
2. 程序是否正常运行：`ps aux | grep canti`
3. 尝试更换端口：`canti webui --port 8888`

### Q: OpenWrt上提示权限不足？

A: 
```bash
chmod +x canti-openwrt-*
```

### Q: 登录失败怎么办？

A: 
1. 检查用户名密码是否正确
2. 检查网络连接
3. 查看错误信息提示

### Q: GUI版本打不开？

A: Windows可能提示"不受信任的应用"，选择"仍要运行"

### Q: 如何修改认证服务器地址？

A: 需要从源码编译，修改 `app/service/auth_web.go` 第17行的 `webAuthBaseUrl`

### Q: 多台设备可以同时使用吗？

A: Web界面可以多设备访问，但校园网账号同时只能一个设备在线

### Q: 是否支持IPv6？

A: 当前版本主要支持IPv4，IPv6支持正在开发中

### Q: 如何卸载？

A: 
- **Windows**：删除exe文件，如果安装了服务运行 `canti uninstall`
- **OpenWrt**：删除二进制文件和启动脚本
- **Linux**：删除二进制文件和systemd服务文件

## 高级用法

### 使用代理

```bash
export HTTP_PROXY=http://proxy:port
canti login -u 学号 -p 密码
```

### 自定义端口

```bash
canti webui --port 9000
```

### 后台运行

**Linux/OpenWrt**：
```bash
nohup canti webui &
```

**Windows**：
```bash
start /B canti.exe webui
```

### 定时重连

使用配置文件，设置 `reconnect: true`

## 安全建议

1. **不要**在公共电脑上保存密码
2. **定期**更换校园网密码
3. **及时**登出避免被盗用
4. **注意**不要将配置文件分享给他人
5. **建议**从官方渠道下载程序

## 获取帮助

- 查看命令帮助：`canti --help`
- 查看子命令帮助：`canti login --help`
- 项目主页：https://github.com/lensferno/canti
- 提交Issue：https://github.com/lensferno/canti/issues

## 更新日志

查看最新版本和更新内容：https://github.com/lensferno/canti/releases
