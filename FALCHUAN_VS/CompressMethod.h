#ifndef COMPRESSMETHOD_H
#define COMPRESSMETHOD_H

#include <iostream>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <armadillo>
#include "zfp.h"
#pragma comment(lib,"zfp.lib")

int compress_accuracy(const arma::fmat& TraceData, const double accuracy, std::vector<char>& buffer, size_t& compressed_size_out);

int compress_precision(const arma::fmat& TraceData, const double precision, std::vector<char>& buffer, size_t& compressed_size_out);

int compress_ratio(const arma::fmat& TraceData, const double bits_per_value, std::vector<char>& buffer, size_t& compressed_size_out);

int compress_reversible(const arma::fmat& TraceData, std::vector<char>& buffer, size_t& compressed_size_out);

int decompress_accuracy(const std::vector<char>& buffer, size_t cols, size_t rows, const double accuracy, arma::fmat& decompressedData);

int decompress_precision(const std::vector<char>& buffer, size_t cols, size_t rows, const double precision, arma::fmat& decompressedData);

int decompress_ratio(const std::vector<char>& buffer, size_t cols, size_t rows, const double bits_per_value, arma::fmat& decompressedData);

int decompress_reversible(const std::vector<char>& buffer, size_t cols, size_t rows, arma::fmat& decompressedData);

int part_read(const std::string& filePath, size_t All_T, size_t Samples, size_t t_start, size_t t_end, arma::fmat& TraceData);

bool readCompressedData(const std::string& filename, std::vector<char>& need_decompress);

#endif // DATA_READER_H
#pragma once
