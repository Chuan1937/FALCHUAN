#include "FALCHUAN.h"
#include <QFileDialog>
#include <QRegularExpression>
#include <QMessageBox>
#include <QCheckBox>
#include "zfp.h"
#include <armadillo>
#include <omp.h>
#pragma comment(lib,"zfp.lib")


/////����ѹ���ʵ�����£�bit_per_value��ѹ���ʹ�ϵΪѹ����=-3.1279171*bit_per_value+99.99380986;
///bit_per_value=��-99.99380986+ѹ���ʣ�/-3.1279171;


//////////////////////////////////////ȫ�ֱ���///////////////////////////////
#include <QGlobalStatic>
Q_GLOBAL_STATIC(QString, globalCompressFilePathName)
Q_GLOBAL_STATIC(arma::fmat, globalTraceData)
Q_GLOBAL_STATIC(size_t, globalcompressed_size_out)
Q_GLOBAL_STATIC(size_t, globalfilesize)
Q_GLOBAL_STATIC(size_t, globalget_rows)
Q_GLOBAL_STATIC(size_t, globalSample)

FALCHUAN::FALCHUAN(QWidget* parent)
    : QMainWindow(parent)
{

    ui.setupUi(this);

    // ��������ʱ��ʾ��ҳ����������������Ϊ��һ��ҳ��
    ui.stackedWidget->setCurrentIndex(0);

    //1.
    connect(ui.ChooseCompressFile, SIGNAL(clicked()), this, SLOT(on_ChooseCompressFile_CompressFilePath_clicked()));
    //2.
    connect(ui.ChooseCompressStoreFile, SIGNAL(clicked()), this, SLOT(on_ChooseCompressStoreFile_CompressFileStorePath_clicked()));
    //3.
    connect(ui.ReadCompressPathButton, SIGNAL(clicked()), this, SLOT(on_ReadCompressPathButton_ReadCompressPathFile_cliked()));
    //4.
    connect(ui.NextPage, SIGNAL(clicked()), this, SLOT(on_NextPage_page2_clicked()));


    //5.ռλ����Ĭ����ʾ�ı�
    ui.TraceStart->setPlaceholderText(QString::fromLocal8Bit("�˴�Ĭ��Ϊ1����ʾ�����һ�С�"));
    ui.TraceEnd->setPlaceholderText(QString::fromLocal8Bit("�˴�Ĭ�����������,Ҳ����ȫ��ѹ��"));

    ui.Ratio->setPlaceholderText(QString::fromLocal8Bit("90.6"));
    // Ĭ�Ͻ���ֵ

    ui.DeCompressStartTrace->setPlaceholderText(QString::fromLocal8Bit("Ĭ��Ϊ1����ʾ�����һ�С�"));//��ѹ����ʼ�����

    ui.DeCompressEndTrace->setPlaceholderText(QString::fromLocal8Bit("Ĭ��Ϊ����У���ʾȫ����ѹ��"));//��ѹ�����������

    //Ĭ�Ϲ�ѡfrm
    ui.FRM->setChecked(true);

    //6.
    connect(ui.COMPRESS, SIGNAL(clicked()), this, SLOT(on_COMPRESS_page1_clicked()));

    //7.
    connect(ui.BackMain, SIGNAL(clicked()), this, SLOT(on_BackMain_page3_clicked()));

    //8.
    connect(ui.BackCompress, SIGNAL(clicked()), this, SLOT(on_BackCompress_page1_clicked()));

    //9.
    connect(ui.DECOMPRESS, SIGNAL(clicked()), this, SLOT(on_DECOMPRESS__page4_clicked()));

    //10.
    connect(ui.BackfirstMain, SIGNAL(clicked()), this, SLOT(on_BackfirstMain_page3_clicked()));

    //11.
    connect(ui.NeedDecompressPath, SIGNAL(clicked()), this, SLOT(on_NeedDecompressPath_NeedDecompressPathFile_clicked()));

    //12.
    connect(ui.DecompressStorePath, SIGNAL(clicked()), this, SLOT(on_DecompressStorePath_DecompressStorePathFile_clicked()));

    //13.
    connect(ui.StartCompress, SIGNAL(clicked()), this, SLOT(on_StartCompress_CompressBrowser_clicked()));

    //14.
    connect(ui.StartDecompress, SIGNAL(clicked()), this, SLOT(on_StartDecompress_DecompressBrowser_clicked()));

    //15.
    connect(ui.CMQuit, SIGNAL(clicked()), this, SLOT(on_CMQuit_Quit_clicked()));

    //16.
    connect(ui.DEQuit, SIGNAL(clicked()), this, SLOT(on_DEQuit_Quit_clicked()));
}
FALCHUAN::~FALCHUAN() {}





////////////////////////1.�����ѡ����Ҫѹ�����ļ�����ťѡ���ļ�////////////////////////
void FALCHUAN::on_ChooseCompressFile_CompressFilePath_clicked()
{
    QString CompressFilePathName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("��ѡ������Ҫѹ�����ļ�"), "", QString("Geoeast inner files (*.bin)"));
    if (!CompressFilePathName.isEmpty())
    {
        *globalCompressFilePathName() = CompressFilePathName;
        // ��·���еķ�б�ܼӱ�
        CompressFilePathName.replace("\\", "\\\\");
        ui.CompressFilePath->setPlainText(CompressFilePathName);
    }
    else
    {
        QMessageBox::warning(this, "Invalid Path", QString::fromLocal8Bit("δѡ���ļ���·����Ч."));
    }
}






