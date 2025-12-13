// +build gui

package gui

import (
	"canti/app/conf"
	"canti/app/service"
	"encoding/json"
	"fmt"
	
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"
	"fyne.io/fyne/v2"
)

type GUI struct {
	service      *service.Service
	window       fyne.Window
	usernameEntry *widget.Entry
	passwordEntry *widget.Entry
	statusLabel  *widget.Label
	infoLabel    *widget.Label
}

func NewGUI() *GUI {
	config := conf.Config{
		Method:    conf.LoginWebMethod,
		Reconnect: false,
		Silence:   true,
	}
	
	return &GUI{
		service: service.NewService(config),
	}
}

func (g *GUI) Start() {
	myApp := app.New()
	g.window = myApp.NewWindow("Canti - 校园网认证")
	
	// Create UI elements
	g.usernameEntry = widget.NewEntry()
	g.usernameEntry.SetPlaceHolder("请输入学号")
	
	g.passwordEntry = widget.NewPasswordEntry()
	g.passwordEntry.SetPlaceHolder("请输入密码")
	
	g.statusLabel = widget.NewLabel("")
	g.infoLabel = widget.NewLabel("")
	
	loginBtn := widget.NewButton("登录", func() {
		g.login()
	})
	
	logoutBtn := widget.NewButton("登出", func() {
		g.logout()
	})
	
	statusBtn := widget.NewButton("查看状态", func() {
		g.checkStatus()
	})
	
	// Layout
	content := container.NewVBox(
		widget.NewLabel("🔐 Canti"),
		widget.NewLabel("WUST 校园网认证客户端"),
		widget.NewSeparator(),
		widget.NewLabel("用户名（学号）:"),
		g.usernameEntry,
		widget.NewLabel("密码:"),
		g.passwordEntry,
		container.NewGridWithColumns(2, loginBtn, logoutBtn),
		statusBtn,
		widget.NewSeparator(),
		g.statusLabel,
		g.infoLabel,
	)
	
	g.window.SetContent(content)
	g.window.Resize(fyne.NewSize(400, 500))
	g.window.ShowAndRun()
}

func (g *GUI) login() {
	username := g.usernameEntry.Text
	password := g.passwordEntry.Text
	
	if username == "" || password == "" {
		g.statusLabel.SetText("❌ 请输入用户名和密码")
		return
	}
	
	g.statusLabel.SetText("⏳ 正在登录...")
	
	config := conf.Config{
		Username:  username,
		Password:  password,
		Method:    conf.LoginWebMethod,
		Reconnect: false,
		Silence:   true,
	}
	g.service.SetConfig(config)
	
	status, err := g.service.WebLogin()
	if err != nil {
		g.statusLabel.SetText(fmt.Sprintf("❌ 登录失败: %s", err.Error()))
		g.infoLabel.SetText("")
		return
	}
	
	g.statusLabel.SetText("✅ 登录成功")
	g.displayStatus(status)
}

func (g *GUI) logout() {
	g.statusLabel.SetText("⏳ 正在登出...")
	
	err := g.service.WebLogout()
	if err != nil {
		g.statusLabel.SetText(fmt.Sprintf("❌ 登出失败: %s", err.Error()))
		return
	}
	
	g.statusLabel.SetText("✅ 登出成功")
	g.infoLabel.SetText("")
}

func (g *GUI) checkStatus() {
	g.statusLabel.SetText("⏳ 正在查询状态...")
	
	status, err := g.service.WebGetOnlineStatus()
	if err != nil {
		g.statusLabel.SetText(fmt.Sprintf("❌ 查询失败: %s", err.Error()))
		g.infoLabel.SetText("")
		return
	}
	
	g.statusLabel.SetText("✅ 在线")
	g.displayStatus(status)
}

func (g *GUI) displayStatus(status *service.OnlineStatus) {
	jsonBytes, _ := json.MarshalIndent(status, "", "  ")
	info := fmt.Sprintf("姓名: %s\n账号: %s\nIP: %s\nMAC: %s\n已用流量: %s",
		status.Name, status.Username, status.Ip, status.Mac, status.Bytes)
	g.infoLabel.SetText(info)
}
