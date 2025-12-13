package conf

const (
	LoginWebMethod   = "web"   // WUST原系统
	LoginPPPOEMethod = "pppoe" // PPPoE拨号
	LoginSRunMethod  = "srun"  // SRun深澜系统
)

type Config struct {
	Username  string `json:"username" yaml:"username"`
	Password  string `json:"password" yaml:"password"`
	Method    string `json:"method" yaml:"method"` // web, pppoe, srun
	Reconnect bool   `json:"reconnect" yaml:"reconnect"`
	Silence   bool   `json:"silence" yaml:"silence"`
}