////////////////////////2.�����ѡ��ѹ���ļ����·������ťѡ���ļ���////////////////
void FALCHUAN::on_ChooseCompressStoreFile_CompressFileStorePath_clicked()
{
    ui.FRM->setChecked(true);
    bool isFRM = ui.FRM->isChecked();


    QString rate = ui.Ratio->text();

    size_t get_rows_f = *globalget_rows();
    size_t Samples_f = *globalSample();

    if (isFRM) {


        ///ѹ����Ĭ��86.89%��4.2��
        double compress_ratio = 90.6;
        if (!rate.isEmpty()) {
            compress_ratio = rate.toDouble();
        }
        ///bit_per_value=��-99.99380986+ѹ���ʣ�/-3.1279171;
        double bpv_temp = (-99.99380986 + compress_ratio) / -3.1279171;
        double bpv = std::round(bpv_temp * 10.0) / 10.0; // ����һλС��
        // // �ڽ���ѡ�񱣴�·��֮ǰ���ȼ���Ƿ�ѡ����ѹ��������ѹ����
        // if ((!isFRM && !isFLM)) {
        //     QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("�����ѡ��ѹ������"));
        //     return;
        // }





        // ����Ĭ���ļ���������������������
        QString defaultFileName = "compress";
        if (isFRM) {
            defaultFileName += QString(".frm_%1_%2_%3")
                .arg(QString::number(bpv * 10, 'f', 0))
                .arg(QString::number(get_rows_f))
                .arg(QString::number(Samples_f));
        }


        // ��ȡ�ļ�����·��
        QString CompressStorePathName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("Please select the file to store your compressed data"), defaultFileName, QString("FRL Files(.*)"));
        if (!CompressStorePathName.isEmpty())
        {
            // ��·���еķ�б�ܼӱ�
            CompressStorePathName.replace("\\", "\\\\");

            ui.CompressFileStorePath->setPlainText(CompressStorePathName);
        }
        else
        {
            QMessageBox::warning(this, "Invalid Path", QString::fromLocal8Bit("δѡ���ļ���·����Ч."));
        }
    }

    //  if (isFLM) {
             //
    //      /////compress_ratio = 54.64 * tolerance^0.142+8.68(0.001....1.1)
    //          ////tolerance =3.468e-21 * compress_ratio^11.45(13%-60)
    //
    //          /////compress_ratio =  -217.6 * tolerance^(-0.01902 )+ 278.5 (5....60)
    //          ///tolerance = 3.395e-30  * compress_ratio^16.53 (67% -80)
    //
    //      double compress_ratio = 61.66964613;
    //      double Tolerance = 1.111105;
    //      double TLA = 1.111105;
    //      if (!rate.isEmpty()) {
    //          compress_ratio = rate.toDouble();
       //       if (compress_ratio < 15) {
       //           compress_ratio = 15;
       //       }
       //       if (compress_ratio > 75) {
       //           compress_ratio = 75;
       //       }
    //          if(13<compress_ratio <60) {
             // 	Tolerance = 3.468e-21 * pow(compress_ratio, 11.45);
    //              TLA = std::round(Tolerance * 1000000.0) / 1000000.0; // ������λС��
             // }
             // if(60<compress_ratio<80) {
             // 	Tolerance = 3.395e-30 * pow(compress_ratio, 16.53);
    //              TLA = std::round(Tolerance * 1000000.0) / 1000000.0; // ������λС��
             // }
    //      }
    //
    //      // �ڽ���ѡ�񱣴�·��֮ǰ���ȼ���Ƿ�ѡ����ѹ��������ѹ����
    //      if ((!isFRM && !isFLM)) {
    //          QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("�����ѡ��ѹ������"));
    //          return;
    //      }
    //
    //      // ����Ĭ���ļ���������������������
    //      QString defaultFileName = "compress";
    //      if (isFLM) {
    //          defaultFileName += QString(".flm_%1_%2_%3")
    //              .arg(QString::number(TLA * 1000000, 'f', 0))
    //              .arg(QString::number(get_rows_f))
    //              .arg(QString::number(Samples_f));
    //      }
    //
    //      // ��ȡ�ļ�����·��
    //      QString CompressStorePathName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("Please select the file to store your compressed data"), defaultFileName, QString("FLM Files(.*)"));
    //      if (!CompressStorePathName.isEmpty())
    //      {
    //          // ��·���еķ�б�ܼӱ�
    //          CompressStorePathName.replace("\\", "\\\\");
    //
    //          ui.CompressFileStorePath->setPlainText(CompressStorePathName);
    //      }
    //      else
    //      {
    //          QMessageBox::warning(this, "Invalid Path", QString::fromLocal8Bit("δѡ���ļ���·����Ч."));
    //      }
    //  }


}





