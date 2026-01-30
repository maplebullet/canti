#ifndef GLOBALDATA_H
#define GLOBALDATA_H
#include<QString>
#include<QPushButton>
//#include <QSqlDatabase>
//#include <QSqlQuery>
//#include<QSqlRecord>

class globaldata
{
public:
//    globaldata();


    static  QString sqlFile;
    static  int keynum_db_star;
    static  int keynum_db_end;
    static  QPushButton* openCollection;
    static  QString reportAdress;
    static  std::string  m_onnx  ;
    static  QString  m_json  ;
    static  QString  currentPrintPDF  ;
    static QString userInfoTemp;
    static double  left_up;
    static double  left_back;
    static double left_mid;//左上比例，左下比例
    static double right_up;
    static double right_back;
    static double right_mid;//右上比例,右下比例

    static double starpertcentNum;
    static double endpertcentNum;
    static double maxvalue;
    static double minvalue;
 //   static Jingtai result_Jingtai;
    //    QImage heartImage=*(mapper_->mainCanvas_);
    // static QImage Jingtai_leftTuoyuan;
    // static QImage Jingtai_rightTuoyuan;
    // static QImage Jingtai_right_leftTuoyuan;
    // static QImage Jingtai_right_leftzuobiao;
    static QString  reportPath_jingtai;
    static QString reportPath_dongtai ;
    static QString  dataRecordPath_jingtai;
    static QString dataRecordPath_dongtai ;
    static QImage* image_zudi;
    static QString user_id;       // 编号（字符串）
    static QString user_name;     // 姓名（字符串）
    static int user_height;       // 身高（整数）
    static int user_weight;       // 体重（整数）
    static int user_age;          // 年龄（整数）
    static float user_bmi;        // BMI（浮点数）
    static QString user_gender;


};

#endif // GLOBALDATA_H
