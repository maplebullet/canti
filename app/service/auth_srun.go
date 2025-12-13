package service

import (
	"canti/library/ecode"
	"crypto/hmac"
	"crypto/md5"
	"crypto/sha1"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"net/url"
	"regexp"
	"strconv"
	"strings"
	"time"
)

// Srun portal configuration constants
// Default values can be overridden via config
const (
	defaultSrunBaseUrl = "http://10.145.255.21"
	defaultSrunAcId    = 4
	srunPortalPath     = "/cgi-bin/srun_portal"
	srunGetChallenge   = "/cgi-bin/get_challenge"
)

// SrunConfig holds Srun portal specific configuration
type SrunConfig struct {
	BaseUrl string `json:"srunBaseUrl" yaml:"srunBaseUrl"`
	AcId    int    `json:"acId" yaml:"acId"`
}

// SrunChallengeResp represents the response from get_challenge API
type SrunChallengeResp struct {
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

// SrunLoginResp represents the response from login API
type SrunLoginResp struct {
	AccessToken   string `json:"access_token"`
	CheckoutDate  int    `json:"checkout_date"`
	ClientIP      string `json:"client_ip"`
	Ecode         int    `json:"ecode"`
	Error         string `json:"error"`
	ErrorMsg      string `json:"error_msg"`
	OnlineIP      string `json:"online_ip"`
	PloyMsg       string `json:"ploy_msg"`
	RealName      string `json:"real_name"`
	RemainFlux    int64  `json:"remain_flux"`
	RemainTimes   int    `json:"remain_times"`
	Res           string `json:"res"`
	ServerFlag    int    `json:"server_flag"`
	SrunVer       string `json:"srun_ver"`
	SucMsg        string `json:"suc_msg"`
	Sysver        string `json:"sysver"`
	Username      string `json:"username"`
	WalletBalance int64  `json:"wallet_balance"`
}

// getChallenge gets the challenge token from Srun portal
func (s *Service) getSrunChallenge(baseUrl, username, ip string) (*SrunChallengeResp, error) {
	callback := s.generateCallback()
	timestamp := time.Now().UnixMilli()

	params := url.Values{}
	params.Set("callback", callback)
	params.Set("username", username)
	params.Set("ip", ip)
	params.Set("_", strconv.FormatInt(timestamp, 10))

	reqUrl := fmt.Sprintf("%s%s?%s", baseUrl, srunGetChallenge, params.Encode())
	resp, err := s.requester.R().Get(reqUrl)
	if err != nil {
		return nil, ecode.RequestErr
	}
	if !resp.IsSuccess() {
		return nil, ecode.RequestErr
	}

	// Parse JSONP response
	jsonData, err := s.parseJSONP(resp.String())
	if err != nil {
		return nil, err
	}

	challengeResp := &SrunChallengeResp{}
	if err := json.Unmarshal([]byte(jsonData), challengeResp); err != nil {
		return nil, ecode.NewErrCode(-1, "failed to parse challenge response")
	}

	return challengeResp, nil
}

// SrunLogin performs login using Srun portal authentication
func (s *Service) SrunLogin() (*OnlineStatus, error) {
	baseUrl := defaultSrunBaseUrl
	if s.conf.SrunBaseUrl != "" {
		baseUrl = s.conf.SrunBaseUrl
	}

	acId := defaultSrunAcId
	if s.conf.AcId > 0 {
		acId = s.conf.AcId
	}

	// Get client IP first
	ip, err := s.getSrunClientIP(baseUrl)
	if err != nil {
		return nil, err
	}

	// Get challenge token
	challenge, err := s.getSrunChallenge(baseUrl, s.conf.Username, ip)
	if err != nil {
		return nil, err
	}

	if challenge.Res != "ok" {
		return nil, ecode.NewErrCode(challenge.Ecode, challenge.Error)
	}

	token := challenge.Challenge

	// Encode password with HMAC-MD5
	passwordEncoded := s.encodePassword(s.conf.Password, token)

	// Build info parameter
	info := s.buildInfo(s.conf.Username, s.conf.Password, ip, acId, token)

	// Build checksum
	chksum := s.buildChecksum(token, s.conf.Username, passwordEncoded, acId, ip, info)

	// Build login request
	callback := s.generateCallback()
	timestamp := time.Now().UnixMilli()

	params := url.Values{}
	params.Set("callback", callback)
	params.Set("action", "login")
	params.Set("username", s.conf.Username)
	params.Set("password", "{MD5}"+passwordEncoded)
	params.Set("os", "Windows 10")
	params.Set("name", "Windows")
	params.Set("double_stack", "0")
	params.Set("chksum", chksum)
	params.Set("info", info)
	params.Set("ac_id", strconv.Itoa(acId))
	params.Set("ip", ip)
	params.Set("n", "200")
	params.Set("type", "1")
	params.Set("_", strconv.FormatInt(timestamp, 10))

	reqUrl := fmt.Sprintf("%s%s?%s", baseUrl, srunPortalPath, params.Encode())
	resp, err := s.requester.R().Get(reqUrl)
	if err != nil {
		return nil, ecode.RequestErr
	}
	if !resp.IsSuccess() {
		return nil, ecode.RequestErr
	}

	// Parse JSONP response
	jsonData, err := s.parseJSONP(resp.String())
	if err != nil {
		return nil, err
	}

	loginResp := &SrunLoginResp{}
	if err := json.Unmarshal([]byte(jsonData), loginResp); err != nil {
		return nil, ecode.NewErrCode(-1, "failed to parse login response")
	}

	if loginResp.Res != "ok" || loginResp.Error != "ok" {
		errMsg := loginResp.ErrorMsg
		if errMsg == "" {
			errMsg = loginResp.Error
		}
		return nil, ecode.NewErrCode(loginResp.Ecode, errMsg)
	}

	onlineStatus := &OnlineStatus{
		Time:     time.Now(),
		Bytes:    "",
		Name:     loginResp.RealName,
		Ip:       loginResp.OnlineIP,
		Mac:      "",
		Username: loginResp.Username,
	}

	return onlineStatus, nil
}

// SrunLogout performs logout using Srun portal
func (s *Service) SrunLogout() error {
	baseUrl := defaultSrunBaseUrl
	if s.conf.SrunBaseUrl != "" {
		baseUrl = s.conf.SrunBaseUrl
	}

	acId := defaultSrunAcId
	if s.conf.AcId > 0 {
		acId = s.conf.AcId
	}

	// Get client IP
	ip, err := s.getSrunClientIP(baseUrl)
	if err != nil {
		return err
	}

	callback := s.generateCallback()
	timestamp := time.Now().UnixMilli()

	params := url.Values{}
	params.Set("callback", callback)
	params.Set("action", "logout")
	params.Set("username", s.conf.Username)
	params.Set("ac_id", strconv.Itoa(acId))
	params.Set("ip", ip)
	params.Set("_", strconv.FormatInt(timestamp, 10))

	reqUrl := fmt.Sprintf("%s%s?%s", baseUrl, srunPortalPath, params.Encode())
	resp, err := s.requester.R().Get(reqUrl)
	if err != nil {
		return ecode.RequestErr
	}
	if !resp.IsSuccess() {
		return ecode.RequestErr
	}

	// Parse JSONP response
	jsonData, err := s.parseJSONP(resp.String())
	if err != nil {
		return err
	}

	logoutResp := &SrunLoginResp{}
	if err := json.Unmarshal([]byte(jsonData), logoutResp); err != nil {
		return ecode.NewErrCode(-1, "failed to parse logout response")
	}

	if logoutResp.Error != "ok" && logoutResp.Error != "logout_ok" {
		return ecode.NewErrCode(logoutResp.Ecode, logoutResp.ErrorMsg)
	}

	return nil
}

// getSrunClientIP gets the client IP from Srun portal redirect
func (s *Service) getSrunClientIP(baseUrl string) (string, error) {
	resp, err := s.requester.R().Get(baseUrl)
	if err != nil {
		return "", ecode.RequestErr
	}

	// Try to extract IP from redirect URL or page content
	body := resp.String()

	// Pattern to match IP in URL parameters or JavaScript
	patterns := []string{
		`ip=(\d+\.\d+\.\d+\.\d+)`,
		`user_ip\s*[:=]\s*["']?(\d+\.\d+\.\d+\.\d+)`,
		`"ip"\s*:\s*"(\d+\.\d+\.\d+\.\d+)"`,
	}

	for _, pattern := range patterns {
		regex := regexp.MustCompile(pattern)
		matches := regex.FindStringSubmatch(body)
		if len(matches) > 1 {
			return matches[1], nil
		}
	}

	// Check Location header for redirect
	location := resp.Header().Get("Location")
	if location != "" {
		regex := regexp.MustCompile(`ip=(\d+\.\d+\.\d+\.\d+)`)
		matches := regex.FindStringSubmatch(location)
		if len(matches) > 1 {
			return matches[1], nil
		}
	}

	return "", ecode.NewErrCode(-1, "failed to get client IP")
}

// parseJSONP extracts JSON from JSONP response
func (s *Service) parseJSONP(response string) (string, error) {
	// Match jQuery callback format: jQuery...(...)
	regex := regexp.MustCompile(`^[a-zA-Z_$][a-zA-Z0-9_$]*\((.*)\)$`)
	matches := regex.FindStringSubmatch(strings.TrimSpace(response))
	if len(matches) > 1 {
		return matches[1], nil
	}
	return "", ecode.NewErrCode(-1, "invalid JSONP response format")
}

// generateCallback generates a jQuery-style callback name for JSONP requests.
// The format mimics jQuery's $.ajax callback naming: "jQuery{random}_{timestamp}"
// where random is a pseudo-random number derived from the current time.
func (s *Service) generateCallback() string {
	timestamp := time.Now().UnixMilli()
	// Generate a pseudo-random number by combining seconds and nanoseconds
	random := timestamp/1000*1000000 + int64(time.Now().Nanosecond()%1000000)
	return fmt.Sprintf("jQuery%d_%d", random, timestamp)
}

// encodePassword encodes password using HMAC-MD5 with token
func (s *Service) encodePassword(password, token string) string {
	h := hmac.New(md5.New, []byte(token))
	h.Write([]byte(password))
	return hex.EncodeToString(h.Sum(nil))
}

// buildInfo builds the encrypted info parameter
func (s *Service) buildInfo(username, password, ip string, acId int, token string) string {
	info := map[string]interface{}{
		"username": username,
		"password": password,
		"ip":       ip,
		"acid":     acId,
		"enc_ver":  "srun_bx1",
	}
	jsonBytes, err := json.Marshal(info)
	if err != nil {
		// Fallback to empty if marshal fails (shouldn't happen with basic types)
		jsonBytes = []byte("{}")
	}
	encoded := s.xEncode(string(jsonBytes), token)
	return "{SRBX1}" + s.base64Encode(encoded)
}

// Srun portal constants for checksum calculation
const (
	srunChecksumN    = "200" // n parameter for checksum
	srunChecksumType = "1"   // type parameter for checksum
)

// buildChecksum builds the SHA1 checksum
func (s *Service) buildChecksum(token, username, passwordMd5 string, acId int, ip, info string) string {
	// Checksum format: token + username + token + hmd5 + token + acid + token + ip + token + n + token + type + token + info
	data := fmt.Sprintf("%s%s%s%s%s%d%s%s%s%s%s%s%s%s",
		token, username, token, passwordMd5, token, acId, token, ip, token, srunChecksumN, token, srunChecksumType, token, info)
	h := sha1.New()
	h.Write([]byte(data))
	return hex.EncodeToString(h.Sum(nil))
}

// XEncode magic numbers (XXTEA algorithm constants)
// These are standard XXTEA cipher constants used by the Srun portal
const (
	xEncodeDelta = uint32(0x9E3779B9) // Golden ratio derived constant for XXTEA
)

// xEncode implements the XEncode algorithm used by Srun portal
// This is a modified XXTEA block cipher used by Srun for info parameter encryption
func (s *Service) xEncode(str, key string) string {
	if str == "" {
		return ""
	}

	v := s.strToBytes(str, true)
	k := s.strToBytes(key, false)

	if len(k) < 4 {
		k = append(k, make([]uint32, 4-len(k))...)
	}

	n := len(v) - 1
	z := v[n]
	y := v[0]
	d := uint32(0)
	q := 6 + 52/(n+1)

	for q > 0 {
		d += xEncodeDelta
		e := (d >> 2) & 3
		var p int
		for p = 0; p < n; p++ {
			y = v[p+1]
			m := (z>>5 ^ y<<2) + (y>>3 ^ z<<4) ^ (d ^ y) + (k[(uint32(p)&3)^e] ^ z)
			v[p] += m
			z = v[p]
		}
		y = v[0]
		m := (z>>5 ^ y<<2) + (y>>3 ^ z<<4) ^ (d ^ y) + (k[(uint32(p)&3)^e] ^ z)
		v[n] += m
		z = v[n]
		q--
	}

	return s.bytesToStr(v, false)
}

// strToBytes converts string to uint32 array for XEncode
func (s *Service) strToBytes(str string, includeLen bool) []uint32 {
	length := len(str)
	n := (length + 3) / 4

	if n == 0 {
		n = 1
	}

	var result []uint32
	if includeLen {
		result = make([]uint32, n+1)
		result[n] = uint32(length)
	} else {
		result = make([]uint32, n)
	}

	for i := 0; i < length; i++ {
		result[i/4] |= uint32(str[i]) << (uint32(i%4) * 8)
	}

	return result
}

// bytesToStr converts uint32 array back to string
func (s *Service) bytesToStr(data []uint32, includeLen bool) string {
	length := len(data)
	n := length * 4

	if includeLen {
		m := data[length-1]
		if m < uint32(n-3) || m > uint32(n) {
			return ""
		}
		n = int(m)
	}

	result := make([]byte, n)
	for i := 0; i < n; i++ {
		result[i] = byte(data[i/4] >> (uint32(i%4) * 8) & 0xff)
	}

	return string(result)
}

// base64Encode performs Srun-specific base64 encoding
// The Srun portal uses a custom base64 alphabet for encoding
func (s *Service) base64Encode(str string) string {
	// Srun portal custom base64 alphabet (64 characters)
	const alpha = "LVoJPiCN2R8G90yg+hmFHuacZ1OWMnrsSTXkYpUq/3dlbfE47w6xezA9BFIKtfj5"
	result := ""
	length := len(str)

	for i := 0; i < length; i += 3 {
		var b1, b2, b3 byte
		b1 = str[i]
		if i+1 < length {
			b2 = str[i+1]
		}
		if i+2 < length {
			b3 = str[i+2]
		}

		d1 := b1 >> 2
		d2 := ((b1 & 0x03) << 4) | (b2 >> 4)
		d3 := ((b2 & 0x0f) << 2) | (b3 >> 6)
		d4 := b3 & 0x3f

		result += string(alpha[d1])
		result += string(alpha[d2])

		if i+1 < length {
			result += string(alpha[d3])
		} else {
			result += "="
		}

		if i+2 < length {
			result += string(alpha[d4])
		} else {
			result += "="
		}
	}

	return result
}