////////////////////////3.�����������Ҫѹ�����ļ�·������ť�����ļ�/////////////////////
void FALCHUAN::on_ReadCompressPathButton_ReadCompressPathFile_cliked()
{
    arma::fmat TraceData;
    if (globalCompressFilePathName->isEmpty()) {
        QMessageBox::warning(this, "Invalid Path", QString::fromLocal8Bit("�ļ�·����Ч��δѡ���ļ�."));
        return;
    }
    std::string CompressFilePathNameStr = globalCompressFilePathName->toStdString();

    QString allTraceCountStr = ui.AllTrace->text();
    QString samplesPerTraceStr = ui.Sample->text();

    // ��� All_T �� Samples �Ƿ�Ϊ��
    if (allTraceCountStr.isEmpty() || samplesPerTraceStr.isEmpty()) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("�����������ļ���������������"));
        return;
    }

    size_t All_T = allTraceCountStr.toULongLong();
    size_t Samples = samplesPerTraceStr.toULongLong();

    *globalSample() = Samples; //����SamplesΪȫ�ֱ���

    // ��ȡ t_start �� t_end�����û������������Ĭ��ֵ
    size_t t_start = 0;
    size_t t_end = All_T - 1;

    QString t_startStr = ui.TraceStart->text();
    QString t_endStr = ui.TraceEnd->text();

    if (!t_startStr.isEmpty()) {
        t_start = t_startStr.toULongLong() - 1;
    }

    if (!t_endStr.isEmpty()) {
        t_end = t_endStr.toULongLong() - 1;
    }

    size_t get_rows = t_end - t_start + 1;

    *globalget_rows() = get_rows; //����get_rowsΪȫ�ֱ���

    ui.ReadCompressPathFile->setPlainText(QString::fromLocal8Bit("��ʼ�����������ݡ�\n ---------------------------------"));
    QCoreApplication::processEvents();  // ǿ��ˢ��UI


    //////////��ȡ���ݲ���ʱ����������д��TraceData��////////////////////////
    auto read_start = std::chrono::high_resolution_clock::now();

    if (t_start > t_end || t_end > All_T || t_start < 0) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("���������з�Χѡȡ�Ƿ���ȷ"));
    }
    std::ifstream file(CompressFilePathNameStr, std::ios::binary);
    if (!file.is_open()) {
        QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("�޷����ļ�"));
    }

    size_t data_size = get_rows * Samples * sizeof(float);
    file.seekg(0, std::ios::end);

    size_t filesize;
    filesize = file.tellg();//��ȡ�ļ���С

    *globalfilesize() = filesize; //����filesizeΪȫ�ֱ���

    //��ʼ������Ҫ������
    file.seekg(t_start * Samples * sizeof(float), std::ios::beg);

    TraceData.set_size(Samples, get_rows);  // �Ȱ������ȴ洢
    if (!file.read(reinterpret_cast<char*>(TraceData.memptr()), data_size)) {
        QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("�޷���ȡ�ļ�"));
    }
    file.close();

    TraceData = TraceData.t();  // ת�þ���ʹ�䰴�����ȴ洢
    *globalTraceData() = TraceData; //����TraceDataΪȫ�ֱ���

    auto read_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> read_duration = read_end - read_start;

    ui.ReadCompressPathFile->append(QString::fromLocal8Bit("���ݼ�����ɣ���ʱ��%1 �롣\n--------------------------------- ").arg(read_duration.count()));



    // ��ʾ���ص����ݣ��� ReadCompressPathFile ��� QTextBrowser ����ʾ
    QString displayText = QString::fromLocal8Bit("���ļ���СΪ��%1 �ֽڡ�\n���������Ѿ�������ɡ�\n ������ɡ�����")
        .arg(filesize);
    ui.ReadCompressPathFile->append(displayText);
}



//////////////////4.�Ӽ�������ҳ����ת��ѹ������ҳ��//////////////////////////////////
void FALCHUAN::on_NextPage_page2_clicked() {
    QString allTraceCountStr = ui.AllTrace->text();
    QString samplesPerTraceStr = ui.Sample->text();

    // ��� All_T �� Samples �Ƿ�Ϊ��
    if (allTraceCountStr.isEmpty() || samplesPerTraceStr.isEmpty()) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("�����������ļ��л�������"));
        return;
    }


    ui.stackedWidget->setCurrentIndex(2);
}


//////////////////6. ����ҳ����ת����������ҳ��//////////////////////////////////
void FALCHUAN::on_COMPRESS_page1_clicked() {
    ui.stackedWidget->setCurrentIndex(1);
}

//////////////////7. �Ӽ�������ҳ����ת����ҳ��////////////////////////////////
void FALCHUAN::on_BackMain_page3_clicked() {
    ui.stackedWidget->setCurrentIndex(0);
}

///////////////////8.��ѹ������ҳ����ת����������ҳ��////////////////////////////
void FALCHUAN::on_BackCompress_page1_clicked() {
    ui.stackedWidget->setCurrentIndex(1);
}


///////////////////9.����ҳ����ת����ѹ��//////////////////////////////////////
void FALCHUAN::on_DECOMPRESS__page4_clicked() {
    ui.stackedWidget->setCurrentIndex(3);
}

///////////////////10.�ӽ�ѹ��ҳ����ת����ҳ��//////////////////////////////////
void FALCHUAN::on_BackfirstMain_page3_clicked() {
    ui.stackedWidget->setCurrentIndex(0);
}

///////////////////11.ѡ����Ҫ��ѹ�����ļ�·��//////////////////////////////////
void FALCHUAN::on_NeedDecompressPath_NeedDecompressPathFile_clicked() {
    QStringList filters;
    filters << "Geoeast inner files (*.frm* *.flm*)" // ƥ������ .frm* ��.flm�ļ�
        << "All Files (*)"; // ƥ�������ļ�

    QFileDialog dialog(this);
    dialog.setNameFilters(filters);
    dialog.setWindowTitle(QString::fromLocal8Bit("��ѡ������Ҫ��ѹ�����ļ�"));
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec() == QDialog::Accepted) {
        QString NeedDecompressPathName = dialog.selectedFiles().first();
        if (!NeedDecompressPathName.isEmpty()) {
            // ��·���еķ�б�ܼӱ�
            NeedDecompressPathName.replace("\\", "\\\\");
            ui.NeedDecompressPathFile->setPlainText(NeedDecompressPathName);
        }
    }
};


