#include "FALCHUAN.h"
#include <QFileDialog>
#include <QRegularExpression>
#include <QMessageBox>
#include <QCheckBox>
#include "zfp.h"
#include <armadillo>
#include <omp.h>
#pragma comment(lib,"zfp.lib")


/////设置压缩率的情况下，bit_per_value与压缩率关系为压缩率=-3.1279171*bit_per_value+99.99380986;
///bit_per_value=（-99.99380986+压缩率）/-3.1279171;


//////////////////////////////////////全局变量///////////////////////////////
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

    // 设置启动时显示的页面索引，例如设置为第一个页面
    ui.stackedWidget->setCurrentIndex(0);

    //1.
    connect(ui.ChooseCompressFile, SIGNAL(clicked()), this, SLOT(on_ChooseCompressFile_CompressFilePath_clicked()));
    //2.
    connect(ui.ChooseCompressStoreFile, SIGNAL(clicked()), this, SLOT(on_ChooseCompressStoreFile_CompressFileStorePath_clicked()));
    //3.
    connect(ui.ReadCompressPathButton, SIGNAL(clicked()), this, SLOT(on_ReadCompressPathButton_ReadCompressPathFile_cliked()));
    //4.
    connect(ui.NextPage, SIGNAL(clicked()), this, SLOT(on_NextPage_page2_clicked()));


    //5.占位符，默认提示文本
    ui.TraceStart->setPlaceholderText(QString::fromLocal8Bit("此处默认为1，表示数组第一行。"));
    ui.TraceEnd->setPlaceholderText(QString::fromLocal8Bit("此处默认数组最大行,也就是全部压缩"));

    ui.Ratio->setPlaceholderText(QString::fromLocal8Bit("90.6"));
    // 默认结束值

    ui.DeCompressStartTrace->setPlaceholderText(QString::fromLocal8Bit("默认为1，表示数组第一行。"));//解压缩起始地震道

    ui.DeCompressEndTrace->setPlaceholderText(QString::fromLocal8Bit("默认为最大行，表示全部解压缩"));//解压缩结束地震道

    //默认勾选frm
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





////////////////////////1.点击”选择需要压缩的文件“按钮选择文件////////////////////////
void FALCHUAN::on_ChooseCompressFile_CompressFilePath_clicked()
{
    QString CompressFilePathName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("请选择您需要压缩的文件"), "", QString("Geoeast inner files (*.bin)"));
    if (!CompressFilePathName.isEmpty())
    {
        *globalCompressFilePathName() = CompressFilePathName;
        // 将路径中的反斜杠加倍
        CompressFilePathName.replace("\\", "\\\\");
        ui.CompressFilePath->setPlainText(CompressFilePathName);
    }
    else
    {
        QMessageBox::warning(this, "Invalid Path", QString::fromLocal8Bit("未选择文件或路径无效."));
    }
}






