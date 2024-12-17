#pragma once
#include "aoclib/vec.hpp"
#include <optional>
#include <array>
#include <numeric>
#include <iostream>

template<class T = double, int Rows = 2, int Cols = 3>
requires(Rows != 0 && Cols != 0)
struct Matrix 
{
    std::array<T, Rows * Cols> data; // Stored flatly in row-major order.
    static constexpr int rows = Rows;
    static constexpr int cols = Cols;

    Matrix(T init = 0)
    {
        for (int i = 0; i < rows * cols; ++i) {
            data.at(i) = init;
        }
    }

    template<class VecT>
    Matrix(aocutil::Vec2<VecT> a, aocutil::Vec2<VecT> b, aocutil::Vec2<VecT> c)
    requires(rows == 2 && cols == 3)
    {
        data = {
            T(a.x), T(b.x), T(c.x), 
            T(a.y), T(b.y), T(c.y)
        };
    }

    T& at(int row, int col)
    {
        return data.at(row * cols + col);
    }
    T at(int row, int col) const
    {
        return data.at(row * cols + col);
    }

    void swap_row(int row_i, int row_j)
    {
        for (int col_i = 0; col_i < cols; ++col_i) {
            std::swap(at(row_i, col_i), at(row_j, col_i));
        }
    }

    void add_to_row(int row_i, T add)
    {
        for (int col_i = 0; col_i < cols; ++col_i) {
            data.at(row_i, col_i) += add;
        }
    }

    void add_row_to_row(int dst_row_i, int src_row_i, double src_row_scale = 1)
    {
        for (int col_i = 0; col_i < cols; ++col_i) {
            at(dst_row_i, col_i) += at(src_row_i, col_i) * src_row_scale;
        }
    }

    void scale_row(int row_i, T mul)
    {
        for (int col_i = 0; col_i < cols; ++col_i) {
            data.at(row_i, col_i) *= mul;
        }
    }

    std::optional<int> first_nonzero_col_i(int row_i) const
    {
        for (int col_i = 0; col_i < cols; ++col_i) {
            if (!nearly_equal(at(row_i, col_i), 0)) {
                return col_i;
            }
        }
        return {};
    }

    bool is_row_echelon() const {
        for (int row_i = 0; row_i < rows - 1; ++row_i) {
            const auto col_i = first_nonzero_col_i(row_i); 
            const auto next_col_i = first_nonzero_col_i(row_i + 1); 
            if (col_i && next_col_i && (col_i.value() >= next_col_i.value())) {
                return false;
            } else if (!col_i && next_col_i) {
                return false;
            } 
        }
        return true;
    }

    /*
        cf. https://people.math.aau.dk/~ottosen/MMA2011/rralg.html (last-retrieved 2024-12-17)
            https://en.wikipedia.org/wiki/Row_echelon_form (last-retrieved 2024-12-17)
    */ 
    bool row_echelon(int row_i_start = 0)
    {
        if (row_i_start == rows - 1) { // End.
            assert(is_row_echelon());
            for (int row_i = 0; row_i < rows; ++row_i) {
                const T row_result = at(row_i, cols - 1);
                bool row_all_zero = true;
                for (int col_i = 0; col_i < cols - 1; ++col_i) {
                    if (!nearly_equal(at(row_i, col_i), 0)) {
                        row_all_zero = false;
                        break;
                    }
                }
                if (row_all_zero && !nearly_equal(row_result, 0)) { // No solution.
                    return false;
                }
            }
            return true;
        }

        int pivot_col_i, pivot_row_i;
        if (!find_leftmost_nonzero_col(row_i_start, &pivot_col_i, &pivot_row_i)) {
            return row_echelon(rows - 1); // Goto End.
        }

        assert(pivot_col_i >= row_i_start);
        if (pivot_row_i != row_i_start) {
            swap_row(row_i_start, pivot_row_i);
            pivot_row_i = row_i_start;
        }

        const T pivot_val = at(row_i_start, pivot_row_i); 
        assert(pivot_val != 0);

        for (int row_i = pivot_row_i + 1; row_i < rows; ++row_i) { // Add scaled pivot row to rows below to create zeros below pivot.
            if (T val = at(row_i, pivot_col_i); !nearly_equal(val, 0)) {
                // pivot_val * x = -val
                double scale = -val / (double)pivot_val; 
                add_row_to_row(row_i, row_i_start, scale);
                at(row_i, pivot_col_i) = 0; //
            }
        }

        return row_echelon(pivot_row_i + 1);
    }

    bool reduced_row_echelon()
    {
        const bool solvable = row_echelon();

        for (int row_i = rows - 1, i = 0; row_i >= 0; --row_i, ++i) {
            T& row_result = at(row_i, cols - 1); 
            const int leftmost_col_i = cols - 2 - i;
            for (int c = cols - 2; c > leftmost_col_i; --c) { // No iterations for i == 0
                row_result -= at(row_i, c) * at(row_i + i, cols - 1);
                at(row_i, c) = 0;
            }
            if (!nearly_equal(at(row_i, leftmost_col_i), 0)) {
                row_result /= at(row_i, leftmost_col_i);
                at(row_i, leftmost_col_i) = 1;
            } 
        }
        return solvable;
    }

    bool find_leftmost_nonzero_col(int row_i_start = 0, int *result_col = nullptr, int *result_row = nullptr)
    {
        int zero_col_i = 0; 
        int row_i = row_i_start;
        for (; row_i < rows; ++row_i) { // Find first nonzero-column
            if (!nearly_equal(at(row_i, zero_col_i), 0)) {
                if (result_col) {
                    *result_col = zero_col_i;
                }
                if (result_row) {
                    *result_row = row_i;
                }
                return true;
            }
            if (row_i == rows-1) {
                if (++zero_col_i < cols - 1) {
                    row_i = row_i_start;
                } else {
                    break;
                }
            }
        }
        return false;
    }

    static bool nearly_equal(double a, double b, double epsilon = 2.0e-6)
    {
        // cf. https://floating-point-gui.de/errors/comparison/ (last retrieved 2024-12-16)
        const double abs_a = std::abs(a);
        const double abs_b = std::abs(b);
        const double abs_diff = std::abs(a - b);
        constexpr double DBL_MIN_NORMAL = std::numeric_limits<double>::min();
        constexpr double DBL_MAX = std::numeric_limits<double>::max();

        if (a == b) {
            return true;
        } else if (a == 0 || b == 0 || (abs_a + abs_b < DBL_MIN_NORMAL)) { // Absolute error. 
            return abs_diff < (epsilon * DBL_MIN_NORMAL);
        } else { // Relative error.
            return abs_diff / std::min((abs_a + abs_b), DBL_MAX) < epsilon;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Matrix& m)
    {
        // os << std::setw(5) << std::setfill('0');
        for (int row_i = 0; row_i < m.rows; ++row_i) {
            for (int col_i = 0; col_i < m.cols; ++col_i) {
                os << m.at(row_i, col_i) << " ";
            }
            os << "\n";
        }
        return os;
    }
};


