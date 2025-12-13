# SRun认证系统使用指南

本指南说明如何使用Canti连接使用SRun认证系统的校园网。

## 什么是SRun？

SRun（深澜软件）是一种常见的校园网认证系统，许多高校使用该系统进行网络认证。

**特征识别：**
- 登录URL包含 `/cgi-bin/srun_portal` 或 `/cgi-bin/get_challenge`
- 认证需要获取challenge码
- 使用XXTEA加密算法
- 响应格式为JSONP

## 快速开始

### 命令行使用

```bash
# 使用SRun认证登录
./canti login -u 学号 -p 密码 -m srun

# 示例
./canti login -u 2025020632 -p yourpassword -m srun
```

### 配置文件使用

创建 `config.yml`:

```yaml
username: "2025020632"
password: "yourpassword"
method: "srun"
reconnect: true
silence: false
```

然后运行：

```bash
./canti login --config config.yml
```

### Web UI使用

1. 启动Web界面：
```bash
./canti webui --port 8080
```

2. 在浏览器中打开 http://localhost:8080

3. 输入学号和密码，点击登录即可

**注意：** Web UI会自动检测并使用SRun认证（需要确保代码中的`webAuthBaseUrl`已正确配置）

## 配置说明

### 修改认证服务器地址

编辑 `app/service/auth_srun.go` 文件，修改第8行：

```go
const (
    srunBaseUrl = "http://10.145.255.21"  // 改为您学校的认证服务器地址
    ...
)
```

### 修改ac_id参数

不同学校的ac_id可能不同（通常是1、2、3、4等），如果登录失败，尝试修改：

```go
const (
    ...
    srunDefaultAcId = "4"  // 改为您学校的ac_id
    ...
)
```

**如何查找ac_id：**
1. 在浏览器登录时打开开发者工具（F12）
2. 查看Network标签中的登录请求
3. 找到URL中的 `ac_id=X` 参数

## 功能支持

### 已实现功能

- ✅ 登录认证
- ✅ 获取Challenge
- ✅ 密码MD5加密
- ✅ Info参数XXTEA+Base64加密
- ✅ Chksum校验和计算
- ✅ JSONP响应解析
- ✅ 登出功能
- ✅ 状态查询（基础）

### 待完善功能

- ⏳ 详细的在线状态信息
- ⏳ 流量统计
- ⏳ 在线时长统计
- ⏳ 自动重连（需要配合状态查询）

## 技术细节

### 认证流程

1. **获取Challenge**
   ```
   GET /cgi-bin/get_challenge?username=学号&ip=&_=时间戳
   ```
   
   响应：
   ```json
   {
     "challenge": "...",
     "client_ip": "10.242.182.12",
     "ecode": 0
   }
   ```

2. **加密处理**
   - 密码加密：`{MD5}` + MD5(password)
   - Info加密：XXTEA(JSON用户信息, challenge) + 自定义Base64
   - Chksum计算：SHA1(challenge拼接串)

3. **发送登录请求**
   ```
   GET /cgi-bin/srun_portal?action=login&username=...&password=...&chksum=...&info=...&ac_id=...
   ```

4. **解析响应**
   ```json
   {
     "error": "ok",
     "ecode": 0,
     "client_ip": "10.242.182.12"
   }
   ```

### 加密算法说明

**XXTEA加密：**
- 一种轻量级的块加密算法
- 用于加密用户信息JSON
- 使用challenge作为密钥

**自定义Base64：**
- 使用特殊的字母表：`LVoJPiCN2R8G90yg+hmFHuacZ1OWMnrsSTXkYpUq/3dlbfKwv6xztjI7DeBE45QA`
- 与标准Base64编码不同

**SHA1校验：**
- 用于计算请求的完整性校验和
- 防止请求被篡改

## 故障排查

### 问题1：登录失败，提示"请求错误"

**可能原因：**
- 认证服务器地址不正确
- ac_id参数不正确
- 用户名或密码错误

**解决方法：**
1. 检查 `srunBaseUrl` 是否正确
2. 使用浏览器登录，查看Network标签中的请求URL
3. 对比参数是否一致

### 问题2：密码加密不正确

**检查方法：**
1. 在浏览器登录时查看发送的password参数
2. 应该是 `{MD5}` 开头的32位十六进制字符串
3. 可以用在线MD5工具验证

### 问题3：Info参数加密失败

**可能原因：**
- XXTEA加密实现有误
- Base64字母表不匹配

**解决方法：**
1. 查看浏览器中实际发送的info参数
2. 对比前缀是否是 `{SRBX1}`
3. 如果有问题，请提供完整的JS加密代码

### 问题4：Chksum校验失败

**解决方法：**
1. 查看浏览器中JS代码的chksum计算逻辑
2. 确认拼接字符串的顺序
3. 确认是否使用SHA1算法

## 不同学校的差异

### 常见变化

1. **ac_id值不同**
   - 有的学校是1，有的是4
   - 在登录URL中查看

2. **加密算法细节**
   - 有些学校可能使用不同的Base64字母表
   - 有些可能使用不同的加密算法

3. **额外参数**
   - 有些学校可能有额外的参数（如double_stack、os、name等）
   - 这些都已经在代码中实现

### 如何适配您的学校

1. 按照 `CAPTURE_TUTORIAL.md` 抓取完整的请求信息
2. 对比实际请求与代码实现的差异
3. 修改 `auth_srun.go` 中的对应部分
4. 重新编译测试

## 与Web UI集成

Web UI目前使用的是WUST原系统，如需使用SRun：

### 方法1：修改webui代码

编辑 `app/webui/server.go`，在登录函数中根据配置选择认证方式。

### 方法2：全局切换

如果您的学校全部使用SRun，可以：

1. 编辑 `app/conf/conf.go`
2. 修改默认method为srun：
   ```go
   config.Method = conf.LoginSRunMethod
   ```

## 贡献

如果您成功适配了自己学校的SRun系统，欢迎：

1. 提交PR分享您的配置
2. 更新文档说明差异点
3. 帮助其他用户排查问题

## 参考资料

- [SRun协议分析](https://github.com/...)
- [XXTEA加密算法](https://en.wikipedia.org/wiki/XXTEA)
- [深澜软件官网](http://www.srun.com/)

## 获取帮助

遇到问题？

1. 查看 `CAPTURE_TUTORIAL.md` 学习如何抓包
2. 查看 `MIGRATION_GUIDE.md` 了解迁移流程
3. 在GitHub提交Issue，附带完整的抓包信息
4. 加入讨论群获取帮助

---

**更新日期：** 2025-12-13  
**适用版本：** Canti v0.0.1+