////////////////////////2.点击”选择压缩文件存放路径“按钮选择文件夹////////////////
void FALCHUAN::on_ChooseCompressStoreFile_CompressFileStorePath_clicked()
{
    ui.FRM->setChecked(true);
    bool isFRM = ui.FRM->isChecked();


    QString rate = ui.Ratio->text();

    size_t get_rows_f = *globalget_rows();
    size_t Samples_f = *globalSample();

    if (isFRM) {


        ///压缩率默认86.89%（4.2）
        double compress_ratio = 90.6;
        if (!rate.isEmpty()) {
            compress_ratio = rate.toDouble();
        }
        ///bit_per_value=（-99.99380986+压缩率）/-3.1279171;
        double bpv_temp = (-99.99380986 + compress_ratio) / -3.1279171;
        double bpv = std::round(bpv_temp * 10.0) / 10.0; // 保留一位小数
        // // 在进行选择保存路径之前，先检查是否选择了压缩方法和压缩率
        // if ((!isFRM && !isFLM)) {
        //     QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("请必须选择压缩方法"));
        //     return;
        // }





        // 构建默认文件名并加入行数和样本数
        QString defaultFileName = "compress";
        if (isFRM) {
            defaultFileName += QString(".frm_%1_%2_%3")
                .arg(QString::number(bpv * 10, 'f', 0))
                .arg(QString::number(get_rows_f))
                .arg(QString::number(Samples_f));
        }


        // 获取文件保存路径
        QString CompressStorePathName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("Please select the file to store your compressed data"), defaultFileName, QString("FRL Files(.*)"));
        if (!CompressStorePathName.isEmpty())
        {
            // 将路径中的反斜杠加倍
            CompressStorePathName.replace("\\", "\\\\");

            ui.CompressFileStorePath->setPlainText(CompressStorePathName);
        }
        else
        {
            QMessageBox::warning(this, "Invalid Path", QString::fromLocal8Bit("未选择文件或路径无效."));
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
    //              TLA = std::round(Tolerance * 1000000.0) / 1000000.0; // 保留六位小数
             // }
             // if(60<compress_ratio<80) {
             // 	Tolerance = 3.395e-30 * pow(compress_ratio, 16.53);
    //              TLA = std::round(Tolerance * 1000000.0) / 1000000.0; // 保留六位小数
             // }
    //      }
    //
    //      // 在进行选择保存路径之前，先检查是否选择了压缩方法和压缩率
    //      if ((!isFRM && !isFLM)) {
    //          QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("请必须选择压缩方法"));
    //          return;
    //      }
    //
    //      // 构建默认文件名并加入行数和样本数
    //      QString defaultFileName = "compress";
    //      if (isFLM) {
    //          defaultFileName += QString(".flm_%1_%2_%3")
    //              .arg(QString::number(TLA * 1000000, 'f', 0))
    //              .arg(QString::number(get_rows_f))
    //              .arg(QString::number(Samples_f));
    //      }
    //
    //      // 获取文件保存路径
    //      QString CompressStorePathName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("Please select the file to store your compressed data"), defaultFileName, QString("FLM Files(.*)"));
    //      if (!CompressStorePathName.isEmpty())
    //      {
    //          // 将路径中的反斜杠加倍
    //          CompressStorePathName.replace("\\", "\\\\");
    //
    //          ui.CompressFileStorePath->setPlainText(CompressStorePathName);
    //      }
    //      else
    //      {
    //          QMessageBox::warning(this, "Invalid Path", QString::fromLocal8Bit("未选择文件或路径无效."));
    //      }
    //  }


}





////////////////////////3.点击”加载需要压缩的文件路径“按钮加载文件/////////////////////
void FALCHUAN::on_ReadCompressPathButton_ReadCompressPathFile_cliked()
{
    arma::fmat TraceData;
    if (globalCompressFilePathName->isEmpty()) {
        QMessageBox::warning(this, "Invalid Path", QString::fromLocal8Bit("文件路径无效或未选择文件."));
        return;
    }
    std::string CompressFilePathNameStr = globalCompressFilePathName->toStdString();

    QString allTraceCountStr = ui.AllTrace->text();
    QString samplesPerTraceStr = ui.Sample->text();

    // 检查 All_T 和 Samples 是否为空
    if (allTraceCountStr.isEmpty() || samplesPerTraceStr.isEmpty()) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("请必须输入此文件列数或所有行数"));
        return;
    }

    size_t All_T = allTraceCountStr.toULongLong();
    size_t Samples = samplesPerTraceStr.toULongLong();

    *globalSample() = Samples; //设置Samples为全局变量

    // 获取 t_start 和 t_end，如果没有输入则设置默认值
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

    *globalget_rows() = get_rows; //设置get_rows为全局变量

    ui.ReadCompressPathFile->setPlainText(QString::fromLocal8Bit("开始导入您的数据。\n ---------------------------------"));
    QCoreApplication::processEvents();  // 强制刷新UI


    //////////读取数据并计时，并将数据写入TraceData中////////////////////////
    auto read_start = std::chrono::high_resolution_clock::now();

    if (t_start > t_end || t_end > All_T || t_start < 0) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("请检查数组行范围选取是否正确"));
    }
    std::ifstream file(CompressFilePathNameStr, std::ios::binary);
    if (!file.is_open()) {
        QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("无法打开文件"));
    }

    size_t data_size = get_rows * Samples * sizeof(float);
    file.seekg(0, std::ios::end);

    size_t filesize;
    filesize = file.tellg();//获取文件大小

    *globalfilesize() = filesize; //设置filesize为全局变量

    //开始加载需要的数据
    file.seekg(t_start * Samples * sizeof(float), std::ios::beg);

    TraceData.set_size(Samples, get_rows);  // 先按列优先存储
    if (!file.read(reinterpret_cast<char*>(TraceData.memptr()), data_size)) {
        QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("无法读取文件"));
    }
    file.close();

    TraceData = TraceData.t();  // 转置矩阵，使其按行优先存储
    *globalTraceData() = TraceData; //设置TraceData为全局变量

    auto read_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> read_duration = read_end - read_start;

    ui.ReadCompressPathFile->append(QString::fromLocal8Bit("数据加载完成，用时：%1 秒。\n--------------------------------- ").arg(read_duration.count()));



    // 显示加载的数据，在 ReadCompressPathFile 这个 QTextBrowser 中显示
    QString displayText = QString::fromLocal8Bit("此文件大小为：%1 字节。\n您的数据已经导入完成。\n 任务完成。。。")
        .arg(filesize);
    ui.ReadCompressPathFile->append(displayText);
}



