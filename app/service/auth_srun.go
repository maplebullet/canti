package service

import (
	"canti/library/ecode"
	"crypto/md5"
	"crypto/sha1"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"net/url"
	"strings"
	"time"
)

// SRun认证系统的URL常量
const (
	srunBaseUrl           = "http://10.145.255.21"
	srunChallengeUrl      = srunBaseUrl + "/cgi-bin/get_challenge"
	srunPortalUrl         = srunBaseUrl + "/cgi-bin/srun_portal"
	srunDefaultAcId       = "4"
	srunDefaultN          = "200"
	srunDefaultType       = "1"
	srunCustomBase64Alpha = "LVoJPiCN2R8G90yg+hmFHuacZ1OWMnrsSTXkYpUq/3dlbfKwv6xztjI7DeBE45QA"
)

// SRunChallengeResp Challenge响应结构
type SRunChallengeResp struct {
	Challenge string `json:"challenge"`
	ClientIP  string `json:"client_ip"`
	Ecode     int    `json:"ecode"`
	Error     string `json:"error"`
	ErrorMsg  string `json:"error_msg"`
	Expire    string `json:"expire"`
	OnlineIP  string `json:"online_ip"`
	Res       string `json:"res"`
	SrunVer   string `json:"srun_ver"`
	St        int64  `json:"st"`
}

// SRunLoginResp 登录响应结构
type SRunLoginResp struct {
	Error     string `json:"error"`
	ErrorMsg  string `json:"error_msg"`
	Ecode     int    `json:"ecode"`
	ClientIP  string `json:"client_ip"`
	OnlineIP  string `json:"online_ip"`
	SrunVer   string `json:"srun_ver"`
	// 其他字段根据实际响应添加
}

// SRunUserInfo 用于加密的用户信息结构
type SRunUserInfo struct {
	Username string `json:"username"`
	Password string `json:"password"`
	IP       string `json:"ip"`
	Acid     string `json:"acid"`
	EncVer   string `json:"enc_ver"`
}

// SRunLogin SRun系统登录
func (s *Service) SRunLogin() (*OnlineStatus, error) {
	// 1. 获取客户端IP（从challenge响应中获取）
	challenge, clientIP, err := s.srunGetChallenge()
	if err != nil {
		return nil, err
	}

	// 2. 加密密码
	encryptedPassword := s.srunEncryptPassword(s.conf.Password)

	// 3. 准备用户信息用于info加密
	userInfo := SRunUserInfo{
		Username: s.conf.Username,
		Password: s.conf.Password,
		IP:       clientIP,
		Acid:     srunDefaultAcId,
		EncVer:   "srun_bx1",
	}

	// 4. 加密info参数
	encryptedInfo := s.srunEncryptInfo(userInfo, challenge)

	// 5. 计算chksum
	chksum := s.srunCalculateChksum(challenge, s.conf.Username, encryptedPassword, 
		clientIP, srunDefaultAcId, encryptedInfo)

	// 6. 构建登录URL
	loginUrl := s.srunBuildLoginUrl(s.conf.Username, encryptedPassword, clientIP, 
		chksum, encryptedInfo, srunDefaultAcId)

	// 7. 发送登录请求
	resp, err := s.requester.R().Get(loginUrl)
	if err != nil {
		return nil, ecode.RequestErr
	}

	// 8. 解析JSONP响应
	body := resp.String()
	loginResp, err := s.srunParseJSONP(body)
	if err != nil {
		return nil, err
	}

	// 9. 检查登录结果
	if loginResp.Ecode != 0 {
		return nil, ecode.NewErrCode(loginResp.Ecode, loginResp.ErrorMsg)
	}

	// 10. 构建在线状态
	onlineStatus := &OnlineStatus{
		Time:     time.Now(),
		Ip:       clientIP,
		Username: s.conf.Username,
	}

	return onlineStatus, nil
}

// srunGetChallenge 获取challenge
func (s *Service) srunGetChallenge() (string, string, error) {
	// 构建URL
	params := url.Values{}
	params.Add("callback", "jQuery_callback")
	params.Add("username", s.conf.Username)
	params.Add("ip", "")
	params.Add("_", fmt.Sprintf("%d", time.Now().UnixMilli()))

	reqUrl := fmt.Sprintf("%s?%s", srunChallengeUrl, params.Encode())

	// 发送请求
	resp, err := s.requester.R().Get(reqUrl)
	if err != nil {
		return "", "", ecode.RequestErr
	}

	// 解析JSONP响应
	body := resp.String()
	
	// 提取JSON部分: jQuery_callback({...})
	start := strings.Index(body, "(")
	end := strings.LastIndex(body, ")")
	if start == -1 || end == -1 {
		return "", "", fmt.Errorf("invalid JSONP response")
	}
	
	jsonStr := body[start+1 : end]

	var challengeResp SRunChallengeResp
	if err := json.Unmarshal([]byte(jsonStr), &challengeResp); err != nil {
		return "", "", err
	}

	if challengeResp.Ecode != 0 {
		return "", "", fmt.Errorf("challenge error: %s", challengeResp.ErrorMsg)
	}

	return challengeResp.Challenge, challengeResp.ClientIP, nil
}

