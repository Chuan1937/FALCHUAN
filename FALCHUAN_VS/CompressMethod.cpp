#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <armadillo>
#include "zfp.h"
#include "omp.h"


#pragma comment(lib,"zfp.lib")
/////////////////////////////////Function Declaration/////////////////////////
////////////////////////////////compress_ratio function///////////////////////
int compress_ratio(const arma::fmat& TraceData, const double bits_per_value, std::vector<char>& buffer, size_t& compressed_size_out) {

    auto compress_start = std::chrono::high_resolution_clock::now();
    zfp_type type = zfp_type_float;
    zfp_field* field = zfp_field_2d(const_cast<float*>(TraceData.memptr()), type, TraceData.n_cols, TraceData.n_rows);

    zfp_stream* zfp = zfp_stream_open(NULL);
    zfp_stream_set_rate(zfp, bits_per_value, type, 2, 0);

    zfp_stream_set_execution(zfp, zfp_exec_omp);
    zfp_stream_set_omp_threads(zfp, 6);

    size_t bufsize = zfp_stream_maximum_size(zfp, field);
    buffer.resize(bufsize);

    bitstream* stream = stream_open(reinterpret_cast<uchar*>(buffer.data()), bufsize);
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    size_t compressed_size = zfp_compress(zfp, field);
    if (!compressed_size) {
        std::cerr << "Compression failed" << std::endl;
        zfp_field_free(field);
        zfp_stream_close(zfp);
        stream_close(stream);
        return -1;
    }

    compressed_size_out = compressed_size;

    zfp_field_free(field);
    zfp_stream_close(zfp);
    stream_close(stream);

    auto end_compress = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> compress_duration = end_compress - compress_start;
    std::cout << "Compression took " << compress_duration.count() << " seconds" << std::endl;

    return 0;
}


////////////////////////////////decompress_ratio function/////////////////////
int decompress_ratio(const std::vector<char>& buffer, size_t cols, size_t rows, const double bits_per_value, arma::fmat& decompressedData) {
    auto decompress_start = std::chrono::high_resolution_clock::now();

    if (buffer.empty()) {
        std::cerr << "Decompression failed: buffer empty" << std::endl;
        return -1;
    }

    zfp_type type = zfp_type_float;
    zfp_field* field = zfp_field_2d(decompressedData.memptr(), type, cols, rows);
    if (!field) {
        std::cerr << "Decompression failed: Unable to create field" << std::endl;
        return -1;
    }

    zfp_stream* zfp = zfp_stream_open(NULL);
    if (!zfp) {
        std::cerr << "Decompression failed: Cannot open zfp stream" << std::endl;
        zfp_field_free(field);
        return -1;
    }

    zfp_stream_set_rate(zfp, bits_per_value, type, 2, 0);

    const char* dataPtr = reinterpret_cast<const char*>(buffer.data());
    size_t bufferByteSize = buffer.size();

    bitstream* stream = stream_open((void*)dataPtr, bufferByteSize);
    if (!stream) {
        std::cerr << "Decompression failed: Unable to open bitstream" << std::endl;
        zfp_stream_close(zfp);
        zfp_field_free(field);
        return -1;
    }
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    if (!zfp_decompress(zfp, field)) {
        std::cerr << "Decompression failed" << std::endl;
    }
    stream_close(stream);
    zfp_stream_close(zfp);
    zfp_field_free(field);

    auto decompress_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> decompress_duration = decompress_end - decompress_start;
    std::cout << "Decompression took " << decompress_duration.count() << " seconds" << std::endl;

    return 0;
}



