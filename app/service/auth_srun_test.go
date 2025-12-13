package service

import (
	"canti/app/conf"
	"testing"
)

func TestEncodePassword(t *testing.T) {
	srv := NewService(conf.Config{})
	
	// Test HMAC-MD5 password encoding
	token := "testtoken123"
	password := "testpassword"
	
	encoded := srv.encodePassword(password, token)
	
	// The result should be a 32-character hex string
	if len(encoded) != 32 {
		t.Errorf("expected encoded password length 32, got %d", len(encoded))
	}
	
	// Same input should produce same output
	encoded2 := srv.encodePassword(password, token)
	if encoded != encoded2 {
		t.Errorf("encoding not consistent: %s != %s", encoded, encoded2)
	}
	
	// Different token should produce different output
	encoded3 := srv.encodePassword(password, "differenttoken")
	if encoded == encoded3 {
		t.Errorf("different tokens should produce different results")
	}
}

func TestBuildChecksum(t *testing.T) {
	srv := NewService(conf.Config{})
	
	token := "testtoken"
	username := "2025020632"
	passwordMd5 := "e85e2cadf6d9f42ab863fbfa942f908a"
	acId := 4
	ip := "10.242.182.12"
	info := "{SRBX1}testinfo"
	
	checksum := srv.buildChecksum(token, username, passwordMd5, acId, ip, info)
	
	// The result should be a 40-character hex string (SHA1)
	if len(checksum) != 40 {
		t.Errorf("expected checksum length 40, got %d", len(checksum))
	}
	
	// Same input should produce same output
	checksum2 := srv.buildChecksum(token, username, passwordMd5, acId, ip, info)
	if checksum != checksum2 {
		t.Errorf("checksum not consistent: %s != %s", checksum, checksum2)
	}
}

func TestXEncode(t *testing.T) {
	srv := NewService(conf.Config{})
	
	// Test with known input
	str := `{"username":"test","password":"pass","ip":"1.2.3.4","acid":1,"enc_ver":"srun_bx1"}`
	key := "testkey1234"
	
	encoded := srv.xEncode(str, key)
	
	// Result should not be empty
	if encoded == "" {
		t.Errorf("xEncode should not return empty string")
	}
	
	// Same input should produce same output
	encoded2 := srv.xEncode(str, key)
	if encoded != encoded2 {
		t.Errorf("xEncode not consistent")
	}
}

func TestBase64Encode(t *testing.T) {
	srv := NewService(conf.Config{})
	
	// Test with known input
	str := "hello world"
	encoded := srv.base64Encode(str)
	
	// Result should not be empty
	if encoded == "" {
		t.Errorf("base64Encode should not return empty string")
	}
	
	// Same input should produce same output
	encoded2 := srv.base64Encode(str)
	if encoded != encoded2 {
		t.Errorf("base64Encode not consistent")
	}
}

func TestParseJSONP(t *testing.T) {
	srv := NewService(conf.Config{})
	
	// Test valid JSONP response
	jsonp := `jQuery1234567890_1234567890({"key":"value"})`
	json, err := srv.parseJSONP(jsonp)
	if err != nil {
		t.Errorf("parseJSONP failed: %v", err)
	}
	if json != `{"key":"value"}` {
		t.Errorf("expected {\"key\":\"value\"}, got %s", json)
	}
	
	// Test another valid format
	jsonp2 := `callback_function({"result":123})`
	json2, err := srv.parseJSONP(jsonp2)
	if err != nil {
		t.Errorf("parseJSONP failed: %v", err)
	}
	if json2 != `{"result":123}` {
		t.Errorf("expected {\"result\":123}, got %s", json2)
	}
	
	// Test invalid format
	invalid := `not a jsonp response`
	_, err = srv.parseJSONP(invalid)
	if err == nil {
		t.Errorf("parseJSONP should fail on invalid input")
	}
}

func TestGenerateCallback(t *testing.T) {
	srv := NewService(conf.Config{})
	
	callback := srv.generateCallback()
	
	// Should start with "jQuery"
	if len(callback) < 6 || callback[:6] != "jQuery" {
		t.Errorf("callback should start with 'jQuery', got %s", callback)
	}
}

func TestBuildInfo(t *testing.T) {
	srv := NewService(conf.Config{})
	
	username := "2025020632"
	password := "testpassword"
	ip := "10.242.182.12"
	acId := 4
	token := "testtoken123"
	
	info := srv.buildInfo(username, password, ip, acId, token)
	
	// Should start with {SRBX1}
	if len(info) < 7 || info[:7] != "{SRBX1}" {
		t.Errorf("info should start with '{SRBX1}', got %s", info[:min(20, len(info))])
	}
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}