// srunEncryptPassword 加密密码: {MD5} + md5(password)
func (s *Service) srunEncryptPassword(password string) string {
	hash := md5.Sum([]byte(password))
	return "{MD5}" + hex.EncodeToString(hash[:])
}

// srunEncryptInfo 加密info参数
func (s *Service) srunEncryptInfo(userInfo SRunUserInfo, token string) string {
	// 1. 将用户信息转为JSON
	jsonBytes, _ := json.Marshal(userInfo)
	jsonStr := string(jsonBytes)

	// 2. 使用XXTEA加密
	encrypted := xxteaEncrypt(jsonStr, token)

	// 3. 使用自定义Base64编码
	encoded := customBase64Encode(encrypted, srunCustomBase64Alpha)

	// 4. 添加前缀
	return "{SRBX1}" + encoded
}

// srunCalculateChksum 计算chksum: SHA1(challenge + username + challenge + password + ...)
func (s *Service) srunCalculateChksum(challenge, username, password, ip, acid, info string) string {
	// 根据实际的JS代码，chksum的计算方式可能需要调整
	// 这里提供一个通用的实现
	chkstr := challenge + username + challenge + password + challenge + acid + challenge + ip + challenge + srunDefaultN + challenge + srunDefaultType + challenge + info
	
	hash := sha1.Sum([]byte(chkstr))
	return hex.EncodeToString(hash[:])
}

// srunBuildLoginUrl 构建登录URL
func (s *Service) srunBuildLoginUrl(username, password, ip, chksum, info, acid string) string {
	params := url.Values{}
	params.Add("callback", "jQuery_callback")
	params.Add("action", "login")
	params.Add("username", username)
	params.Add("password", password)
	params.Add("os", "Windows 10")
	params.Add("name", "Windows")
	params.Add("double_stack", "0")
	params.Add("chksum", chksum)
	params.Add("info", info)
	params.Add("ac_id", acid)
	params.Add("ip", ip)
	params.Add("n", srunDefaultN)
	params.Add("type", srunDefaultType)
	params.Add("_", fmt.Sprintf("%d", time.Now().UnixMilli()))

	return fmt.Sprintf("%s?%s", srunPortalUrl, params.Encode())
}

// srunParseJSONP 解析JSONP响应
func (s *Service) srunParseJSONP(body string) (*SRunLoginResp, error) {
	// 提取JSON部分
	start := strings.Index(body, "(")
	end := strings.LastIndex(body, ")")
	if start == -1 || end == -1 {
		return nil, fmt.Errorf("invalid JSONP response")
	}
	
	jsonStr := body[start+1 : end]

	var loginResp SRunLoginResp
	if err := json.Unmarshal([]byte(jsonStr), &loginResp); err != nil {
		return nil, err
	}

	return &loginResp, nil
}

// SRunLogout SRun系统登出
func (s *Service) SRunLogout() error {
	// 获取客户端IP
	_, clientIP, err := s.srunGetChallenge()
	if err != nil {
		return err
	}

	params := url.Values{}
	params.Add("callback", "jQuery_callback")
	params.Add("action", "logout")
	params.Add("username", s.conf.Username)
	params.Add("ip", clientIP)
	params.Add("ac_id", srunDefaultAcId)
	params.Add("_", fmt.Sprintf("%d", time.Now().UnixMilli()))

	logoutUrl := fmt.Sprintf("%s?%s", srunPortalUrl, params.Encode())

	resp, err := s.requester.R().Get(logoutUrl)
	if err != nil {
		return ecode.RequestErr
	}

	// 解析响应
	body := resp.String()
	loginResp, err := s.srunParseJSONP(body)
	if err != nil {
		return err
	}

	if loginResp.Ecode != 0 {
		return ecode.NewErrCode(loginResp.Ecode, loginResp.ErrorMsg)
	}

	return nil
}