///////////////////////////////reversible function/////////////////////////////
int compress_reversible(const arma::fmat& TraceData, std::vector<char>& buffer, size_t& compressed_size_out) {
    auto compress_start = std::chrono::high_resolution_clock::now();

    // Initialize ZFP field for 2D data
    zfp_type type = zfp_type_float;
    zfp_field* field = zfp_field_2d(const_cast<float*>(TraceData.memptr()), type, TraceData.n_cols, TraceData.n_rows);

    // Initialize ZFP stream
    zfp_stream* zfp = zfp_stream_open(NULL);
    zfp_stream_set_reversible(zfp);

    // Allocate memory for the compressed stream
    size_t bufsize = zfp_stream_maximum_size(zfp, field);
    buffer.resize(bufsize);

    // Set up the bitstream
    bitstream* stream = stream_open(reinterpret_cast<uchar*>(buffer.data()), bufsize);
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    // Perform compression
    size_t compressed_size = zfp_compress(zfp, field);
    if (!compressed_size) {
        std::cerr << "Compression failed" << std::endl;
        zfp_field_free(field);
        zfp_stream_close(zfp);
        stream_close(stream);
        return -1;
    }

    compressed_size_out = compressed_size;

    // Clean up
    zfp_field_free(field);
    zfp_stream_close(zfp);
    stream_close(stream);
    auto end_compress = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> compress_duration = end_compress - compress_start;
    std::cout << "Compression took " << compress_duration.count() << " seconds" << std::endl;
    return 0;
}




///////////////////////////////reversible decompress function/////////////////////////////
int decompress_reversible(const std::vector<char>& buffer, size_t cols, size_t rows, arma::fmat& decompressedData) {
    auto compress_start = std::chrono::high_resolution_clock::now();
    if (buffer.empty()) {
        std::cerr << "Decompression failed: buffer is empty" << std::endl;
        return -1;
    }

    zfp_type type = zfp_type_float;
    zfp_field* field = zfp_field_2d(decompressedData.memptr(), type, cols, rows);
    if (!field) {
        std::cerr << "Decompression failed: unable to create field" << std::endl;
        return -1;
    }

    zfp_stream* zfp = zfp_stream_open(NULL);
    if (!zfp) {
        std::cerr << "Decompression failed: cannot open zfp stream" << std::endl;
        zfp_field_free(field);
        return -1;
    }

    zfp_stream_set_reversible(zfp);

    const char* dataPtr = reinterpret_cast<const char*>(buffer.data());
    size_t bufferByteSize = buffer.size();

    bitstream* stream = stream_open((void*)dataPtr, bufferByteSize);
    if (!stream) {
        std::cerr << "Decompression failed: Unable to open bitstream" << std::endl;
        zfp_stream_close(zfp);
        zfp_field_free(field);
        return -1;
    }
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    if (!zfp_decompress(zfp, field)) {
        std::cerr << "Decompression failed" << std::endl;
    }
    stream_close(stream);
    zfp_stream_close(zfp);
    zfp_field_free(field);
    auto end_compress = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> compress_duration = end_compress - compress_start;
    std::cout << "Compression took " << compress_duration.count() << " seconds" << std::endl;
    return 0;
}



/////////////////////////////////Function Declaration/////////////////////////
////////////////////////////////compress_accuracyfunction///////////////////////
int compress_accuracy(const arma::fmat& TraceData, const double accuracy, std::vector<char>& buffer, size_t& compressed_size_out) {

    auto compress_start = std::chrono::high_resolution_clock::now();
    zfp_type type = zfp_type_float;
    zfp_field* field = zfp_field_2d(const_cast<float*>(TraceData.memptr()), type, TraceData.n_cols, TraceData.n_rows);

    zfp_stream* zfp = zfp_stream_open(NULL);

    zfp_stream_accuracy(zfp);
    zfp_stream_set_accuracy(zfp, accuracy);

    zfp_stream_set_execution(zfp, zfp_exec_omp);
    zfp_stream_set_omp_threads(zfp, 8);


    size_t bufsize = zfp_stream_maximum_size(zfp, field);
    buffer.resize(bufsize);

    bitstream* stream = stream_open(reinterpret_cast<uchar*>(buffer.data()), bufsize);
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    size_t compressed_size = zfp_compress(zfp, field);
    if (!compressed_size) {
        std::cerr << "Compression failed" << std::endl;
        zfp_field_free(field);
        zfp_stream_close(zfp);
        stream_close(stream);
        return -1;
    }

    compressed_size_out = compressed_size;

    zfp_field_free(field);
    zfp_stream_close(zfp);
    stream_close(stream);

    auto end_compress = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> compress_duration = end_compress - compress_start;
    std::cout << "Compression took " << compress_duration.count() << " seconds" << std::endl;

    return 0;
}


