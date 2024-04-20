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

#ifndef TAILORME_NDARRAYIO_H
#define TAILORME_NDARRAYIO_H

#include <Eigen/Eigen>
#include <unsupported/Eigen/CXX11/Tensor>

// =====================================================================================================================

#define MAGIC_BYTES_NDARRAY "NDAR"

#define FILE_VERSION_1 0x01

#define DTYPE_FLOAT 0x01

// =====================================================================================================================

namespace NDArray {

auto read_vector_f(std::stringstream& buffer) -> Eigen::VectorXf;
auto read_matrix_f(std::stringstream& buffer) -> Eigen::MatrixXf;

auto write_vector_f(std::stringstream& buffer, const Eigen::VectorXf& ndarray) -> void;
auto write_matrix_f(std::stringstream& buffer, const Eigen::MatrixXf& ndarray) -> void;
auto write_tensor_f(std::stringstream& buffer, const Eigen::Tensor<float, 3>& ndarray) -> void;

auto write_buffer_to_file(std::stringstream& buffer, const std::string& filename) -> void;
auto read_file_to_buffer(const std::string& filename, std::stringstream& buffer) -> void;

// all-in-one functions
auto open_vector_f(const std::string& filename) -> Eigen::VectorXf;
auto open_matrix_f(const std::string& filename) -> Eigen::MatrixXf;
auto save_vector_f(const std::string& filename, const Eigen::VectorXf& ndarray) -> void;
auto save_matrix_f(const std::string& filename, const Eigen::MatrixXf& ndarray) -> void;

auto reset_buffer(std::stringstream& buffer) -> void;


} // namespace NDArray

// =====================================================================================================================

#endif // TAILORME_NDARRAYIO_H