//////////////////4.从加载数据页面跳转到压缩参数页面//////////////////////////////////
void FALCHUAN::on_NextPage_page2_clicked() {
    QString allTraceCountStr = ui.AllTrace->text();
    QString samplesPerTraceStr = ui.Sample->text();

    // 检查 All_T 和 Samples 是否为空
    if (allTraceCountStr.isEmpty() || samplesPerTraceStr.isEmpty()) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("请必须输入此文件行或者列数"));
        return;
    }


    ui.stackedWidget->setCurrentIndex(2);
}


//////////////////6. 从主页面跳转到加载数据页面//////////////////////////////////
void FALCHUAN::on_COMPRESS_page1_clicked() {
    ui.stackedWidget->setCurrentIndex(1);
}

//////////////////7. 从加载数据页面跳转到主页面////////////////////////////////
void FALCHUAN::on_BackMain_page3_clicked() {
    ui.stackedWidget->setCurrentIndex(0);
}

///////////////////8.从压缩参数页面跳转到加载数据页面////////////////////////////
void FALCHUAN::on_BackCompress_page1_clicked() {
    ui.stackedWidget->setCurrentIndex(1);
}


///////////////////9.从主页面跳转到解压缩//////////////////////////////////////
void FALCHUAN::on_DECOMPRESS__page4_clicked() {
    ui.stackedWidget->setCurrentIndex(3);
}

///////////////////10.从解压缩页面跳转到主页面//////////////////////////////////
void FALCHUAN::on_BackfirstMain_page3_clicked() {
    ui.stackedWidget->setCurrentIndex(0);
}

///////////////////11.选择需要解压缩的文件路径//////////////////////////////////
void FALCHUAN::on_NeedDecompressPath_NeedDecompressPathFile_clicked() {
    QStringList filters;
    filters << "Geoeast inner files (*.frm* *.flm*)" // 匹配所有 .frm* 与.flm文件
        << "All Files (*)"; // 匹配所有文件

    QFileDialog dialog(this);
    dialog.setNameFilters(filters);
    dialog.setWindowTitle(QString::fromLocal8Bit("请选择您需要解压缩的文件"));
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec() == QDialog::Accepted) {
        QString NeedDecompressPathName = dialog.selectedFiles().first();
        if (!NeedDecompressPathName.isEmpty()) {
            // 将路径中的反斜杠加倍
            NeedDecompressPathName.replace("\\", "\\\\");
            ui.NeedDecompressPathFile->setPlainText(NeedDecompressPathName);
        }
    }
};


///////////////////12.选择解压后的文件存放文件夹////////////////////////////////
void FALCHUAN::on_DecompressStorePath_DecompressStorePathFile_clicked() {
    // 获取源文件路径和名称
    QString sourceFilePath = ui.NeedDecompressPathFile->toPlainText();
    if (sourceFilePath.isEmpty()) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("请先选择压缩源文件"));
        return;
    }

    //提取文件名（不带路径）
    QFileInfo fileInfo(sourceFilePath);
    //QString baseName = fileInfo.completeBaseName(); // 获取文件名（不带扩展名）
    QString baseName = "Decompressed";

    // 设置默认解压路径和文件名
    QString defaultDecompressPath = fileInfo.path() + "/" + baseName + ".bin";

    QString DecompressStorePathName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("Please select the folder where your decompressed files are stored"), defaultDecompressPath, "bin files (*.bin)");
    if (!DecompressStorePathName.isEmpty())
    {
        // 将路径中的反斜杠加倍
        DecompressStorePathName.replace("\\", "\\\\");

        ui.DecompressStorePathFile->setPlainText(DecompressStorePathName);
    }
};


