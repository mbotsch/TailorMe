//======================================================================================================================
// Copyright (c) by Fabian Kemper 2024
//
// This work is licensed under a
// Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// You should have received a copy of the license along with this
// work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
//
//======================================================================================================================

#include "utils/io/ndarray_io.h"

#include <iostream>
#include <fstream>
#include <filesystem>

using namespace Eigen;

namespace NDArray {

//======================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------

auto read_and_match_magic_bytes(std::istream& in_stream) -> void
{
    char magic_number[5] = { 0 };
    in_stream.read(magic_number, 4);
    if (std::strcmp(magic_number, std::string(MAGIC_BYTES_NDARRAY).c_str()) != 0) {
        in_stream.seekg(0);
        std::cerr << "FOUND " << magic_number << " as magic bytes.\n";
        throw std::runtime_error("NDArray read: Magic number mismatch.");
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto read_and_match_file_version(std::istream& in_stream) -> void
{
    char version { 0 };
    in_stream.read(&version, 1);
    if (version != FILE_VERSION_1) {
        throw std::runtime_error("NDArray read: File version does not match 1.");
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto read_ndim(std::istream& in_stream) -> int
{
    // dimensions of vector
    std::byte ndim;

    in_stream.read(reinterpret_cast<char *> (&ndim), sizeof(std::byte));
    return static_cast<int> (ndim);
}

// ---------------------------------------------------------------------------------------------------------------------

auto read_dtype(std::istream& in_stream) -> int
{
    std::byte dtype;

    in_stream.read(reinterpret_cast<char *> (&dtype), sizeof(std::byte));
    return static_cast<int> (dtype);
}

// ---------------------------------------------------------------------------------------------------------------------

auto read_dimensions(std::istream& in_stream, int ndim) -> std::vector<unsigned int>
{
    std::vector<unsigned int> dimensions {};
    // resize vector
    dimensions.resize(static_cast<long> (ndim));
    auto entry_count = static_cast<size_t>(ndim);
    // read dimension data
    in_stream.read(reinterpret_cast<char *> (dimensions.data()), static_cast<std::streamsize> (entry_count * sizeof(uint32_t)));

    return dimensions;
}

// ---------------------------------------------------------------------------------------------------------------------

auto read_vector_f(std::stringstream& buffer) -> Eigen::VectorXf
{
    // hard-casting feels wrong
    auto& in_stream = static_cast<std::istream&> (buffer);

    read_and_match_magic_bytes(in_stream);
    read_and_match_file_version(in_stream);
    int ndim = read_ndim(in_stream);
    if (ndim != 1) {
        throw std::runtime_error("NDArray read: Reading vector, dimension of file is not equal to 1.");
    }

    // read file format type
    int dtype = read_dtype(in_stream);
    if (dtype != DTYPE_FLOAT) {
        throw std::runtime_error("NDArray read: Reading float, dtype of file is not float.");
    }

    // move read head one byte (alignment)
    in_stream.seekg(1, std::ios_base::cur);

    std::vector<uint32_t> dimensions = read_dimensions(in_stream, ndim);

    VectorXf result {};
    result.resize(dimensions[0]);
    long entries = static_cast<long> (dimensions[0]);
    in_stream.read(reinterpret_cast<char *> (result.data()), static_cast<std::streamsize> (entries * sizeof(float)));

    return result;
}

// ---------------------------------------------------------------------------------------------------------------------

auto read_matrix_f(std::stringstream& buffer) -> Eigen::MatrixXf
{
    // hard-casting feels wrong
    auto& in_stream = static_cast<std::istream&> (buffer);

    read_and_match_magic_bytes(in_stream);
    read_and_match_file_version(in_stream);
    int ndim = read_ndim(in_stream);
    if (ndim != 2) {
        throw std::runtime_error("NDArray read: Reading vector, dimension of file is not equal to 2.");
    }

    // read file format type
    int dtype = read_dtype(in_stream);
    if (dtype != DTYPE_FLOAT) {
        throw std::runtime_error("NDArray read: Reading float, dtype of file is not float.");
    }

    // move read head one byte (alignment)
    in_stream.seekg(1, std::ios_base::cur);

    std::vector<uint32_t> dimensions = read_dimensions(in_stream, ndim);

    MatrixXf result {};
    result.resize(dimensions[0], dimensions[1]);
    long entries = static_cast<long> (dimensions[0]) * static_cast<long> (dimensions[1]);
    in_stream.read(reinterpret_cast<char *> (result.data()), static_cast<std::streamsize> (entries * sizeof(float)));

    return result;
}

// ---------------------------------------------------------------------------------------------------------------------

auto write_vector_f(std::stringstream& buffer, const Eigen::VectorXf& ndarray) -> void
{
    reset_buffer(buffer);
    auto& out_stream = static_cast<std::ostream&> (buffer);

    // write magic bytes
    out_stream.write(MAGIC_BYTES_NDARRAY, 4);
    // file version
    out_stream.put(FILE_VERSION_1);

    // write dimensions
    std::byte ndim { static_cast<std::byte> (VectorXf::NumDimensions) };
    out_stream.write((char *) &ndim, 1);

    // write dtype
    out_stream.put(DTYPE_FLOAT);
    out_stream.put('\x00'); // padding

    // write dimension sizes
    uint32_t rows = ndarray.rows();
    out_stream.write(reinterpret_cast<char *> (&rows), sizeof(rows));

    // write data
    long entries = ndarray.size();
    out_stream.write(reinterpret_cast<const char *> (ndarray.data()), static_cast<std::streamsize>(entries * sizeof(float)));
}

// ---------------------------------------------------------------------------------------------------------------------

auto write_matrix_f(std::stringstream& buffer, const Eigen::MatrixXf& ndarray) -> void
{
    reset_buffer(buffer);
    auto& out_stream = static_cast<std::ostream&> (buffer);

    // write magic bytes
    out_stream.write(MAGIC_BYTES_NDARRAY, 4);
    // file version
    out_stream.put(FILE_VERSION_1);

    // write dimensions
    std::byte ndim { static_cast<std::byte> (MatrixXf::NumDimensions) };
    out_stream.write((char *) &ndim, 1);

    // write dtype
    out_stream.put(DTYPE_FLOAT);
    out_stream.put('\x00'); // padding

    // write dimension sizes
    uint32_t rows = ndarray.rows();
    uint32_t cols = ndarray.cols();
    out_stream.write(reinterpret_cast<char *> (&rows), sizeof(rows));
    out_stream.write(reinterpret_cast<char *> (&cols), sizeof(cols));

    // write data
    long entries = ndarray.size();
    out_stream.write(reinterpret_cast<const char *> (ndarray.data()), static_cast<std::streamsize>(entries * sizeof(float)));
}

// ---------------------------------------------------------------------------------------------------------------------

auto write_tensor_f(std::stringstream& buffer, const Eigen::Tensor<float, 3>& ndarray) -> void
{
    reset_buffer(buffer);
    auto& out_stream = static_cast<std::ostream&> (buffer);

    // write magic bytes
    out_stream.write(MAGIC_BYTES_NDARRAY, 4);
    // file version
    out_stream.put(FILE_VERSION_1);

    // write dimensions
    std::byte ndim { static_cast<std::byte> (Eigen::Tensor<float, 3, 0, long>::NumDimensions) };
//    std::cout << "NDIM " << std::to_integer<int>(ndim) << std::endl;
//    out_stream.write(reinterpret_cast<char *> (&ndim), 1);
    out_stream.write((char *) &ndim, 1);

    // write dtype
    out_stream.put(DTYPE_FLOAT);
    // padding
    out_stream.put('\x00');

    // write dimension sizes
    for (int dimension = 0; dimension < Eigen::Tensor<float, 3, 0, long>::NumDimensions; ++dimension) {
        uint32_t dim_size = ndarray.dimension(dimension);
        out_stream.write(reinterpret_cast<char *> (&dim_size), sizeof(dim_size));
    }

    // write data
    long entries = ndarray.size();
    out_stream.write(reinterpret_cast<char const *> (ndarray.data()), static_cast<std::streamsize>(entries * sizeof(float)));
}

// ---------------------------------------------------------------------------------------------------------------------

auto write_buffer_to_file(std::stringstream& buffer, const std::string& filename) -> void
{
    try {
        //std::stringstream buffer;
        std::ofstream ofstream;
        ofstream.open(filename, std::ios::out | std::ios::binary);
        ofstream << buffer.rdbuf();
        // close and flush executed on scope close
    } catch (std::exception& error) {
        std::cerr << error.what() << '\n';
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto read_file_to_buffer(const std::string& filename, std::stringstream& buffer) -> void
{
    // according to this source
    // https://stackoverflow.com/a/138645/10203469
    // the stream is read byte-by-byte which is slow - the solution free's the stream too early, so use byte-by-byte version
    if (not exists(std::filesystem::path(filename))) {
        std::stringstream str_strm;
        str_strm << "Could not find " << filename;
        throw std::runtime_error(str_strm.str());
    }
    try {
        std::ifstream in_stream(filename);
        if (in_stream) {
            reset_buffer(buffer);
            buffer << in_stream.rdbuf();
            buffer.seekg(0, std::ios::beg); // to start
            // close & flush on scope close
            // in_stream.close();
        }
    } catch (std::exception& error) {
        std::cerr << error.what() << '\n';
    }
}

// ---------------------------------------------------------------------------------------------------------------------

auto reset_buffer(std::stringstream& buffer) -> void
{
    const static std::stringstream initial;
    buffer.str(std::string());
    buffer.clear();
    buffer.copyfmt(initial);
}

// ---------------------------------------------------------------------------------------------------------------------

auto open_vector_f(const std::string& filename) -> Eigen::VectorXf
{
    std::stringstream buffer;
    NDArray::read_file_to_buffer(filename, buffer);
    return NDArray::read_vector_f(buffer);
}

// ---------------------------------------------------------------------------------------------------------------------

auto open_matrix_f(const std::string& filename) -> Eigen::MatrixXf
{
    std::stringstream buffer;
    NDArray::read_file_to_buffer(filename, buffer);
    return NDArray::read_matrix_f(buffer);
}

// ---------------------------------------------------------------------------------------------------------------------

auto save_vector_f(const std::string& filename, const VectorXf& ndarray) -> void
{
    std::stringstream buffer;
    NDArray::write_vector_f(buffer, ndarray);
    NDArray::write_buffer_to_file(buffer, filename);
}

// ---------------------------------------------------------------------------------------------------------------------

auto save_matrix_f(const std::string& filename, const MatrixXf& ndarray) -> void
{
    std::stringstream buffer;
    NDArray::write_matrix_f(buffer, ndarray);
    NDArray::write_buffer_to_file(buffer, filename);
}

// ---------------------------------------------------------------------------------------------------------------------


// =====================================================================================================================

} // end namespace NDArray