// SRunGetOnlineStatus SRun系统获取在线状态
func (s *Service) SRunGetOnlineStatus() (*OnlineStatus, error) {
	// 获取客户端IP和在线状态
	_, clientIP, err := s.srunGetChallenge()
	if err != nil {
		return nil, err
	}

	// 构建状态查询URL (使用与登录相同的challenge接口来检测在线状态)
	// 如果用户在线，challenge响应中的online_ip字段会有值
	params := url.Values{}
	params.Add("callback", "jQuery_callback")
	params.Add("username", s.conf.Username)
	params.Add("ip", clientIP)
	params.Add("_", fmt.Sprintf("%d", time.Now().UnixMilli()))

	reqUrl := fmt.Sprintf("%s?%s", srunChallengeUrl, params.Encode())

	resp, err := s.requester.R().Get(reqUrl)
	if err != nil {
		return nil, ecode.RequestErr
	}

	body := resp.String()
	start := strings.Index(body, "(")
	end := strings.LastIndex(body, ")")
	if start == -1 || end == -1 {
		return nil, fmt.Errorf("invalid JSONP response")
	}
	
	jsonStr := body[start+1 : end]

	var challengeResp SRunChallengeResp
	if err := json.Unmarshal([]byte(jsonStr), &challengeResp); err != nil {
		return nil, err
	}

	if challengeResp.Ecode != 0 {
		return nil, ecode.NewErrCode(challengeResp.Ecode, challengeResp.ErrorMsg)
	}

	return &OnlineStatus{
		Time:     time.Now(),
		Ip:       challengeResp.OnlineIP,
		Username: s.conf.Username,
	}, nil
}

// ==== XXTEA加密实现 ====

func xxteaEncrypt(str, key string) string {
	if str == "" {
		return ""
	}

	v := strToLongs(str, true)
	k := strToLongs(key, false)
	
	if len(k) < 4 {
		// 扩展密钥到至少4个元素
		for len(k) < 4 {
			k = append(k, 0)
		}
	}

	n := len(v) - 1
	z := v[n]
	y := v[0]
	delta := uint32(0x9E3779B9)
	q := 6 + 52/(n+1)
	sum := uint32(0)

	for q > 0 {
		sum += delta
		e := (sum >> 2) & 3

		for p := 0; p < n; p++ {
			y = v[p+1]
			mx := (z>>5 ^ y<<2) + (y>>3 ^ z<<4) ^ (sum ^ y) + (k[(p&3)^int(e)] ^ z)
			z = v[p] + mx
			v[p] = z
		}

		y = v[0]
		mx := (z>>5 ^ y<<2) + (y>>3 ^ z<<4) ^ (sum ^ y) + (k[(n&3)^int(e)] ^ z)
		z = v[n] + mx
		v[n] = z

		q--
	}

	return longsToStr(v, false)
}

func strToLongs(s string, includeLength bool) []uint32 {
	length := len(s)
	result := make([]uint32, (length+3)/4)

	for i := 0; i < length; i++ {
		result[i>>2] |= uint32(s[i]) << ((i & 3) << 3)
	}

	if includeLength {
		result = append(result, uint32(length))
	}

	return result
}

func longsToStr(l []uint32, includeLength bool) string {
	length := len(l)
	lastLength := (length - 1) << 2

	if includeLength {
		m := l[length-1]
		if m < uint32(lastLength-3) || m > uint32(lastLength) {
			return ""
		}
		lastLength = int(m)
	}

	result := make([]byte, length<<2)
	for i := 0; i < length; i++ {
		result[i<<2] = byte(l[i] & 0xff)
		result[(i<<2)+1] = byte((l[i] >> 8) & 0xff)
		result[(i<<2)+2] = byte((l[i] >> 16) & 0xff)
		result[(i<<2)+3] = byte((l[i] >> 24) & 0xff)
	}

	if includeLength {
		return string(result[:lastLength])
	}
	return string(result)
}

// ==== 自定义Base64编码 ====

func customBase64Encode(input, alphabet string) string {
	if len(alphabet) != 64 {
		alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
	}

	output := ""
	chr1, chr2, chr3, enc1, enc2, enc3, enc4 := 0, 0, 0, 0, 0, 0, 0
	i := 0
	inputLen := len(input)

	for i < inputLen {
		chr1 = int(input[i])
		i++

		if i < inputLen {
			chr2 = int(input[i])
			i++
		} else {
			chr2 = 0
		}

		if i < inputLen {
			chr3 = int(input[i])
			i++
		} else {
			chr3 = 0
		}

		enc1 = chr1 >> 2
		enc2 = ((chr1 & 3) << 4) | (chr2 >> 4)
		enc3 = ((chr2 & 15) << 2) | (chr3 >> 6)
		enc4 = chr3 & 63

		if chr2 == 0 && i-1 >= inputLen {
			enc3 = 64
			enc4 = 64
		} else if chr3 == 0 && i-1 >= inputLen {
			enc4 = 64
		}

		output += string(alphabet[enc1])
		output += string(alphabet[enc2])
		if enc3 != 64 {
			output += string(alphabet[enc3])
		}
		if enc4 != 64 {
			output += string(alphabet[enc4])
		}
	}

	return output
}
