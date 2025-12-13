# 校园网认证系统迁移指南

本指南帮助您将Canti从原WUST校园网系统迁移到其他学校的认证系统。

## 问题诊断

如果您遇到 "err -1: 请求错误" 或类似错误，说明您的学校使用的认证系统与原WUST系统不同。

### 常见认证系统类型

1. **WUST原系统**（本项目默认）
   - API路径：`/api/account/login`
   - 特点：使用nasId参数

2. **SRun系统**（深澜软件）
   - API路径：`/cgi-bin/srun_portal`
   - 特点：需要challenge、加密等步骤
   - 示例：`http://10.145.255.21/cgi-bin/get_challenge`

3. **Dr.COM系统**
   - API路径：通常是 `/drcom/`
   - 特点：需要特定的加密算法

4. **锐捷系统**
   - API路径：通常是Portal认证
   - 特点：802.1X或Web Portal

## 步骤1：识别您的认证系统

### 方法1：查看登录页面源码

在浏览器登录校园网时：
1. 打开开发者工具（F12）
2. 切换到"网络"（Network）标签
3. 输入用户名密码，点击登录
4. 查看发送的请求

**关键信息：**
- 请求URL（例如：`http://10.145.255.21/cgi-bin/srun_portal`）
- 请求方法（GET/POST）
- 请求参数（username, password, challenge等）
- 响应格式（JSON, JSONP, XML等）

### 方法2：查看浏览器控制台

记录以下信息：
```
请求URL: ___________
请求方法: GET / POST
请求参数:
  - username: 
  - password:
  - 其他参数: 
响应示例: ___________
```

## 步骤2：根据您的情况选择方案

### 方案A：简单修改（仅URL不同）

如果您的系统API结构与WUST类似，只是域名/IP不同：

**修改文件：** `app/service/auth_web.go`

```go
// 第17行，修改为您的认证服务器地址
const (
    webAuthBaseUrl = "http://您的服务器IP或域名"
    apiPath        = "/api"  // 根据实际情况修改
    // ...
)
```

### 方案B：中等修改（API路径不同）

如果API路径不同，需要修改更多：

```go
const (
    webAuthBaseUrl = "http://10.145.255.21"
    
    // 根据实际情况修改所有API路径
    _loginApiUrl     = webAuthBaseUrl + "/cgi-bin/srun_portal"
    _logoutApiUrl    = webAuthBaseUrl + "/cgi-bin/srun_portal"
    _statusApiUrl    = webAuthBaseUrl + "/cgi-bin/get_info"
)
```

### 方案C：完全重写（不同认证系统）

**针对SRun系统（您的情况）**

您需要实现SRun的认证流程，这是一个多步骤的过程：

## 步骤3：实现SRun认证（示例）

基于您提供的API信息，SRun系统需要以下步骤：

### 3.1 获取Challenge

```go
// 第一步：获取challenge
func (s *Service) getSRunChallenge(username, ip string) (string, error) {
    url := fmt.Sprintf("%s/cgi-bin/get_challenge?username=%s&ip=%s", 
        webAuthBaseUrl, username, ip)
    
    resp, err := s.requester.R().Get(url)
    if err != nil {
        return "", err
    }
    
    // 解析JSONP响应
    // jQuery11240652393442636386_1765621983080({"challenge": "..."})
    body := resp.String()
    // 提取JSON部分
    start := strings.Index(body, "(") + 1
    end := strings.LastIndex(body, ")")
    jsonStr := body[start:end]
    
    var result struct {
        Challenge string `json:"challenge"`
        Ecode     int    `json:"ecode"`
    }
    json.Unmarshal([]byte(jsonStr), &result)
    
    if result.Ecode != 0 {
        return "", fmt.Errorf("获取challenge失败")
    }
    
    return result.Challenge, nil
}
```

### 3.2 加密密码

SRun系统通常使用MD5或其他方式加密：

```go
import "crypto/md5"

func (s *Service) encryptPassword(password, challenge string) string {
    // 具体加密方式需要根据您学校的实现
    // 常见方式：MD5(challenge + password)
    data := challenge + password
    hash := md5.Sum([]byte(data))
    return fmt.Sprintf("%x", hash)
}
```

### 3.3 发送登录请求

```go
func (s *Service) SRunLogin() (*OnlineStatus, error) {
    // 1. 获取客户端IP（可能需要从其他接口获取）
    clientIP := "自动获取或配置"
    
    // 2. 获取challenge
    challenge, err := s.getSRunChallenge(s.conf.Username, clientIP)
    if err != nil {
        return nil, err
    }
    
    // 3. 加密密码
    encryptedPwd := s.encryptPassword(s.conf.Password, challenge)
    
    // 4. 发送登录请求
    loginUrl := webAuthBaseUrl + "/cgi-bin/srun_portal"
    resp, err := s.requester.R().
        SetQueryParams(map[string]string{
            "action":   "login",
            "username": s.conf.Username,
            "password": encryptedPwd,
            "ac_id":    "1", // 根据实际情况
            "ip":       clientIP,
            "chksum":   "计算校验和",
            "info":     "编码的info参数",
        }).
        Get(loginUrl)
    
    // 5. 解析响应
    // ...
}
```

## 步骤4：创建新的认证模块（推荐）

为了不破坏原有代码，建议创建新文件：

**创建 `app/service/auth_srun.go`**

