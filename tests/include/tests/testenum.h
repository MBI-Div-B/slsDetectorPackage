#pragma once
#include <cstdint>
enum class func_id{read_data, read_int, read_half_data, combined};
constexpr size_t MB = 1048576;
constexpr size_t DATA_SIZE = 50*MB;