////////////////////////////////decompress_accuracy  function/////////////////////
int decompress_accuracy(const std::vector<char>& buffer, size_t cols, size_t rows, const double accuracy, arma::fmat& decompressedData) {
    auto decompress_start = std::chrono::high_resolution_clock::now();

    if (buffer.empty()) {
        std::cerr << "Decompression failed: buffer empty" << std::endl;
        return -1;
    }

    zfp_type type = zfp_type_float;
    zfp_field* field = zfp_field_2d(decompressedData.memptr(), type, cols, rows);
    if (!field) {
        std::cerr << "Decompression failed: Unable to create field" << std::endl;
        return -1;
    }

    zfp_stream* zfp = zfp_stream_open(NULL);
    if (!zfp) {
        std::cerr << "Decompression failed: Cannot open zfp stream" << std::endl;
        zfp_field_free(field);
        return -1;
    }
    zfp_stream_accuracy(zfp);
    zfp_stream_set_accuracy(zfp, accuracy);


    const char* dataPtr = reinterpret_cast<const char*>(buffer.data());
    size_t bufferByteSize = buffer.size();

    bitstream* stream = stream_open((void*)dataPtr, bufferByteSize);
    if (!stream) {
        std::cerr << "Decompression failed: Unable to open bitstream" << std::endl;
        zfp_stream_close(zfp);
        zfp_field_free(field);
        return -1;
    }
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    if (!zfp_decompress(zfp, field)) {
        std::cerr << "Decompression failed" << std::endl;
    }
    stream_close(stream);
    zfp_stream_close(zfp);
    zfp_field_free(field);

    auto decompress_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> decompress_duration = decompress_end - decompress_start;
    std::cout << "Decompression took " << decompress_duration.count() << " seconds" << std::endl;

    return 0;
}



/////////////////////////////////Function Declaration/////////////////////////
////////////////////////////////compress_precision function///////////////////////
int compress_precision(const arma::fmat& TraceData, const double precision, std::vector<char>& buffer, size_t& compressed_size_out) {

    auto compress_start = std::chrono::high_resolution_clock::now();
    zfp_type type = zfp_type_float;
    zfp_field* field = zfp_field_2d(const_cast<float*>(TraceData.memptr()), type, TraceData.n_cols, TraceData.n_rows);

    zfp_stream* zfp = zfp_stream_open(NULL);

    zfp_stream_precision(zfp);
    zfp_stream_set_precision(zfp, precision);

    zfp_stream_set_execution(zfp, zfp_exec_omp);
    zfp_stream_set_omp_threads(zfp, 8);


    size_t bufsize = zfp_stream_maximum_size(zfp, field);
    buffer.resize(bufsize);

    bitstream* stream = stream_open(reinterpret_cast<uchar*>(buffer.data()), bufsize);
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    size_t compressed_size = zfp_compress(zfp, field);
    if (!compressed_size) {
        std::cerr << "Compression failed" << std::endl;
        zfp_field_free(field);
        zfp_stream_close(zfp);
        stream_close(stream);
        return -1;
    }

    compressed_size_out = compressed_size;

    zfp_field_free(field);
    zfp_stream_close(zfp);
    stream_close(stream);

    auto end_compress = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> compress_duration = end_compress - compress_start;
    std::cout << "Compression took " << compress_duration.count() << " seconds" << std::endl;

    return 0;
}