```go
package service

import (
    "crypto/md5"
    "encoding/json"
    "fmt"
    "strings"
)

const (
    srunBaseUrl = "http://10.145.255.21"
)

// SRun认证相关结构
type SRunChallengeResp struct {
    Challenge string `json:"challenge"`
    ClientIP  string `json:"client_ip"`
    Ecode     int    `json:"ecode"`
    Error     string `json:"error"`
}

type SRunLoginResp struct {
    Error   string `json:"error"`
    ErrorMsg string `json:"error_msg"`
    ClientIP string `json:"client_ip"`
    // 根据实际响应添加字段
}

func (s *Service) SRunLogin() (*OnlineStatus, error) {
    // 实现SRun登录逻辑
    // 1. 获取challenge
    // 2. 加密密码
    // 3. 构建请求
    // 4. 发送登录
    // 5. 解析结果
    return nil, nil
}

func (s *Service) SRunLogout() error {
    // 实现SRun登出逻辑
    return nil
}

func (s *Service) SRunGetStatus() (*OnlineStatus, error) {
    // 实现SRun状态查询
    return nil, nil
}
```

## 步骤5：修改配置支持多种认证

**修改 `app/conf/conf.go`** 添加认证类型配置：

```go
const (
    LoginWebMethod  = "web"      // WUST原系统
    LoginSRunMethod = "srun"     // SRun系统
    LoginDrcomMethod = "drcom"   // Dr.COM系统
)

type Config struct {
    Username   string
    Password   string
    Method     string  // "web", "srun", "drcom"
    // ...其他配置
}
```

**修改 `app/cmd/action.go`** 根据配置选择认证方式：

```go
func login(c *cli.Context) error {
    // ... 读取配置 ...
    
    switch config.Method {
    case conf.LoginWebMethod:
        return loginWeb(config)
    case conf.LoginSRunMethod:
        return loginSRun(config)
    default:
        return fmt.Errorf("不支持的认证方式: %s", config.Method)
    }
}

func loginSRun(config *conf.Config) error {
    srv.SetConfig(*config)
    status, err := srv.SRunLogin()
    if err != nil {
        return err
    }
    // 显示登录信息
    return nil
}
```

## 步骤6：获取更多信息

### 需要从浏览器抓取的信息

使用浏览器开发者工具，记录完整的登录过程：

1. **第一步：获取challenge**
   ```
   URL: http://10.145.255.21/cgi-bin/get_challenge
   参数:
     - callback: jQuery...
     - username: 2025020632
     - ip: 10.242.182.12
   响应: {"challenge": "...", "ecode": 0, ...}
   ```

2. **第二步：登录请求**
   ```
   URL: http://10.145.255.21/cgi-bin/srun_portal
   参数:
     - action: login
     - username: ?
     - password: ?（加密后）
     - ac_id: ?
     - ip: ?
     - chksum: ?
     - info: ?
     - n: ?
     - type: ?
   响应: ?
   ```

3. **第三步：状态查询**
   ```
   URL: ?
   方法: ?
   响应: ?
   ```

4. **第四步：登出**
   ```
   URL: ?
   参数: ?
   ```

### 使用抓包工具

推荐使用：
- **Fiddler**（Windows）
- **Charles**（Mac/Windows）
- **Wireshark**（所有平台）
- **浏览器开发者工具**（最简单）

## 步骤7：测试新实现

```bash
# 编译
go build -o canti .

# 测试
./canti login -u 学号 -p 密码 -m srun

# 或使用配置文件
# config.yml
# username: "2025020632"
# password: "your_password"
# method: "srun"

./canti login --config config.yml
```

## 实用工具脚本

创建 `tools/capture_requests.py` 用于分析抓包：

```python
#!/usr/bin/env python3
import json
import sys

def parse_jsonp(text):
    """解析JSONP响应"""
    start = text.find('(') + 1
    end = text.rfind(')')
    if start > 0 and end > start:
        return json.loads(text[start:end])
    return json.loads(text)

def analyze_request(url, method, params, response):
    """分析请求"""
    print(f"URL: {url}")
    print(f"方法: {method}")
    print(f"参数: {json.dumps(params, indent=2)}")
    print(f"响应: {json.dumps(response, indent=2)}")
    print("-" * 50)

# 使用示例：
# 将浏览器Network标签的请求复制为cURL
# 然后转换为Python代码进行分析
```

## 常见问题

### Q1: 如何确定加密方式？

A: 查看登录页面的JavaScript代码，搜索关键词：
- `md5`, `sha1`, `encrypt`
- `encode`, `hash`
- 查看是否引用了加密库

### Q2: JSONP响应如何处理？

A: JSONP格式为 `callback({"data": "..."})`，需要：
```go
// 去掉callback包装
body := resp.String()
start := strings.Index(body, "(") + 1
end := strings.LastIndex(body, ")")
jsonStr := body[start:end]
json.Unmarshal([]byte(jsonStr), &result)
```

### Q3: 如何获取客户端IP？

A: 方法有：
1. 从challenge响应中的`client_ip`字段
2. 调用专门的获取IP接口
3. 从登录页面URL参数中解析

### Q4: chksum和info参数如何计算？

A: 需要查看JavaScript代码中的计算逻辑，通常是：
```javascript
// 示例（需根据实际情况）
chksum = sha1(challenge + username + challenge + encrypted_password + ...)
info = base64(json_encode({...}))
```

## 获取帮助

如果您需要帮助迁移到您的学校：

1. **提供完整的抓包信息**（隐藏敏感数据）
2. **登录页面URL**
3. **所有网络请求的详细信息**
4. **JavaScript代码片段**（如果可以获取）

提交Issue时附带这些信息：
- 学校名称（可选）
- 认证系统类型（SRun/Dr.COM/锐捷等）
- 完整的HTTP请求/响应示例
- 错误信息截图

## 贡献

如果您成功迁移到新学校，欢迎：
1. 提交PR添加新的认证方式
2. 分享您的实现经验
3. 帮助其他用户

## 参考资源

- [SRun认证协议分析](https://github.com/...)
- [Dr.COM认证原理](https://github.com/...)
- [校园网认证系统逆向工程](https://blog....)
