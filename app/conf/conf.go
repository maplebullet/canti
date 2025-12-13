package conf

const (
	LoginWebMethod   = "web"
	LoginPPPOEMethod = "pppoe"
	LoginSrunMethod  = "srun"
)

type Config struct {
	Username    string `json:"username" yaml:"username"`
	Password    string `json:"password" yaml:"password"`
	Method      string `json:"method" yaml:"method"`
	Reconnect   bool   `json:"reconnect" yaml:"reconnect"`
	Silence     bool   `json:"silence" yaml:"silence"`
	SrunBaseUrl string `json:"srunBaseUrl" yaml:"srunBaseUrl"` // Srun portal base URL (e.g., http://10.145.255.21)
	AcId        int    `json:"acId" yaml:"acId"`               // Srun access control ID (default: 4)
}