///////////////////12.ѡ���ѹ����ļ�����ļ���////////////////////////////////
void FALCHUAN::on_DecompressStorePath_DecompressStorePathFile_clicked() {
    // ��ȡԴ�ļ�·��������
    QString sourceFilePath = ui.NeedDecompressPathFile->toPlainText();
    if (sourceFilePath.isEmpty()) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("����ѡ��ѹ��Դ�ļ�"));
        return;
    }

    //��ȡ�ļ���������·����
    QFileInfo fileInfo(sourceFilePath);
    //QString baseName = fileInfo.completeBaseName(); // ��ȡ�ļ�����������չ����
    QString baseName = "Decompressed";

    // ����Ĭ�Ͻ�ѹ·�����ļ���
    QString defaultDecompressPath = fileInfo.path() + "/" + baseName + ".bin";

    QString DecompressStorePathName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("Please select the folder where your decompressed files are stored"), defaultDecompressPath, "bin files (*.bin)");
    if (!DecompressStorePathName.isEmpty())
    {
        // ��·���еķ�б�ܼӱ�
        DecompressStorePathName.replace("\\", "\\\\");

        ui.DecompressStorePathFile->setPlainText(DecompressStorePathName);
    }
};


//////////////////////////13.��ʼѹ����ť/////////////////////////////////////
void FALCHUAN::on_StartCompress_CompressBrowser_clicked()
{
    try {
        //��ȡCPU����߳���
        int threads = omp_get_max_threads();


        //QString CompressStorePathName; // �Ѿ��洢��ѹ���ļ����·��
        if (globalTraceData->is_empty()) {
            QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("TraceData δ��ʼ��"));
            return;
        }
        arma::fmat TraceData;
        TraceData = *globalTraceData();

        // ��ȡ����,��ת��Ϊstd::string(ѹ���ļ��洢�ļ���λ��)
        std::string CompressStorePathNameStr = ui.CompressFileStorePath->toPlainText().toStdString();

        if (CompressStorePathNameStr.empty()) {
            QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("����ѡ��ѹ���ļ����·��"));
            return;
        }




        // ��ȡ FRM �� FLM ��ѡ���״̬
        ui.FRM->setChecked(true);
        bool isFRMChecked = ui.FRM->isChecked();
        //bool isFLMChecked = ui.FLM->isChecked();

  //       if (!isFLMChecked and !isFRMChecked ) {
        // 	QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("�����ѡ��һ��ѹ������"));
        // 	return;
        // }

        

        if (isFRMChecked) {
            // ���� FRM ѹ������

             // ѹ��������ѹ���ʣ�ѹ������
            double compress_ratio = 90.6; // ʹ��Ĭ��ֵ

            QString R = ui.Ratio->text();

            if (!R.isEmpty()) {
                compress_ratio = R.toDouble(); // ʹ���û������ֵ
            }


            /// bit_per_value=��-99.99380986+ѹ���ʣ�/-3.1279171;
            double bpv_temp = (-99.9938 + compress_ratio) / -3.1279;
            double bpv = std::round(bpv_temp * 10.0) / 10.0; // ����һλС��



            ui.CompressBrowser->setPlainText(QString::fromLocal8Bit("��ʼ����ѹ��������\n ---------------------------------"));
            QCoreApplication::processEvents(); // ǿ��ˢ��UI


            ////////////////////��ʼѹ��////////////////////////////////////////
            auto compress_start = std::chrono::high_resolution_clock::now();
            zfp_type type = zfp_type_float;

            // ��� TraceData �Ƿ��ѳ�ʼ��
            if (TraceData.is_empty()) {
                QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("TraceData δ��ʼ��"));
                return;
            }


            zfp_field* field = zfp_field_2d(TraceData.memptr(), type, TraceData.n_cols, TraceData.n_rows);
            zfp_stream* zfp = zfp_stream_open(NULL);
            zfp_stream_set_rate(zfp, bpv, type, 2, 0);
            zfp_stream_set_execution(zfp, zfp_exec_omp);
            zfp_stream_set_omp_threads(zfp, threads - 2);

            size_t bufsize = zfp_stream_maximum_size(zfp, field);

            std::vector<char> buffer; // ���建����
            buffer.resize(bufsize);

            bitstream* stream = stream_open(buffer.data(), bufsize);
            zfp_stream_set_bit_stream(zfp, stream);
            zfp_stream_rewind(zfp);

            size_t compressed_size = zfp_compress(zfp, field);
            if (!compressed_size) {
                QMessageBox::warning(this, "Compression failed", QString::fromLocal8Bit("ѹ��ʧ�ܣ������ԣ�"));
                return;
            }
            size_t compressed_size_out;
            compressed_size_out = compressed_size; // ѹ�����ļ���С
            *globalcompressed_size_out() = compressed_size_out;


            zfp_field_free(field);
            zfp_stream_close(zfp);
            stream_close(stream);

            auto end_compress = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> compress_duration = end_compress - compress_start;
            /////////////////ѹ�������////////////////////////////////////////

            if (*globalfilesize() == 0) {
                QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("filesize δ��ʼ��"));
                return;
            }

            size_t filesize = *globalfilesize();
            double Calculate_Rate = 100.0 * (1.0 - (double)compressed_size_out / (double)filesize);

            // ui.CompressBrowser->append(QString::fromLocal8Bit("Total filesizes in origin file is ��%1 byte \n the file compressed is %2 byte")
            //     .arg(filesize)
            // .arg(compressed_size_out));
            // QCoreApplication::processEvents(); // ǿ��ˢ��UI

            if (*globalget_rows() == 0) {
                QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("get_rows δ��ʼ��"));
            }

            size_t get_rows = *globalget_rows();

            double per_trace_byte = (double)compressed_size_out / get_rows;

            ui.CompressBrowser->append(QString::fromLocal8Bit("\n����ѹ��ʹ��ʱ��Ϊ��%1 �롣\n�˴ε�ѹ����Ϊ��%2 %\n��ʼ��ѹ������д���ļ�������\n---------------------------------")
                .arg(compress_duration.count())
                .arg(Calculate_Rate));
            QCoreApplication::processEvents(); // ǿ��ˢ��UI

            //////////////��ѹ������д���ļ�//////////////////////////////////////
            auto write_start = std::chrono::high_resolution_clock::now();
            std::ofstream file(CompressStorePathNameStr, std::ios::binary);
            if (!file.is_open()) {
                QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("�޷����ļ�"));
            }
            else {
                file.write(buffer.data(), compressed_size_out);
                file.close();
            }

            auto write_end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> write_duration = write_end - write_start;

            ui.CompressBrowser->append(QString::fromLocal8Bit("ѹ������д���ļ���ɡ�\n�˴�����д�뻨��ʱ��Ϊ��%1 �롣\nѹ���ļ���СΪ��%2 �ֽڡ�\n---------------------------------\n ������ɡ�����")
                .arg(write_duration.count())
                .arg(compressed_size_out));
        }








        // if (isFLMChecked) {
        //     // ���� FLM ѹ������
        //
        //     // ui.CompressBrowser->setPlainText(QString::fromLocal8Bit("�㷨���ڿ����С�����\n --------------------------------- \n �������������"));
        //
        //     /////compress_ratio = 54.64 * tolerance^0.142+8.68(0.001....1.1)
        //     ////tolerance =3.468e-21 * compress_ratio^11.45
        //
        //     /////compress_ratio =  -217.6 * tolerance^(-0.01902 )+ 278.5 (5....60)
        //     ///tolerance = 3.395e-30  * compress_ratio^16.53
        //      // ѹ��������ѹ���ʣ�ѹ������
        //     QString rate = ui.Ratio->text();
        //     double compress_ratio = 61.66964613;
        //     double Tolerance = 1.111105;
        //     double TLA = 1.111105;
        //     if (!rate.isEmpty()) {
        //         compress_ratio = rate.toDouble();
        //         if (compress_ratio < 15) {
        //             compress_ratio = 15;
        //         }
        //         if (compress_ratio > 75) {
        //             compress_ratio = 75;
        //         }
        //         if (13 < compress_ratio < 60) {
        //             Tolerance = 3.468e-21 * pow(compress_ratio, 11.45);
        //             TLA = std::round(Tolerance * 1000000.0) / 1000000.0; // ������λС��
        //         }
        //         if (60 < compress_ratio < 80) {
        //             Tolerance = 3.395e-30 * pow(compress_ratio, 16.53);
        //             TLA = std::round(Tolerance * 1000000.0) / 1000000.0; // ������λС��
        //         }
        //     }
        //
        //
        //
        //     ui.CompressBrowser->setPlainText(QString::fromLocal8Bit("��ʼ����ѹ��������\n ---------------------------------"));
        //     QCoreApplication::processEvents(); // ǿ��ˢ��UI
        //
        //
        //     ////////////////////��ʼѹ��////////////////////////////////////////
        //     auto compress_start = std::chrono::high_resolution_clock::now();
        //     zfp_type type = zfp_type_float;
        //
        //     // ��� TraceData �Ƿ��ѳ�ʼ��
        //     if (TraceData.is_empty()) {
        //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("TraceData δ��ʼ��"));
        //         return;
        //     }
        //
        //
        //     zfp_field* field = zfp_field_2d(TraceData.memptr(), type, TraceData.n_cols, TraceData.n_rows);
        //     zfp_stream* zfp = zfp_stream_open(NULL);
        //
        //
        //     // ����Ϊ�̶�����ģʽ
        //     zfp_stream_set_accuracy(zfp, TLA);
        //
        //
        //     zfp_stream_set_execution(zfp, zfp_exec_omp);
        //     zfp_stream_set_omp_threads(zfp, threads - 2);
        //
        //     size_t bufsize = zfp_stream_maximum_size(zfp, field);
        //
        //     std::vector<char> buffer; // ���建����
        //     buffer.resize(bufsize);
        //
        //     bitstream* stream = stream_open(buffer.data(), bufsize);
        //     zfp_stream_set_bit_stream(zfp, stream);
        //     zfp_stream_rewind(zfp);
        //
        //     size_t compressed_size = zfp_compress(zfp, field);
        //     if (!compressed_size) {
        //         QMessageBox::warning(this, "Compression failed", QString::fromLocal8Bit("ѹ��ʧ�ܣ������ԣ�"));
        //         return;
        //     }
        //     size_t compressed_size_out;
        //     compressed_size_out = compressed_size; // ѹ�����ļ���С
        //     *globalcompressed_size_out() = compressed_size_out;
        //
        //
        //     zfp_field_free(field);
        //     zfp_stream_close(zfp);
        //     stream_close(stream);
        //
        //     auto end_compress = std::chrono::high_resolution_clock::now();
        //     std::chrono::duration<double> compress_duration = end_compress - compress_start;
        //     /////////////////ѹ�������////////////////////////////////////////
        //
        //     if (*globalfilesize() == 0) {
        //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("filesize δ��ʼ��"));
        //         return;
        //     }
        //
        //     size_t filesize = *globalfilesize();
        //     double Calculate_Rate = 100.0 * (1.0 - (double)compressed_size_out / (double)filesize);
        //
        //     // ui.CompressBrowser->append(QString::fromLocal8Bit("Total filesizes in origin file is ��%1 byte \n the file compressed is %2 byte")
        //     //     .arg(filesize)
        //     // .arg(compressed_size_out));
        //     // QCoreApplication::processEvents(); // ǿ��ˢ��UI
        //
        //     if (*globalget_rows() == 0) {
        //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("get_rows δ��ʼ��"));
        //     }
        //
        //     size_t get_rows = *globalget_rows();
        //
        //     double per_trace_byte = (double)compressed_size_out / get_rows;
        //
        //     ui.CompressBrowser->append(QString::fromLocal8Bit("����ѹ���Ѿ���ɡ�\nѹ����ÿ��������ֽ�����ͬ����Ϊ��%1 �ֽڡ�\n����ѹ��ʹ��ʱ��Ϊ��%2 �롣\n�˴ε�ѹ����Ϊ��%3 %\n��ʼ��ѹ������д���ļ�������\n---------------------------------")
        //         .arg(per_trace_byte)
        //         .arg(compress_duration.count())
        //         .arg(Calculate_Rate));
        //     QCoreApplication::processEvents(); // ǿ��ˢ��UI
        //
        //     //////////////��ѹ������д���ļ�//////////////////////////////////////
        //     auto write_start = std::chrono::high_resolution_clock::now();
        //     std::ofstream file(CompressStorePathNameStr, std::ios::binary);
        //     if (!file.is_open()) {
        //         QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("�޷����ļ�"));
        //     }
        //     else {
        //         file.write(buffer.data(), compressed_size_out);
        //         file.close();
        //     }
        //
        //     auto write_end = std::chrono::high_resolution_clock::now();
        //     std::chrono::duration<double> write_duration = write_end - write_start;
        //
        //     ui.CompressBrowser->append(QString::fromLocal8Bit("ѹ������д���ļ���ɡ�\n�˴�����д�뻨��ʱ��Ϊ��%1 �롣\nѹ���ļ���СΪ��%2 �ֽڡ�\n---------------------------------\n ������ɡ�����")
        //         .arg(write_duration.count())
        //         .arg(compressed_size_out));
        // }






    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("����������") + e.what());
    }
    catch (...) {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("������δ֪����"));
    }
}







