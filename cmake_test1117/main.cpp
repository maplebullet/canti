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

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}