////////////////////////////////decompress_precision  function/////////////////////
int decompress_precision(const std::vector<char>& buffer, size_t cols, size_t rows, const double precision, arma::fmat& decompressedData) {
    auto decompress_start = std::chrono::high_resolution_clock::now();

    if (buffer.empty()) {
        std::cerr << "Decompression failed: buffer empty" << std::endl;
        return -1;
    }

    zfp_type type = zfp_type_float;
    zfp_field* field = zfp_field_2d(decompressedData.memptr(), type, cols, rows);
    if (!field) {
        std::cerr << "Decompression failed: Unable to create field" << std::endl;
        return -1;
    }

    zfp_stream* zfp = zfp_stream_open(NULL);
    if (!zfp) {
        std::cerr << "Decompression failed: Cannot open zfp stream" << std::endl;
        zfp_field_free(field);
        return -1;
    }
	zfp_stream_precision(zfp);
	zfp_stream_set_precision(zfp, precision);


    const char* dataPtr = reinterpret_cast<const char*>(buffer.data());
    size_t bufferByteSize = buffer.size();

    bitstream* stream = stream_open((void*)dataPtr, bufferByteSize);
    if (!stream) {
        std::cerr << "Decompression failed: Unable to open bitstream" << std::endl;
        zfp_stream_close(zfp);
        zfp_field_free(field);
        return -1;
    }
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    if (!zfp_decompress(zfp, field)) {
        std::cerr << "Decompression failed" << std::endl;
    }
    stream_close(stream);
    zfp_stream_close(zfp);
    zfp_field_free(field);

    auto decompress_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> decompress_duration = decompress_end - decompress_start;
    std::cout << "Decompression took " << decompress_duration.count() << " seconds" << std::endl;

    return 0;
}













////////////////////////////// file read//////////////////////////////
int part_read(const std::string& filePath, size_t All_T, size_t Samples, size_t t_start, size_t t_end, arma::fmat& TraceData) {

    auto read_start = std::chrono::high_resolution_clock::now();
    std::cout << "start read data.." << std::endl;
    if (t_start > t_end || t_end >= All_T || t_start < 0) {
        std::cerr << "Invalid t_start or t_end parameter" << std::endl;
        return -1;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filePath << std::endl;
        return -1;
    }

    size_t get_rows = t_end - t_start + 1;
    size_t data_size = get_rows * Samples * sizeof(float);

    file.seekg(t_start * Samples * sizeof(float), std::ios::beg);

    TraceData.set_size(Samples, get_rows);  // 先按列优先存储
    if (!file.read(reinterpret_cast<char*>(TraceData.memptr()), data_size)) {
        std::cerr << "Failed to read file: " << filePath << std::endl;
        file.close();
        return -1;
    }
    file.close();

    TraceData = TraceData.t();  // 转置矩阵，使其按行优先存储

    auto read_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> read_duration = read_end - read_start;
    std::cout << "part_read took " << read_duration.count() << " seconds" << std::endl;

    return 0;
}


///////////////////////////// Read the data that needs to be decompressed/////
bool readCompressedData(const std::string& filename, std::vector<char>& need_decompress) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return false;
    }

    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);

    need_decompress.resize(size);
    if (!file.read(need_decompress.data(), size)) {
        std::cerr << "Failed to read file: " << filename << std::endl;
        file.close();
        return false;
    }
    file.close();

    return true;
}

///////////////////////////////// 主函数 //////////////////////////////////////