//////////////////////////13.开始压缩按钮/////////////////////////////////////
void FALCHUAN::on_StartCompress_CompressBrowser_clicked()
{
    try {
        //获取CPU最大线程数
        int threads = omp_get_max_threads();


        //QString CompressStorePathName; // 已经存储的压缩文件存放路径
        if (globalTraceData->is_empty()) {
            QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("TraceData 未初始化"));
            return;
        }
        arma::fmat TraceData;
        TraceData = *globalTraceData();

        // 获取参数,并转换为std::string(压缩文件存储文件夹位置)
        std::string CompressStorePathNameStr = ui.CompressFileStorePath->toPlainText().toStdString();

        if (CompressStorePathNameStr.empty()) {
            QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("请先选择压缩文件存放路径"));
            return;
        }




        // 获取 FRM 和 FLM 复选框的状态
        ui.FRM->setChecked(true);
        bool isFRMChecked = ui.FRM->isChecked();
        //bool isFLMChecked = ui.FLM->isChecked();

  //       if (!isFLMChecked and !isFRMChecked ) {
        // 	QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("请必须选择一种压缩方法"));
        // 	return;
        // }

        

        if (isFRMChecked) {
            // 处理 FRM 压缩方法

             // 压缩参数，压缩率，压缩方法
            double compress_ratio = 90.6; // 使用默认值

            QString R = ui.Ratio->text();

            if (!R.isEmpty()) {
                compress_ratio = R.toDouble(); // 使用用户输入的值
            }


            /// bit_per_value=（-99.99380986+压缩率）/-3.1279171;
            double bpv_temp = (-99.9938 + compress_ratio) / -3.1279;
            double bpv = std::round(bpv_temp * 10.0) / 10.0; // 保留一位小数



            ui.CompressBrowser->setPlainText(QString::fromLocal8Bit("开始您的压缩。。。\n ---------------------------------"));
            QCoreApplication::processEvents(); // 强制刷新UI


            ////////////////////开始压缩////////////////////////////////////////
            auto compress_start = std::chrono::high_resolution_clock::now();
            zfp_type type = zfp_type_float;

            // 检查 TraceData 是否已初始化
            if (TraceData.is_empty()) {
                QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("TraceData 未初始化"));
                return;
            }


            zfp_field* field = zfp_field_2d(TraceData.memptr(), type, TraceData.n_cols, TraceData.n_rows);
            zfp_stream* zfp = zfp_stream_open(NULL);
            zfp_stream_set_rate(zfp, bpv, type, 2, 0);
            zfp_stream_set_execution(zfp, zfp_exec_omp);
            zfp_stream_set_omp_threads(zfp, threads - 2);

            size_t bufsize = zfp_stream_maximum_size(zfp, field);

            std::vector<char> buffer; // 定义缓冲区
            buffer.resize(bufsize);

            bitstream* stream = stream_open(buffer.data(), bufsize);
            zfp_stream_set_bit_stream(zfp, stream);
            zfp_stream_rewind(zfp);

            size_t compressed_size = zfp_compress(zfp, field);
            if (!compressed_size) {
                QMessageBox::warning(this, "Compression failed", QString::fromLocal8Bit("压缩失败，请重试！"));
                return;
            }
            size_t compressed_size_out;
            compressed_size_out = compressed_size; // 压缩后文件大小
            *globalcompressed_size_out() = compressed_size_out;


            zfp_field_free(field);
            zfp_stream_close(zfp);
            stream_close(stream);

            auto end_compress = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> compress_duration = end_compress - compress_start;
            /////////////////压缩率输出////////////////////////////////////////

            if (*globalfilesize() == 0) {
                QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("filesize 未初始化"));
                return;
            }

            size_t filesize = *globalfilesize();
            double Calculate_Rate = 100.0 * (1.0 - (double)compressed_size_out / (double)filesize);

            // ui.CompressBrowser->append(QString::fromLocal8Bit("Total filesizes in origin file is ：%1 byte \n the file compressed is %2 byte")
            //     .arg(filesize)
            // .arg(compressed_size_out));
            // QCoreApplication::processEvents(); // 强制刷新UI

            if (*globalget_rows() == 0) {
                QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("get_rows 未初始化"));
            }

            size_t get_rows = *globalget_rows();

            double per_trace_byte = (double)compressed_size_out / get_rows;

            ui.CompressBrowser->append(QString::fromLocal8Bit("\n本次压缩使用时间为：%1 秒。\n此次的压缩率为：%2 %\n开始将压缩数据写入文件。。。\n---------------------------------")
                .arg(compress_duration.count())
                .arg(Calculate_Rate));
            QCoreApplication::processEvents(); // 强制刷新UI

            //////////////将压缩数据写入文件//////////////////////////////////////
            auto write_start = std::chrono::high_resolution_clock::now();
            std::ofstream file(CompressStorePathNameStr, std::ios::binary);
            if (!file.is_open()) {
                QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("无法打开文件"));
            }
            else {
                file.write(buffer.data(), compressed_size_out);
                file.close();
            }

            auto write_end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> write_duration = write_end - write_start;

            ui.CompressBrowser->append(QString::fromLocal8Bit("压缩数据写入文件完成。\n此次数据写入花费时间为：%1 秒。\n压缩文件大小为：%2 字节。\n---------------------------------\n 任务完成。。。")
                .arg(write_duration.count())
                .arg(compressed_size_out));
        }








        // if (isFLMChecked) {
        //     // 处理 FLM 压缩方法
        //
        //     // ui.CompressBrowser->setPlainText(QString::fromLocal8Bit("算法还在开发中。。。\n --------------------------------- \n 任务结束。。。"));
        //
        //     /////compress_ratio = 54.64 * tolerance^0.142+8.68(0.001....1.1)
        //     ////tolerance =3.468e-21 * compress_ratio^11.45
        //
        //     /////compress_ratio =  -217.6 * tolerance^(-0.01902 )+ 278.5 (5....60)
        //     ///tolerance = 3.395e-30  * compress_ratio^16.53
        //      // 压缩参数，压缩率，压缩方法
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
        //             TLA = std::round(Tolerance * 1000000.0) / 1000000.0; // 保留六位小数
        //         }
        //         if (60 < compress_ratio < 80) {
        //             Tolerance = 3.395e-30 * pow(compress_ratio, 16.53);
        //             TLA = std::round(Tolerance * 1000000.0) / 1000000.0; // 保留六位小数
        //         }
        //     }
        //
        //
        //
        //     ui.CompressBrowser->setPlainText(QString::fromLocal8Bit("开始您的压缩。。。\n ---------------------------------"));
        //     QCoreApplication::processEvents(); // 强制刷新UI
        //
        //
        //     ////////////////////开始压缩////////////////////////////////////////
        //     auto compress_start = std::chrono::high_resolution_clock::now();
        //     zfp_type type = zfp_type_float;
        //
        //     // 检查 TraceData 是否已初始化
        //     if (TraceData.is_empty()) {
        //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("TraceData 未初始化"));
        //         return;
        //     }
        //
        //
        //     zfp_field* field = zfp_field_2d(TraceData.memptr(), type, TraceData.n_cols, TraceData.n_rows);
        //     zfp_stream* zfp = zfp_stream_open(NULL);
        //
        //
        //     // 设置为固定精度模式
        //     zfp_stream_set_accuracy(zfp, TLA);
        //
        //
        //     zfp_stream_set_execution(zfp, zfp_exec_omp);
        //     zfp_stream_set_omp_threads(zfp, threads - 2);
        //
        //     size_t bufsize = zfp_stream_maximum_size(zfp, field);
        //
        //     std::vector<char> buffer; // 定义缓冲区
        //     buffer.resize(bufsize);
        //
        //     bitstream* stream = stream_open(buffer.data(), bufsize);
        //     zfp_stream_set_bit_stream(zfp, stream);
        //     zfp_stream_rewind(zfp);
        //
        //     size_t compressed_size = zfp_compress(zfp, field);
        //     if (!compressed_size) {
        //         QMessageBox::warning(this, "Compression failed", QString::fromLocal8Bit("压缩失败，请重试！"));
        //         return;
        //     }
        //     size_t compressed_size_out;
        //     compressed_size_out = compressed_size; // 压缩后文件大小
        //     *globalcompressed_size_out() = compressed_size_out;
        //
        //
        //     zfp_field_free(field);
        //     zfp_stream_close(zfp);
        //     stream_close(stream);
        //
        //     auto end_compress = std::chrono::high_resolution_clock::now();
        //     std::chrono::duration<double> compress_duration = end_compress - compress_start;
        //     /////////////////压缩率输出////////////////////////////////////////
        //
        //     if (*globalfilesize() == 0) {
        //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("filesize 未初始化"));
        //         return;
        //     }
        //
        //     size_t filesize = *globalfilesize();
        //     double Calculate_Rate = 100.0 * (1.0 - (double)compressed_size_out / (double)filesize);
        //
        //     // ui.CompressBrowser->append(QString::fromLocal8Bit("Total filesizes in origin file is ：%1 byte \n the file compressed is %2 byte")
        //     //     .arg(filesize)
        //     // .arg(compressed_size_out));
        //     // QCoreApplication::processEvents(); // 强制刷新UI
        //
        //     if (*globalget_rows() == 0) {
        //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("get_rows 未初始化"));
        //     }
        //
        //     size_t get_rows = *globalget_rows();
        //
        //     double per_trace_byte = (double)compressed_size_out / get_rows;
        //
        //     ui.CompressBrowser->append(QString::fromLocal8Bit("您的压缩已经完成。\n压缩后每道地震道字节数相同，都为：%1 字节。\n本次压缩使用时间为：%2 秒。\n此次的压缩率为：%3 %\n开始将压缩数据写入文件。。。\n---------------------------------")
        //         .arg(per_trace_byte)
        //         .arg(compress_duration.count())
        //         .arg(Calculate_Rate));
        //     QCoreApplication::processEvents(); // 强制刷新UI
        //
        //     //////////////将压缩数据写入文件//////////////////////////////////////
        //     auto write_start = std::chrono::high_resolution_clock::now();
        //     std::ofstream file(CompressStorePathNameStr, std::ios::binary);
        //     if (!file.is_open()) {
        //         QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("无法打开文件"));
        //     }
        //     else {
        //         file.write(buffer.data(), compressed_size_out);
        //         file.close();
        //     }
        //
        //     auto write_end = std::chrono::high_resolution_clock::now();
        //     std::chrono::duration<double> write_duration = write_end - write_start;
        //
        //     ui.CompressBrowser->append(QString::fromLocal8Bit("压缩数据写入文件完成。\n此次数据写入花费时间为：%1 秒。\n压缩文件大小为：%2 字节。\n---------------------------------\n 任务完成。。。")
        //         .arg(write_duration.count())
        //         .arg(compressed_size_out));
        // }






    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("程序发生错误：") + e.what());
    }
    catch (...) {
        QMessageBox::critical(this, "Error", QString::fromLocal8Bit("程序发生未知错误。"));
    }
}