/////////////////////////////////////14.��ʼ��ѹ����ť///////////////////////
void FALCHUAN::on_StartDecompress_DecompressBrowser_clicked() {



    std::vector<char> Need_decompress;
    std::string NeedDecompressPathFileStr = ui.NeedDecompressPathFile->toPlainText().toStdString();
    //��ȡ�����ѹ������ļ���ַ
    std::string DecompressStorePathFileStr = ui.DecompressStorePathFile->toPlainText().toStdString();




    if (NeedDecompressPathFileStr.empty()) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("����ѡ����Ҫ��ѹ�����ļ�"));

    }

    if (DecompressStorePathFileStr.empty()) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("Please select the unzipped file storage folder first"));
    }

    ///////////////��ȡ��ѹ����Ҫ�Ĳ���////////////////////////////////////
    QString NeedDecompressPathName = ui.NeedDecompressPathFile->toPlainText();
    // ��ȡ�ļ���
    QFileInfo fileInfo(NeedDecompressPathName);
    QString fileName = fileInfo.fileName(); // ��ȡ�����ļ�����������׺
    ////////////frm�㷨
    // ʹ��������ʽ��ȡ������ʶ�������ֲ��ּ���������������
    QRegularExpression re("([a-zA-Z]+)_(\\d+)_(\\d+)_([0-9]+)");
    QRegularExpressionMatch match = re.match(fileName);

    QString letters = match.captured(1); // �� "frm"
    QString bpvNumber = match.captured(2); // �����ʲ��֣��̶���λ��
    //QString torlance = match.captured(2); // �ݲ�֣��̶�6λ��
    QString rows = match.captured(3); // ������λ���ɱ�
    QString samples = match.captured(4); // ������

    // ��bpvNumberת��ΪС����ʽ
    double numericValue = bpvNumber.toDouble() / 10.0; // ת��ΪС����ʽ

    //double torlanceValue = torlance.toDouble() / 1000000.0; // ת��ΪС����ʽ


    // ��rows��samplesת��Ϊsize_t
    size_t get_rows_f = rows.toULongLong();
    size_t Samples_f = samples.toULongLong();

    // ��Ҫ��ѹ������ʼ������ͽ�������������û������������Ĭ��ֵ
    size_t DeCoStartTrace = 0;
    size_t DeCoEndTrace = get_rows_f - 1;

    QString DCST = ui.DeCompressStartTrace->text();
    QString DCET = ui.DeCompressEndTrace->text();

    if (!DCST.isEmpty()) {
        DeCoStartTrace = DCST.toULongLong() - 1; // ��Ҫ��ѹ����ʼ�����
    }

    if (!DCET.isEmpty()) {
        DeCoEndTrace = DCET.toULongLong() - 1; // ��Ҫ��ѹ�Ľ��������
    }


    //////////////��ȡ��Ҫ��ѹ�����ļ�//////////////////////////////////////
    if (letters == "frm") {

        std::ifstream file(NeedDecompressPathFileStr, std::ios::binary | std::ios::ate);

        if (!file.is_open()) {
            QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("�޷����ļ�"));
        }

        std::streampos size = file.tellg();//��ȡ��ѹ���ļ��ܴ�С
        file.seekg(0, std::ios::beg);

        Need_decompress.resize(size);
        if (!file.read(Need_decompress.data(), size)) {
            QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("�޷���ȡ�ļ�"));

        }
        file.close();

        ///////////////////////////////��ʼ��ѹ��////////////////////////////
        ui.DecompressBrowser->setPlainText(QString::fromLocal8Bit("\n��ʼ��ѹ����������\n ---------------------------------"));
        QCoreApplication::processEvents();//ǿ��ˢ��UI



        // if (*globalget_rows == 0) {
        //     QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("get_rows δ��ʼ��"));
        // }
        //
        // if (*globalSample == 0) {
        //     QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("Samples δ��ʼ��"));
        // }

        //size_t get_trace = *globalget_rows();
        //size_t Samples = *globalSample();
        arma::fmat decompressedData(get_rows_f, Samples_f);
        //��ѹ���������

        auto decompress_start = std::chrono::high_resolution_clock::now();


        if (Need_decompress.empty()) {
            QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("Need_decompress��Ҫ��ѹ������δ��ʼ��"));
        }

        zfp_type type = zfp_type_float;
        zfp_field* field = zfp_field_2d(decompressedData.memptr(), type, Samples_f, get_rows_f);

        if (!field) {
            QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("�ڲ�fieldδ��ʼ��"));
        }

        zfp_stream* zfp = zfp_stream_open(NULL);
        if (!zfp) {
            QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("�ڲ�zfpδ��ʼ��"));
            zfp_field_free(field);
        }

        //��ѹ��ʱ������ͬ����
        zfp_stream_set_rate(zfp, numericValue, type, 2, 0);

        const char* dataPtr = reinterpret_cast<const char*>(Need_decompress.data());
        size_t bufferByteSize = Need_decompress.size();

        bitstream* stream = stream_open((void*)dataPtr, bufferByteSize);

        if (!stream) {
            QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("�ڲ�streamδ��ʼ��"));
            zfp_field_free(field);
            zfp_stream_close(zfp);
        }

        zfp_stream_set_bit_stream(zfp, stream);
        zfp_stream_rewind(zfp);

        if (!zfp_decompress(zfp, field)) {
            QMessageBox::warning(this, "Decompression failed", QString::fromLocal8Bit("��ѹ��ʧ�ܣ������ԣ�"));

        }

        stream_close(stream);
        zfp_stream_close(zfp);
        zfp_field_free(field);

        auto decompress_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> decompress_duration = decompress_end - decompress_start;

        //��ѹ��������ݣ���ȡ���ض������У�
        arma::fmat LastDecompressData = decompressedData.rows(DeCoStartTrace, DeCoEndTrace);

        //����decompressedData�Ĵ�С
        size_t decom_filesize = (DeCoEndTrace - DeCoStartTrace + 1) * Samples_f * sizeof(float);

        ui.DecompressBrowser->append(QString::fromLocal8Bit("���Ľ�ѹ�������Ѿ���ɡ�\n ���ν�ѹ������ʱ��Ϊ��%1 �롣\n ���ڿ�ʼ������ѹ���������д����̡�����\n ---------------------------------")
            .arg(decompress_duration.count()));
        QCoreApplication::processEvents();//ǿ��ˢ��UI

        //////д�����
        // ��decompressedDataת��Ϊһά����
        auto one_start = std::chrono::high_resolution_clock::now();
        // �Խ�ѹ����ľ������ת��
        arma::fmat transposedData = LastDecompressData.t();
        // ��ת�ú�ľ���ת��Ϊһά���飨�����ȣ�

        std::vector<float> oneDimensionalArray(transposedData.memptr(), transposedData.memptr() + transposedData.n_elem);


        std::ofstream outputFile(DecompressStorePathFileStr, std::ios::binary);
        outputFile.write(reinterpret_cast<const char*>(oneDimensionalArray.data()), oneDimensionalArray.size() * sizeof(float));
        outputFile.close();


        auto one_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> compress_duration = one_end - one_start;

        ui.DecompressBrowser->append(QString::fromLocal8Bit(" ��ѹ������д���ļ���ɡ�\n�˴�����д�뻨��ʱ��Ϊ��%1 �롣\n --------------------------------- \n ������ɡ�������")
            .arg(compress_duration.count()));
    }

    // if(letters=="flm") {
       //  		// ���� FLM ѹ������
    //
    //     std::ifstream file(NeedDecompressPathFileStr, std::ios::binary | std::ios::ate);
    //
    //     if (!file.is_open()) {
    //         QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("�޷����ļ�"));
    //     }
    //
    //     std::streampos size = file.tellg();//��ȡ��ѹ���ļ��ܴ�С
    //     file.seekg(0, std::ios::beg);
    //
    //     Need_decompress.resize(size);
    //     if (!file.read(Need_decompress.data(), size)) {
    //         QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("�޷���ȡ�ļ�"));
    //
    //     }
    //     file.close();
    //
    //     ///////////////////////////////��ʼ��ѹ��////////////////////////////
    //     ui.DecompressBrowser->setPlainText(QString::fromLocal8Bit("\n��ʼ��ѹ��������\n����Ҫ��ѹ�����ļ���СΪ��%1 �ֽڡ�\n ����Ҫ��ѹ�������ĵ����Ϊ�� <%2> ������ <%3> ��һ�� <%4> ���������\n ---------------------------------")
    //         .arg(size)
    //         .arg(DeCoStartTrace + 1)
    //         .arg(DeCoEndTrace + 1)
    //         .arg(DeCoEndTrace - DeCoStartTrace + 1));
    //     QCoreApplication::processEvents();//ǿ��ˢ��UI
    //
    //
    //
    //     // if (*globalget_rows == 0) {
    //     //     QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("get_rows δ��ʼ��"));
    //     // }
    //     //
    //     // if (*globalSample == 0) {
    //     //     QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("Samples δ��ʼ��"));
    //     // }
    //
    //     //size_t get_trace = *globalget_rows();
    //     //size_t Samples = *globalSample();
    //     arma::fmat decompressedData(get_rows_f, Samples_f);
    //     //��ѹ���������
    //
    //     auto decompress_start = std::chrono::high_resolution_clock::now();
    //
    //
    //     if (Need_decompress.empty()) {
    //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("Need_decompress��Ҫ��ѹ������δ��ʼ��"));
    //     }
    //
    //     zfp_type type = zfp_type_float;
    //     zfp_field* field = zfp_field_2d(decompressedData.memptr(), type, Samples_f, get_rows_f);
    //
    //     if (!field) {
    //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("�ڲ�fieldδ��ʼ��"));
    //     }
    //
    //     zfp_stream* zfp = zfp_stream_open(NULL);
    //     if (!zfp) {
    //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("�ڲ�zfpδ��ʼ��"));
    //         zfp_field_free(field);
    //     }
    //
    //     //��ѹ��ʱ������ͬ����
    //     // ����Ϊ�̶�����ģʽ
    //     zfp_stream_set_accuracy(zfp, torlanceValue);
    //
    //     const char* dataPtr = reinterpret_cast<const char*>(Need_decompress.data());
    //     size_t bufferByteSize = Need_decompress.size();
    //
    //     bitstream* stream = stream_open((void*)dataPtr, bufferByteSize);
    //
    //     if (!stream) {
    //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("�ڲ�streamδ��ʼ��"));
    //         zfp_field_free(field);
    //         zfp_stream_close(zfp);
    //     }
    //
    //     zfp_stream_set_bit_stream(zfp, stream);
    //     zfp_stream_rewind(zfp);
    //
    //     if (!zfp_decompress(zfp, field)) {
    //         QMessageBox::warning(this, "Decompression failed", QString::fromLocal8Bit("��ѹ��ʧ�ܣ������ԣ�"));
    //
    //     }
    //
    //     stream_close(stream);
    //     zfp_stream_close(zfp);
    //     zfp_field_free(field);
    //
    //     auto decompress_end = std::chrono::high_resolution_clock::now();
    //     std::chrono::duration<double> decompress_duration = decompress_end - decompress_start;
    //
    //     //��ѹ��������ݣ���ȡ���ض������У�
    //     arma::fmat LastDecompressData = decompressedData.rows(DeCoStartTrace, DeCoEndTrace);
    //
    //     //����decompressedData�Ĵ�С
    //     size_t decom_filesize = (DeCoEndTrace - DeCoStartTrace + 1) * Samples_f * sizeof(float);
    //
    //     ui.DecompressBrowser->append(QString::fromLocal8Bit("���Ľ�ѹ�������Ѿ���ɡ�\n ���ν�ѹ������ʱ��Ϊ��%1 �롣\n ���ڿ�ʼ������ѹ���������д����̡�����\n ---------------------------------")
    //         .arg(decompress_duration.count()));
    //     QCoreApplication::processEvents();//ǿ��ˢ��UI
    //
    //     //////д�����
    //     // ��decompressedDataת��Ϊһά����
    //     auto one_start = std::chrono::high_resolution_clock::now();
    //     // �Խ�ѹ����ľ������ת��
    //     arma::fmat transposedData = LastDecompressData.t();
    //     // ��ת�ú�ľ���ת��Ϊһά���飨�����ȣ�
    //
    //     std::vector<float> oneDimensionalArray(transposedData.memptr(), transposedData.memptr() + transposedData.n_elem);
    //
    //
    //     std::ofstream outputFile(DecompressStorePathFileStr, std::ios::binary);
    //     outputFile.write(reinterpret_cast<const char*>(oneDimensionalArray.data()), oneDimensionalArray.size() * sizeof(float));
    //     outputFile.close();
    //
    //
    //     auto one_end = std::chrono::high_resolution_clock::now();
    //     std::chrono::duration<double> compress_duration = one_end - one_start;
    //
    //     ui.DecompressBrowser->append(QString::fromLocal8Bit(" ��ѹ������д���ļ���ɡ�\n�˴�����д�뻨��ʱ��Ϊ��%1 �롣\n��ѹ������ļ��ܴ�СΪ %2 �ֽڡ�\n --------------------------------- \n ������ɡ�������")
    //         .arg(compress_duration.count())
    //         .arg(decom_filesize));
    //
    //
    // }

}





/////////////////////////15.��ѹ��ҳ���˳�/////////////////////////
void FALCHUAN::on_CMQuit_Quit_clicked()
{
    this->close();
}

///////////////////////16.�ӽ�ѹ��ҳ���˳�////////////////////////
void FALCHUAN::on_DEQuit_Quit_clicked()
{
    this->close();
}