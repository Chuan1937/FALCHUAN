#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_FALCHUAN.h"
#include <armadillo>
#include "zfp.h"
class FALCHUAN : public QMainWindow
{
    Q_OBJECT


public:
    FALCHUAN(QWidget* parent = nullptr);
    ~FALCHUAN();
    // arma::fmat TraceData; // 地震数据矩阵
   //static  QString CompressFilePathName; //需要压缩的文件路径

public slots:
    //void on_<object name>_<signal name>(<signal parameters>);
    //就不用再写connect了(还没成功，但至少不会重复弹出了)

    //1.选择需要压缩的文件路径
    void on_ChooseCompressFile_CompressFilePath_clicked();

    //2.选择压缩文件存放路径
    void on_ChooseCompressStoreFile_CompressFileStorePath_clicked();

    //3.点击”加载需要压缩的文件路径“按钮加载文件
    void on_ReadCompressPathButton_ReadCompressPathFile_cliked();

    //4.从加载数据页面跳转到压缩参数页面
    void on_NextPage_page2_clicked();

    //6. 从主页面跳转到加载数据页面
    void on_COMPRESS_page1_clicked();

    //7. 从加载数据页面跳转到主页面
    void on_BackMain_page3_clicked();

    //8.从压缩参数页面跳转到加载数据页面
    void on_BackCompress_page1_clicked();

    //9.从主页面跳转到解压缩
    void on_DECOMPRESS__page4_clicked();

    //10.从解压缩页面跳转到主页面
    void on_BackfirstMain_page3_clicked();

    //11.选择需要解压缩的文件路径
    void on_NeedDecompressPath_NeedDecompressPathFile_clicked();
    //12.选择解压后的文件存放文件夹
    void on_DecompressStorePath_DecompressStorePathFile_clicked();

    //13.开始压缩按钮
    void on_StartCompress_CompressBrowser_clicked();

    //14.开始解压缩按钮
    void on_StartDecompress_DecompressBrowser_clicked();

    //15.从压缩页面退出
    void on_CMQuit_Quit_clicked();

    //16.从解压缩页面退出
    void on_DEQuit_Quit_clicked();

private:
    Ui::Seismic ui;

};

