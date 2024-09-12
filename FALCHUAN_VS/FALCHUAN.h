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
    // arma::fmat TraceData; // �������ݾ���
   //static  QString CompressFilePathName; //��Ҫѹ�����ļ�·��

public slots:
    //void on_<object name>_<signal name>(<signal parameters>);
    //�Ͳ�����дconnect��(��û�ɹ��������ٲ����ظ�������)

    //1.ѡ����Ҫѹ�����ļ�·��
    void on_ChooseCompressFile_CompressFilePath_clicked();

    //2.ѡ��ѹ���ļ����·��
    void on_ChooseCompressStoreFile_CompressFileStorePath_clicked();

    //3.�����������Ҫѹ�����ļ�·������ť�����ļ�
    void on_ReadCompressPathButton_ReadCompressPathFile_cliked();

    //4.�Ӽ�������ҳ����ת��ѹ������ҳ��
    void on_NextPage_page2_clicked();

    //6. ����ҳ����ת����������ҳ��
    void on_COMPRESS_page1_clicked();

    //7. �Ӽ�������ҳ����ת����ҳ��
    void on_BackMain_page3_clicked();

    //8.��ѹ������ҳ����ת����������ҳ��
    void on_BackCompress_page1_clicked();

    //9.����ҳ����ת����ѹ��
    void on_DECOMPRESS__page4_clicked();

    //10.�ӽ�ѹ��ҳ����ת����ҳ��
    void on_BackfirstMain_page3_clicked();

    //11.ѡ����Ҫ��ѹ�����ļ�·��
    void on_NeedDecompressPath_NeedDecompressPathFile_clicked();
    //12.ѡ���ѹ����ļ�����ļ���
    void on_DecompressStorePath_DecompressStorePathFile_clicked();

    //13.��ʼѹ����ť
    void on_StartCompress_CompressBrowser_clicked();

    //14.��ʼ��ѹ����ť
    void on_StartDecompress_DecompressBrowser_clicked();

    //15.��ѹ��ҳ���˳�
    void on_CMQuit_Quit_clicked();

    //16.�ӽ�ѹ��ҳ���˳�
    void on_DEQuit_Quit_clicked();

private:
    Ui::Seismic ui;

};

