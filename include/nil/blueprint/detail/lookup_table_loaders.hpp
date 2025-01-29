//---------------------------------------------------------------------------//
// Copyright (c) 2023 Dmitrii Tabalin <d.tabalin@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#pragma once

#include <nil/blueprint/detail/lookup_table_precomputes.hpp>

namespace nil {
    namespace blueprint {
        namespace components {
            namespace detail {

                template<typename BluePrintFieldType, std::size_t N>
                void parse_lookup_table(
                    const std::array<std::array<uint32_t, 2>, N>& arr,
                    std::vector<std::vector<typename BluePrintFieldType::value_type>>& result
                ) {
                    using value_type = typename BluePrintFieldType::value_type;

                    result.resize(2);

                    for (auto& column: result) {
                        column.resize(N);
                    }

                    for (std::size_t i = 0; i < N; ++i) {
                        value_type first(arr[i][0]);
                        value_type second(arr[i][1]);

                        result[0][i] = std::move(first);
                        result[1][i] = std::move(second);
                    }
                }

                // This forcefully includes the table in the binary
                // It's not in binary form here because for current tables the ASCII form is actually smaller
                // due to the advanced compression stratgy of "we don't have to write leading zeroes"
                template <typename BlueprintFieldType>
                bool load_lookup_table_from_bin(
                    std::string table_name,
                    std::vector<std::vector<typename BlueprintFieldType::value_type>> &result) {

                    if (table_name == "8_split_4") {
                        parse_lookup_table<BlueprintFieldType>(table_8_split_4, result);
                        return true;
                    } else if (table_name == "8_split_7") {
                        parse_lookup_table<BlueprintFieldType>(table_8_split_7, result);
                        return true;
                    } else {
                        return false;
                    }
                }
            }   // namespace detail
        }       // namespace components
    }           // namespace blueprint
}    // namespace nil