/////////////////////////////////////14.开始解压缩按钮///////////////////////
void FALCHUAN::on_StartDecompress_DecompressBrowser_clicked() {



    std::vector<char> Need_decompress;
    std::string NeedDecompressPathFileStr = ui.NeedDecompressPathFile->toPlainText().toStdString();
    //获取保存解压缩后的文件地址
    std::string DecompressStorePathFileStr = ui.DecompressStorePathFile->toPlainText().toStdString();




    if (NeedDecompressPathFileStr.empty()) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("请先选择需要解压缩的文件"));

    }

    if (DecompressStorePathFileStr.empty()) {
        QMessageBox::warning(this, "Input Error", QString::fromLocal8Bit("Please select the unzipped file storage folder first"));
    }

    ///////////////提取解压缩需要的参数////////////////////////////////////
    QString NeedDecompressPathName = ui.NeedDecompressPathFile->toPlainText();
    // 获取文件名
    QFileInfo fileInfo(NeedDecompressPathName);
    QString fileName = fileInfo.fileName(); // 获取完整文件名，包括后缀
    ////////////frm算法
    // 使用正则表达式提取方法标识符、数字部分及其行数和样本数
    QRegularExpression re("([a-zA-Z]+)_(\\d+)_(\\d+)_([0-9]+)");
    QRegularExpressionMatch match = re.match(fileName);

    QString letters = match.captured(1); // 如 "frm"
    QString bpvNumber = match.captured(2); // 比特率部分，固定两位数
    //QString torlance = match.captured(2); // 容差部分，固定6位数
    QString rows = match.captured(3); // 行数，位数可变
    QString samples = match.captured(4); // 样本数

    // 将bpvNumber转换为小数形式
    double numericValue = bpvNumber.toDouble() / 10.0; // 转换为小数形式

    //double torlanceValue = torlance.toDouble() / 1000000.0; // 转换为小数形式


    // 将rows，samples转换为size_t
    size_t get_rows_f = rows.toULongLong();
    size_t Samples_f = samples.toULongLong();

    // 需要解压缩的起始地震道和结束地震道，如果没有输入则设置默认值
    size_t DeCoStartTrace = 0;
    size_t DeCoEndTrace = get_rows_f - 1;

    QString DCST = ui.DeCompressStartTrace->text();
    QString DCET = ui.DeCompressEndTrace->text();

    if (!DCST.isEmpty()) {
        DeCoStartTrace = DCST.toULongLong() - 1; // 需要解压的起始地震道
    }

    if (!DCET.isEmpty()) {
        DeCoEndTrace = DCET.toULongLong() - 1; // 需要解压的结束地震道
    }


    //////////////读取需要解压缩的文件//////////////////////////////////////
    if (letters == "frm") {

        std::ifstream file(NeedDecompressPathFileStr, std::ios::binary | std::ios::ate);

        if (!file.is_open()) {
            QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("无法打开文件"));
        }

        std::streampos size = file.tellg();//获取解压缩文件总大小
        file.seekg(0, std::ios::beg);

        Need_decompress.resize(size);
        if (!file.read(Need_decompress.data(), size)) {
            QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("无法读取文件"));

        }
        file.close();

        ///////////////////////////////开始解压缩////////////////////////////
        ui.DecompressBrowser->setPlainText(QString::fromLocal8Bit("\n开始解压缩。。。。\n ---------------------------------"));
        QCoreApplication::processEvents();//强制刷新UI



        // if (*globalget_rows == 0) {
        //     QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("get_rows 未初始化"));
        // }
        //
        // if (*globalSample == 0) {
        //     QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("Samples 未初始化"));
        // }

        //size_t get_trace = *globalget_rows();
        //size_t Samples = *globalSample();
        arma::fmat decompressedData(get_rows_f, Samples_f);
        //解压缩后的数据

        auto decompress_start = std::chrono::high_resolution_clock::now();


        if (Need_decompress.empty()) {
            QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("Need_decompress需要解压的数据未初始化"));
        }

        zfp_type type = zfp_type_float;
        zfp_field* field = zfp_field_2d(decompressedData.memptr(), type, Samples_f, get_rows_f);

        if (!field) {
            QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("内部field未初始化"));
        }

        zfp_stream* zfp = zfp_stream_open(NULL);
        if (!zfp) {
            QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("内部zfp未初始化"));
            zfp_field_free(field);
        }

        //与压缩时设置相同参数
        zfp_stream_set_rate(zfp, numericValue, type, 2, 0);

        const char* dataPtr = reinterpret_cast<const char*>(Need_decompress.data());
        size_t bufferByteSize = Need_decompress.size();

        bitstream* stream = stream_open((void*)dataPtr, bufferByteSize);

        if (!stream) {
            QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("内部stream未初始化"));
            zfp_field_free(field);
            zfp_stream_close(zfp);
        }

        zfp_stream_set_bit_stream(zfp, stream);
        zfp_stream_rewind(zfp);

        if (!zfp_decompress(zfp, field)) {
            QMessageBox::warning(this, "Decompression failed", QString::fromLocal8Bit("解压缩失败，请重试！"));

        }

        stream_close(stream);
        zfp_stream_close(zfp);
        zfp_field_free(field);

        auto decompress_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> decompress_duration = decompress_end - decompress_start;

        //解压缩后的数据（提取出特定连续行）
        arma::fmat LastDecompressData = decompressedData.rows(DeCoStartTrace, DeCoEndTrace);

        //计算decompressedData的大小
        size_t decom_filesize = (DeCoEndTrace - DeCoStartTrace + 1) * Samples_f * sizeof(float);

        ui.DecompressBrowser->append(QString::fromLocal8Bit("您的解压缩操作已经完成。\n 本次解压缩所用时间为：%1 秒。\n 现在开始将您解压缩后的数据写入磁盘。。。\n ---------------------------------")
            .arg(decompress_duration.count()));
        QCoreApplication::processEvents();//强制刷新UI

        //////写入磁盘
        // 将decompressedData转换为一维数组
        auto one_start = std::chrono::high_resolution_clock::now();
        // 对解压缩后的矩阵进行转置
        arma::fmat transposedData = LastDecompressData.t();
        // 将转置后的矩阵转换为一维数组（列优先）

        std::vector<float> oneDimensionalArray(transposedData.memptr(), transposedData.memptr() + transposedData.n_elem);


        std::ofstream outputFile(DecompressStorePathFileStr, std::ios::binary);
        outputFile.write(reinterpret_cast<const char*>(oneDimensionalArray.data()), oneDimensionalArray.size() * sizeof(float));
        outputFile.close();


        auto one_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> compress_duration = one_end - one_start;

        ui.DecompressBrowser->append(QString::fromLocal8Bit(" 解压缩数据写入文件完成。\n此次数据写入花费时间为：%1 秒。\n --------------------------------- \n 任务完成。。。。")
            .arg(compress_duration.count()));
    }

    // if(letters=="flm") {
       //  		// 处理 FLM 压缩方法
    //
    //     std::ifstream file(NeedDecompressPathFileStr, std::ios::binary | std::ios::ate);
    //
    //     if (!file.is_open()) {
    //         QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("无法打开文件"));
    //     }
    //
    //     std::streampos size = file.tellg();//获取解压缩文件总大小
    //     file.seekg(0, std::ios::beg);
    //
    //     Need_decompress.resize(size);
    //     if (!file.read(Need_decompress.data(), size)) {
    //         QMessageBox::warning(this, "File Error", QString::fromLocal8Bit("无法读取文件"));
    //
    //     }
    //     file.close();
    //
    //     ///////////////////////////////开始解压缩////////////////////////////
    //     ui.DecompressBrowser->setPlainText(QString::fromLocal8Bit("\n开始解压缩。。。\n您需要解压缩的文件大小为：%1 字节。\n 您需要解压缩出来的地震道为第 <%2> 道至第 <%3> 道一共 <%4> 个地震道。\n ---------------------------------")
    //         .arg(size)
    //         .arg(DeCoStartTrace + 1)
    //         .arg(DeCoEndTrace + 1)
    //         .arg(DeCoEndTrace - DeCoStartTrace + 1));
    //     QCoreApplication::processEvents();//强制刷新UI
    //
    //
    //
    //     // if (*globalget_rows == 0) {
    //     //     QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("get_rows 未初始化"));
    //     // }
    //     //
    //     // if (*globalSample == 0) {
    //     //     QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("Samples 未初始化"));
    //     // }
    //
    //     //size_t get_trace = *globalget_rows();
    //     //size_t Samples = *globalSample();
    //     arma::fmat decompressedData(get_rows_f, Samples_f);
    //     //解压缩后的数据
    //
    //     auto decompress_start = std::chrono::high_resolution_clock::now();
    //
    //
    //     if (Need_decompress.empty()) {
    //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("Need_decompress需要解压的数据未初始化"));
    //     }
    //
    //     zfp_type type = zfp_type_float;
    //     zfp_field* field = zfp_field_2d(decompressedData.memptr(), type, Samples_f, get_rows_f);
    //
    //     if (!field) {
    //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("内部field未初始化"));
    //     }
    //
    //     zfp_stream* zfp = zfp_stream_open(NULL);
    //     if (!zfp) {
    //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("内部zfp未初始化"));
    //         zfp_field_free(field);
    //     }
    //
    //     //与压缩时设置相同参数
    //     // 设置为固定精度模式
    //     zfp_stream_set_accuracy(zfp, torlanceValue);
    //
    //     const char* dataPtr = reinterpret_cast<const char*>(Need_decompress.data());
    //     size_t bufferByteSize = Need_decompress.size();
    //
    //     bitstream* stream = stream_open((void*)dataPtr, bufferByteSize);
    //
    //     if (!stream) {
    //         QMessageBox::warning(this, "Data Error", QString::fromLocal8Bit("内部stream未初始化"));
    //         zfp_field_free(field);
    //         zfp_stream_close(zfp);
    //     }
    //
    //     zfp_stream_set_bit_stream(zfp, stream);
    //     zfp_stream_rewind(zfp);
    //
    //     if (!zfp_decompress(zfp, field)) {
    //         QMessageBox::warning(this, "Decompression failed", QString::fromLocal8Bit("解压缩失败，请重试！"));
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
    //     //解压缩后的数据（提取出特定连续行）
    //     arma::fmat LastDecompressData = decompressedData.rows(DeCoStartTrace, DeCoEndTrace);
    //
    //     //计算decompressedData的大小
    //     size_t decom_filesize = (DeCoEndTrace - DeCoStartTrace + 1) * Samples_f * sizeof(float);
    //
    //     ui.DecompressBrowser->append(QString::fromLocal8Bit("您的解压缩操作已经完成。\n 本次解压缩所用时间为：%1 秒。\n 现在开始将您解压缩后的数据写入磁盘。。。\n ---------------------------------")
    //         .arg(decompress_duration.count()));
    //     QCoreApplication::processEvents();//强制刷新UI
    //
    //     //////写入磁盘
    //     // 将decompressedData转换为一维数组
    //     auto one_start = std::chrono::high_resolution_clock::now();
    //     // 对解压缩后的矩阵进行转置
    //     arma::fmat transposedData = LastDecompressData.t();
    //     // 将转置后的矩阵转换为一维数组（列优先）
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
    //     ui.DecompressBrowser->append(QString::fromLocal8Bit(" 解压缩数据写入文件完成。\n此次数据写入花费时间为：%1 秒。\n解压缩后的文件总大小为 %2 字节。\n --------------------------------- \n 任务完成。。。。")
    //         .arg(compress_duration.count())
    //         .arg(decom_filesize));
    //
    //
    // }

}





/////////////////////////15.从压缩页面退出/////////////////////////
void FALCHUAN::on_CMQuit_Quit_clicked()
{
    this->close();
}

///////////////////////16.从解压缩页面退出////////////////////////
void FALCHUAN::on_DEQuit_Quit_clicked()
{
    this->close();
}