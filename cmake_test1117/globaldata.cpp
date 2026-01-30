#include "globaldata.h"

QString globaldata::sqlFile ="";
int globaldata::keynum_db_star=0;
int globaldata::keynum_db_end=0;
QPushButton* globaldata::openCollection=NULL;

QString globaldata::reportAdress="";
std::string globaldata::m_onnx="";
QString globaldata::m_json=" ";
//QString sqlFile_=globaldata::sqlFile;
QString  globaldata::currentPrintPDF="";

QString globaldata::userInfoTemp="";

double globaldata::left_up=0.0, globaldata::left_back=0.0,globaldata::left_mid=0.0;//左上比例，左下比例
double globaldata::right_up=0.0, globaldata::right_back=0.0,globaldata::right_mid=0.0;//右上比例,右下比例

 double globaldata::starpertcentNum=1.0,globaldata::endpertcentNum=99.8;

 double globaldata::maxvalue=500.0,globaldata::minvalue=0.0;
   //Jingtai globaldata::result_Jingtai;
  // QImage globaldata::Jingtai_leftTuoyuan;
  // QImage globaldata::Jingtai_rightTuoyuan;
  // QImage globaldata::Jingtai_right_leftTuoyuan;
  // QImage globaldata::Jingtai_right_leftzuobiao;
    QString  globaldata::reportPath_jingtai="";
    QString globaldata::reportPath_dongtai="" ;
     QString  globaldata::dataRecordPath_jingtai="" ;
     QString globaldata::dataRecordPath_dongtai ="" ;
     QImage* globaldata::image_zudi=NULL;

     QString globaldata::user_id = "";
     QString globaldata::user_name = "";
     int globaldata::user_height = 0;
     int globaldata::user_weight = 0;
     int globaldata::user_age = 0;
     float globaldata::user_bmi = 0;
     QString globaldata::user_gender = "";


