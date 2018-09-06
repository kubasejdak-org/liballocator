/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2018, Kuba Sejdak <kuba.sejdak@gmail.com>
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// 1. Redistributions of source code must retain the above copyright notice, this
///    list of conditions and the following disclaimer.
///
/// 2. Redistributions in binary form must reproduce the above copyright notice,
///    this list of conditions and the following disclaimer in the documentation
///    and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
/// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
/// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
/// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
/////////////////////////////////////////////////////////////////////////////////////

#ifndef UTILS_H
#define UTILS_H

#include <cstddef>

namespace Memory::utils {

/// @brief Returns the given value, that is rounded up to the closest power of 2.
/// @param[in] value        Value to be rounded.
/// @return Value rounded up to the closest power of 2.
inline std::size_t roundPower2(std::size_t value)
{
    auto result = value;

    --result;
    result |= (result >> 1);
    result |= (result >> 2);
    result |= (result >> 4);
    result |= (result >> 8);
    result |= (result >> 16);
    ++result;

    return result;
}

/// @brief Returns the given pointer moved by given number of bytes.
/// @param[in] ptr          Pointer to be moved.
/// @param[in] step         Number of bytes to move the pointer.
/// @return Value of the moved pointer.
template <typename T>
T* movePtr(T* ptr, std::size_t step)
{
    auto* tmp = reinterpret_cast<char*>(ptr);
    tmp += step;

    return reinterpret_cast<T*>(tmp);
}

} // namespace Memory::utils

#endif
