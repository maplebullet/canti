//#include "process.h"
//using namespace std;
//
//
//int main() {
//    //Jingtai jtresult = jingtai_analyce(R"(D:\project\butaifenlei\jiaoxing\2024_11_18_18_05_05.txt)", R"(D:\project\butaifenlei\model.onnx)");
//    Dongtai result=dongtai_analyce(R"(D:\project\butaifenlei\jiaoxing\2024.1.6\xuruidongman.txt)");
//    // Output the information of detected blocks
//    //show_video(frames, 500);
//    //for (const auto& block : blocks) {
//        //show_video(block.images, 500);
//    //}
//    return 0;
//}

#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>

int main(int argc, char* argv[])
{
    // 启用高DPI缩放支持（Qt 5.6 - Qt 5.x，Qt 6默认启用）
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    
    QApplication a(argc, argv);
    
    // 设计分辨率
    const int designWidth = 1920;
    const int designHeight = 1080;
    
    MainWindow w;
    
    // 获取主屏幕信息
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int screenWidth = screenGeometry.width();
        int screenHeight = screenGeometry.height();
        
        // 根据屏幕分辨率调整窗口大小
        double scaleX = static_cast<double>(screenWidth) / designWidth;
        double scaleY = static_cast<double>(screenHeight) / designHeight;
        double scale = qMin(scaleX, scaleY);  // 取较小值以保持纵横比
        
        int windowWidth = static_cast<int>(designWidth * scale);
        int windowHeight = static_cast<int>(designHeight * scale);
        
        // 如果屏幕足够大，使用设计尺寸；否则使用缩放后的尺寸
        if (screenWidth >= designWidth && screenHeight >= designHeight) {
            w.resize(designWidth, designHeight);
        } else {
            w.resize(windowWidth, windowHeight);
        }
        
        // 居中显示窗口
        w.move((screenWidth - w.width()) / 2, (screenHeight - w.height()) / 2);
    } else {
        // 如果无法获取屏幕信息，使用默认设计尺寸
        w.resize(designWidth, designHeight);
    }
    
    w.show();
    return a.exec();
}