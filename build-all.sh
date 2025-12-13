#!/usr/bin/env bash
set -e

export VERSION=0.0.1-dev
export VERSION_GO_VAR=canti/app/version.Version
LDFLAGS="-X ${VERSION_GO_VAR}=${VERSION} -s -w"

# 创建构建目录
rm -rf ./build
mkdir -p ./build

echo "=========================================="
echo "开始构建 Canti 多版本"
echo "=========================================="

# CLI版本 - 适用于OpenWrt和服务器
echo ""
echo "构建 CLI 版本（包含Web UI）..."
echo "=========================================="

# OpenWrt MIPS
echo "构建 OpenWrt MIPS..."
CGO_ENABLED=0 GOOS=linux GOARCH=mipsle GOMIPS=softfloat go build -ldflags "${LDFLAGS}" -o ./build/canti-openwrt-mipsle .
echo "✓ OpenWrt MIPS 完成"

# OpenWrt ARM
echo "构建 OpenWrt ARM..."
CGO_ENABLED=0 GOOS=linux GOARCH=arm GOARM=7 go build -ldflags "${LDFLAGS}" -o ./build/canti-openwrt-arm7 .
echo "✓ OpenWrt ARM7 完成"

# Linux AMD64 (CLI + WebUI)
echo "构建 Linux AMD64..."
CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -ldflags "${LDFLAGS}" -o ./build/canti-linux-amd64 .
echo "✓ Linux AMD64 完成"

# Windows AMD64 (CLI + WebUI)
echo "构建 Windows AMD64..."
CGO_ENABLED=0 GOOS=windows GOARCH=amd64 go build -ldflags "${LDFLAGS}" -o ./build/canti-windows-amd64.exe .
echo "✓ Windows AMD64 完成"

# Windows 386 (CLI + WebUI)
echo "构建 Windows 386..."
CGO_ENABLED=0 GOOS=windows GOARCH=386 go build -ldflags "${LDFLAGS}" -o ./build/canti-windows-386.exe .
echo "✓ Windows 386 完成"

echo ""
echo "=========================================="
echo "构建 GUI 版本（Windows桌面）..."
echo "=========================================="

# Windows GUI版本 (需要CGO)
echo "构建 Windows GUI AMD64..."
if command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    CGO_ENABLED=1 GOOS=windows GOARCH=amd64 CC=x86_64-w64-mingw32-gcc go build -tags gui -ldflags "${LDFLAGS} -H=windowsgui" -o ./build/canti-gui-windows-amd64.exe .
    echo "✓ Windows GUI AMD64 完成"
else
    echo "⚠ 跳过 Windows GUI 构建（需要 mingw-w64）"
    echo "  在 Windows 上可以运行: go build -tags gui -ldflags \"-H=windowsgui\" -o canti-gui.exe ."
fi

echo ""
echo "=========================================="
echo "压缩二进制文件..."
echo "=========================================="

cd build

# 压缩 (如果有upx)
if command -v upx &> /dev/null; then
    echo "使用 UPX 压缩..."
    for file in canti-*; do
        if [ -f "$file" ]; then
            echo "压缩 $file..."
            upx -8 "$file" 2>/dev/null || echo "  跳过 $file"
        fi
    done
    echo "✓ 压缩完成"
else
    echo "⚠ UPX 未安装，跳过压缩"
fi

echo ""
echo "=========================================="
echo "创建发布包..."
echo "=========================================="

# 创建tar.gz包
for file in canti-*; do
    if [ -f "$file" ]; then
        echo "打包 $file..."
        tar -czf "${file}.tar.gz" "$file"
    fi
done

cd ..

echo ""
echo "=========================================="
echo "构建完成！"
echo "=========================================="
echo "构建产物位于 ./build/ 目录："
ls -lh ./build/ | grep -E "\.tar\.gz$|\.exe$|canti-"
echo ""
echo "说明："
echo "  - canti-openwrt-*: OpenWrt路由器版本（CLI + Web UI）"
echo "  - canti-linux-*: Linux服务器版本（CLI + Web UI）"
echo "  - canti-windows-*.exe: Windows命令行版本（CLI + Web UI）"
echo "  - canti-gui-windows-*.exe: Windows图形界面版本（Fyne GUI）"
echo ""
echo "使用方法："
echo "  CLI模式: ./canti login -u 学号 -p 密码"
echo "  Web模式: ./canti webui"
echo "  GUI模式: ./canti-gui.exe (仅限GUI版本)"