//int main() {
//    const std::string path = "D:\\APP\\GeoEastData\\DATA\\data\\BGP33\\chuan\\Seismics\\PostData-for-training\\Volume.part000";
//
//    ////////////////////////////.part000 文件数据读取并重构矩阵 ////////////////////
//    size_t Samples = 2001;
//    size_t All_Trace = 210000;
//    size_t trace_start = 0;
//    size_t trace_end = 209999;
//    size_t get_trace = trace_end - trace_start + 1;
//    const double bpv = 3.0;
//    const double precision = 24;
//    const double tolerance = 1e-1;
//    arma::fmat TraceData;
//
//    // 读取part000文件数据
//    int read_result = part_read(path, All_Trace, Samples, trace_start, trace_end, TraceData);
//
//    if (read_result != 0) {
//        std::cerr << "Error reading part000 file" << std::endl;
//        return read_result;
//    }
//
//    std::cout << "Origin data at index 2001: " << TraceData(0, 2000) << std::endl;
//
//    /////////////////////////////// 压缩 ////////////////////////////////////////
//
//    size_t compressed_size_out;
//    std::vector<char> buffer;
//
//    //int compress = compress_ratio(TraceData, bpv, buffer, compressed_size_out);
//    int compress = compress_reversible(TraceData, buffer, compressed_size_out);
//
//
//    if (compress != 0) {
//        std::cerr << "Compression failed" << std::endl;
//        return -1;
//    }
//
//    std::ofstream compressedFile("Compressed.bin", std::ios::binary);
//    if (compressedFile.is_open()) {
//        compressedFile.write(reinterpret_cast<const char*>(buffer.data()), compressed_size_out);
//        compressedFile.close();
//    }
//    else {
//        std::cerr << "Failed to open file for writing." << std::endl;
//    }
//
//    ///////////////////////////// 压缩率 ////////////////////////////////////////
//
//    const std::string path_compressed = "Compressed.bin";
//    std::ifstream file_origin_size(path, std::ios::binary | std::ios::ate);
//    if (!file_origin_size.is_open()) {
//        std::cerr << "Unable to open file: " << path << std::endl;
//        return -1;
//    }
//    std::streampos origin_size = file_origin_size.tellg();
//    file_origin_size.close();
//
//    std::ifstream file_compress_size(path_compressed, std::ios::binary | std::ios::ate);
//    if (!file_compress_size.is_open()) {
//        std::cerr << "Unable to open file: " << path_compressed << std::endl;
//        return -1;
//    }
//    std::streampos compressed_size = file_compress_size.tellg();
//    file_compress_size.close();
//
//    if (compressed_size == 0) {
//        std::cerr << "Compressed file size is zero." << std::endl;
//        return -1;
//    }
//    double compress_ratio = 1 - (double)compressed_size / origin_size;
//    std::cout << "Compress Ratio: " << compress_ratio * 100 << "%" << std::endl;
//
//    ////////////////////////////// 解压缩 ///////////////////////////////////////
//
//    const std::string path_need_decompress = "Compressed.bin";
//    std::vector<char> Need_decompress;
//    if (!readCompressedData(path_need_decompress, Need_decompress)) {
//        std::cerr << "Failed to read compressed data" << std::endl;
//        return -1;
//    }
//
//    arma::fmat decompressedData(get_trace, Samples);
//    //int decompress = decompress_ratio(Need_decompress, Samples, get_trace, bpv, decompressedData);
//    int decompress = decompress_reversible(Need_decompress, Samples, get_trace, decompressedData);
//
//    if (decompress != 0) {
//        std::cerr << "Decompression failed" << std::endl;
//        return -1;
//    }
//
//    std::cout << "Decompressed data at index 2001: " << decompressedData(0, 2000) << std::endl;
//
//
//    auto one_start = std::chrono::high_resolution_clock::now();
//
//    // 将decompressedData转换为一维数组
//
//    // 对解压缩后的矩阵进行转置
//    arma::fmat transposedData = decompressedData.t();
//    // 将转置后的矩阵转换为一维数组（列优先）
//
//
//    std::vector<float> oneDimensionalArray(transposedData.memptr(), transposedData.memptr() + transposedData.n_elem);
//
//    auto one_end = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double> compress_duration = one_end - one_start;
//    std::cout << "trans_matrix_one took " << compress_duration.count() << " seconds" << std::endl;
//
//
//
//
//
//
//    // Output the resulting one-dimensional array to verify
//    std::cout << "One Dimensional Array: \n" << oneDimensionalArray[2000] << std::endl;
//
//    // std::cout << "start writing data to disk.." << std::endl;
//    //
//    // std::ofstream outputFile("Decompressed.part000", std::ios::binary);
//    // outputFile.write(reinterpret_cast<const char*>(oneDimensionalArray.data()), oneDimensionalArray.size() * sizeof(float));
//    // outputFile.close();
//    //
//    // std::cout << "Data written to disk" << std::endl;
//
//    return 0;
//}
