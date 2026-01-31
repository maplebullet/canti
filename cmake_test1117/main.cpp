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

int main(int argc, char* argv[])
{
    // 启用高DPI缩放支持（Qt 5.6 - Qt 5.x，Qt 6默认启用）
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    
    QApplication a(argc, argv);
    
    // 获取屏幕分辨率，计算缩放比例（以1920x1080为基准）
    QScreen *screen = QGuiApplication::primaryScreen();
    double scaleFactor = 1.0;
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int screenWidth = screenGeometry.width();
        // 以1920为基准计算缩放比例
        scaleFactor = static_cast<double>(screenWidth) / 1920.0;
        // 限制缩放范围在0.5到2.0之间
        scaleFactor = qBound(0.5, scaleFactor, 2.0);
    }
    
    MainWindow w(nullptr, scaleFactor);
    
    // 一打开就最大化显示，自动适配当前屏幕分辨率
    w.showMaximized();
    
    return a.exec();
}