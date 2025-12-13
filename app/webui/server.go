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
	}
}

type LoginRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
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

	config := conf.Config{
		Username:  req.Username,
		Password:  req.Password,
		Method:    conf.LoginWebMethod,
		Reconnect: false,
		Silence:   true,
	}
	s.service.SetConfig(config)

	status, err := s.service.WebLogin()
	if err != nil {
		s.sendJSON(w, Response{Success: false, Message: fmt.Sprintf("登录失败: %s", err.Error())})
		return
	}

	s.mu.Lock()
	s.currentUser = req.Username
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

	err := s.service.WebLogout()
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

	status, err := s.service.WebGetOnlineStatus()
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
