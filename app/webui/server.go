package webui

import (
	"canti/app/conf"
	"canti/app/service"
	"embed"
	"encoding/json"
	"fmt"
	"io/fs"
	"net/http"
	"sync"
)

//go:embed static/*
var staticFiles embed.FS

type Server struct {
	service      *service.Service
	port         int
	mu           sync.RWMutex
	currentUser  string
	currentMethod string
	isLoggedIn   bool
}

func NewServer(port int) *Server {
	config := conf.Config{
		Method:    conf.LoginWebMethod,
		Reconnect: false,
		Silence:   true,
	}
	return &Server{
		service: service.NewService(config),
		port:    port,
		currentMethod: conf.LoginWebMethod,
	}
}

type LoginRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
	Method   string `json:"method"` // web, srun, pppoe
}

type Response struct {
	Success bool        `json:"success"`
	Message string      `json:"message"`
	Data    interface{} `json:"data,omitempty"`
}

func (s *Server) handleLogin(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req LoginRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		s.sendJSON(w, Response{Success: false, Message: "无效的请求"})
		return
	}

	if req.Username == "" || req.Password == "" {
		s.sendJSON(w, Response{Success: false, Message: "用户名和密码不能为空"})
		return
	}

	// 如果没有指定方法，使用默认方法
	method := req.Method
	if method == "" {
		method = conf.LoginWebMethod
	}

	// 验证方法是否有效
	if method != conf.LoginWebMethod && method != conf.LoginSRunMethod && method != conf.LoginPPPOEMethod {
		s.sendJSON(w, Response{Success: false, Message: "无效的认证方法"})
		return
	}

	config := conf.Config{
		Username:  req.Username,
		Password:  req.Password,
		Method:    method,
		Reconnect: false,
		Silence:   true,
	}
	s.service.SetConfig(config)

	var status interface{}
	var err error

	// 根据方法选择不同的登录函数
	switch method {
	case conf.LoginSRunMethod:
		status, err = s.service.SRunLogin()
	case conf.LoginWebMethod:
		status, err = s.service.WebLogin()
	case conf.LoginPPPOEMethod:
		status, err = s.service.PPPoELogin()
	default:
		s.sendJSON(w, Response{Success: false, Message: "不支持的认证方法"})
		return
	}

	if err != nil {
		s.sendJSON(w, Response{Success: false, Message: fmt.Sprintf("登录失败: %s", err.Error())})
		return
	}

	s.mu.Lock()
	s.currentUser = req.Username
	s.currentMethod = method
	s.isLoggedIn = true
	s.mu.Unlock()

	s.sendJSON(w, Response{
		Success: true,
		Message: "登录成功",
		Data:    status,
	})
}

func (s *Server) handleLogout(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	s.mu.RLock()
	method := s.currentMethod
	s.mu.RUnlock()

	var err error
	switch method {
	case conf.LoginSRunMethod:
		err = s.service.SRunLogout()
	case conf.LoginWebMethod:
		err = s.service.WebLogout()
	case conf.LoginPPPOEMethod:
		err = s.service.PPPoELogout()
	default:
		err = fmt.Errorf("未知的认证方法")
	}

	if err != nil {
		s.sendJSON(w, Response{Success: false, Message: fmt.Sprintf("登出失败: %s", err.Error())})
		return
	}

	s.mu.Lock()
	s.isLoggedIn = false
	s.mu.Unlock()

	s.sendJSON(w, Response{Success: true, Message: "登出成功"})
}

func (s *Server) handleStatus(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	s.mu.RLock()
	method := s.currentMethod
	s.mu.RUnlock()

	var status interface{}
	var err error

	switch method {
	case conf.LoginSRunMethod:
		status, err = s.service.SRunGetOnlineStatus()
	case conf.LoginWebMethod:
		status, err = s.service.WebGetOnlineStatus()
	case conf.LoginPPPOEMethod:
		status, err = s.service.PPPoEGetOnlineStatus()
	default:
		err = fmt.Errorf("未知的认证方法")
	}

	if err != nil {
		s.sendJSON(w, Response{Success: false, Message: fmt.Sprintf("获取状态失败: %s", err.Error())})
		return
	}

	s.sendJSON(w, Response{
		Success: true,
		Message: "在线",
		Data:    status,
	})
}

func (s *Server) sendJSON(w http.ResponseWriter, resp Response) {
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(resp)
}

func (s *Server) Start() error {
	// Serve static files
	staticFS, err := fs.Sub(staticFiles, "static")
	if err != nil {
		return err
	}
	http.Handle("/", http.FileServer(http.FS(staticFS)))

	// API endpoints
	http.HandleFunc("/api/login", s.handleLogin)
	http.HandleFunc("/api/logout", s.handleLogout)
	http.HandleFunc("/api/status", s.handleStatus)

	addr := fmt.Sprintf(":%d", s.port)
	fmt.Printf("Web界面启动在 http://localhost%s\n", addr)
	fmt.Printf("请在浏览器中打开上述地址进行操作\n")
	return http.ListenAndServe(addr, nil)
}
