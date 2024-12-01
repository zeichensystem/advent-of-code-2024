#pragma once

#include <vector>
#include <limits>
#include <cassert>
#include <optional>
#include "vec.hpp"

namespace aocutil
{

template<typename ElemType>
class Grid; // Forward declaration.

// cf. on custom iterators: https://internalpointers.com/post/writing-custom-iterators-modern-cpp (last retrieved 2024-06-19)
template<typename ElemType, bool is_const>
struct GridColIterator 
{
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = int;
    using value_type        = ElemType;
    using pointer           = typename std::conditional_t<is_const, const ElemType*, ElemType*>;  
    using reference         = typename std::conditional_t<is_const, const ElemType&, ElemType&>;
    using parent_ptr_type   = typename std::conditional_t<is_const, const Grid<ElemType>*, Grid<ElemType>*>;

    GridColIterator(int column, int row, parent_ptr_type parent) : col(column), current_row(row), parent(parent) 
    {
        assert(parent);
        if (parent == nullptr || !parent->pos_on_grid(col, row)) {
            ptr = nullptr;
        } else {
            ptr = &parent->at(col, row);
        }
    };

    reference operator*() const {assert(ptr != nullptr); return *ptr; }
    pointer operator->() const {assert(ptr != nullptr); return ptr; }

    // Prefix:
    GridColIterator& operator++() 
    {
        assert(parent);
         if (current_row < parent->height() - 1 && current_row >= -1)  {
            ++current_row;
            ptr = &parent->at(col, current_row);
         } else {
            ++current_row;
            ptr = nullptr;
         }
         return *this; 
    }  

    GridColIterator& operator--() 
    {
        assert(parent);
         if (current_row > 0 && current_row <= parent->height())  {
            --current_row;
            ptr = &parent->at(col, current_row);
         } else {
            --current_row;
            ptr = nullptr;
         }
         return *this; 
    }  

    // Postfix: 
    GridColIterator operator++(int) { GridColIterator tmp = *this; ++(*this); return tmp;}
    GridColIterator operator--(int) { GridColIterator tmp = *this; --(*this); return tmp;}

    GridColIterator& operator+=(const difference_type& n) 
    {
        current_row += n;
        if (current_row < 0 || current_row >= parent->height()) {
            ptr = nullptr; 
        } else {
            ptr = &parent->at(col, current_row);
        }
        return *this;
    }

    GridColIterator& operator-=(const difference_type& n) {
        return *this += -n;
    }

    GridColIterator operator+(const difference_type& n) const
    {
        GridColIterator res = GridColIterator(col, current_row + n, parent);
        if (res.current_row < 0 || res.current_row >= parent->height()) {
            res.ptr = nullptr; 
        } else {
            res.ptr = &parent->at(col, res.current_row);
        }
        return res;
    }

    GridColIterator operator-(const difference_type& n) const {
        return *this + -n;
    }

    ElemType& operator[](difference_type n) const
    {
        int new_row = current_row - n;
        if (new_row < 0 || new_row >= parent->height()) {
            throw std::out_of_range("GridColIterator: subscript out of range");
        }
        return parent->at(col, new_row);
    }

    difference_type operator-(const GridColIterator& other) const 
    {
        assert(parent == other.parent);
        assert(col == other.col); 
        return current_row - other.current_row;
    }

    auto operator<=>(const GridColIterator& rhs) const
    {
        assert(col == rhs.col);
        return current_row - rhs.current_row;
    }

    friend GridColIterator operator+(const difference_type& n, const GridColIterator& iter) 
    {
        return iter + n;
    }

    friend GridColIterator operator-(const difference_type& n, const GridColIterator& iter)
    {
        return iter - n; 
    }

    friend bool operator==(const GridColIterator& a, const GridColIterator& b) 
    {
        assert(a.col == b.col && a.parent == b.parent); 
        bool eq = a.current_row == b.current_row; 
        assert(!(eq && (a.ptr != b.ptr)));
        return eq;
    };

    friend bool operator!=(const GridColIterator& a, const GridColIterator& b) 
    {
        assert(a.col == b.col && a.parent == b.parent);
        return a.current_row != b.current_row;
    };  

private:
    int col; 
    int current_row;
    parent_ptr_type parent = nullptr;
    pointer ptr; 
};


template<typename ElemType, bool is_const>
struct GridIterator 
{
private: 
    bool idx_on_grid(int idx) const {
        return idx >= 0 && idx < std::ssize(parent->data);
    }

public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = int;
    using value_type        = ElemType;
    using pointer           = typename std::conditional_t<is_const, const ElemType*, ElemType*>;  
    using reference         = typename std::conditional_t<is_const, const ElemType&, ElemType&>;
    using parent_ptr_type   = typename std::conditional_t<is_const, const Grid<ElemType>*, Grid<ElemType>*>;

    GridIterator(int column, int row, parent_ptr_type parent) :  parent(parent) 
    {
        assert(parent);
        current_idx = column + row * parent->width();
        if (parent == nullptr || !idx_on_grid(current_idx)) {
            ptr = nullptr;
            // current_idx = current_idx < 0 ? -1 : std::ssize(parent->data);
        } else {
            ptr = &parent->data.at(current_idx);
        }
    };

    reference operator*() const {if (!ptr) {std::cout << current_idx << "\n";} assert(ptr != nullptr); return *ptr; }
    pointer operator->() const {assert(ptr != nullptr); return ptr; }

    // Prefix:
    GridIterator& operator++() 
    {
        assert(parent);
        ++current_idx;

        if (idx_on_grid(current_idx)) {
            ptr = &parent->data.at(current_idx);
        } else {
            // current_idx = current_idx < 0 ? -1 : std::ssize(parent->data);
            ptr = nullptr;
        }
         return *this;
    }  

    GridIterator& operator--() 
    {
        assert(parent);
        --current_idx;

        if (idx_on_grid(current_idx)) {
            ptr = &parent->data.at(current_idx);
        } else {
            // current_idx = current_idx < 0 ? -1 : std::ssize(parent->data);
            ptr = nullptr;
        }
         return *this;
    }  

    // Postfix: 
    GridIterator operator++(int) { GridIterator tmp = *this; ++(*this); return tmp;}
    GridIterator operator--(int) { GridIterator tmp = *this; --(*this); return tmp;}

    GridIterator& operator+=(const difference_type& n) 
    {
        assert(parent);
        current_idx += n;

        if (idx_on_grid(current_idx)) {
            ptr = &parent->data.at(current_idx);
        } else {
            // current_idx = current_idx < 0 ? -1 : std::ssize(parent->data);
            ptr = nullptr;
        }
        return *this;
    }

    GridIterator& operator-=(const difference_type& n) {
        return *this += -n;
    }

    GridIterator operator+(const difference_type& n) const
    {
        assert(parent);
        int current_col = current_idx % parent->width();
        int current_row = current_idx / parent->width();
        GridIterator res = GridIterator(current_col, current_row, parent);
        res.current_idx = current_idx + n;;
    
        if (idx_on_grid(res.current_idx)) {
            res.ptr = &parent->data.at(res.current_idx);
        } else {
            // res.current_idx = res.current_idx < 0 ? -1 : std::ssize(parent->data);
            res.ptr = nullptr;
        }
 
        return res;
    }

    GridIterator operator-(const difference_type& n) const {
        return *this + -n;
    }

    ElemType& operator[](difference_type n) const
    {
        difference_type new_idx = current_idx + n;
        if (!idx_on_grid(new_idx)) {
            throw std::out_of_range("GridIterator: subscript out of range.");
        }
        return parent->data.at(new_idx);
    }

    difference_type operator-(const GridIterator& other) const 
    {
        assert(parent == other.parent); 
        return current_idx - other.current_idx; 
    }

    auto operator<=>(const GridIterator& rhs) const
    {
        assert(rhs.parent == parent); 
        return current_idx - rhs.current_idx; 
    }

    friend GridIterator operator+(const difference_type& n, const GridIterator& iter) 
    {
        return iter + n;
    }

    friend GridIterator operator-(const difference_type& n, const GridIterator& iter)
    {
        return iter - n;
    }

    friend bool operator==(const GridIterator& a, const GridIterator& b) 
    {
        assert(a.parent == b.parent); 
        bool eq = a.current_idx == b.current_idx; 
        assert(!(eq && a.ptr != b.ptr));
        return eq;
    };

    friend bool operator!=(const GridIterator& a, const GridIterator& b) 
    {
        assert(a.parent == b.parent); 
        bool neq = a.current_idx != b.current_idx;
        assert(!((a.ptr != nullptr) && neq && (a.ptr == b.ptr)));
        return neq;
    };  

private:
    int current_idx = 0;
    parent_ptr_type parent = nullptr;
    pointer ptr = nullptr; 
};


template<typename ElemType>
class Grid 
{
    std::vector<ElemType> data;
    int width_ = 0, height_ = 0; 

    using GridColIteratorMut = GridColIterator<ElemType, false>;
    using GridColIteratorConst = GridColIterator<ElemType, true>;
    using GridIteratorMut = GridIterator<ElemType, false>; 
    using GridIteratorConst = GridIterator<ElemType, true>;

    friend GridColIteratorMut;
    friend GridColIteratorConst;
    friend GridIteratorMut;
    friend GridIteratorConst;

    using RowType = std::conditional_t<std::is_same<ElemType, char>::value, std::string, std::vector<ElemType>>;

    int calc_idx(int x, int y) const {
        return x + y * width();
    }

    Vec2<int> idx_to_pos(int idx) const 
    {
        int y = idx / width(); 
        int x = idx % width();
        assert(x >= 0 && x < width());
        assert(y >= 0 && y < height());
        return Vec2<int>{.x = x, .y = y};
    }

public: 
    Grid() = default; 

    Grid(const std::vector<RowType>& rows) 
    {
        if (!rows.size()) {
            return;
        }
        width_ = rows.at(0).size();
        height_ = rows.size();
        for (const auto& row : rows) {
            if (std::ssize(row) != width_) {
                throw std::invalid_argument("Grid::Grid: Rows of different width");
            }
            for (const auto & elem : row) {
                data.push_back(elem);
            }
        }
    }

    void push_row(const RowType& row) 
    {
        if (width_ != 0 && !(std::ssize(row) == width_)) {
            throw  std::out_of_range("Grid add_row: row size does not match");
        } else {
            width_ = std::ssize(row);
        }
        for (const auto& elem : row) {
            data.push_back(elem);
        }
        ++height_;
        if (width_ == 0 || height_ == 0) {
            throw std::runtime_error("Grid: Tried to push empty row.");
        }
    }

    std::optional<ElemType> try_get(int x, int y) const 
    {
        if (!pos_on_grid(x, y)) {
            return {};
        }
        return data.at(calc_idx(x, y));
    }
    std::optional<ElemType> try_get(const Vec2<int>& pos) const {
        return try_get(pos.x, pos.y); 
    }

    ElemType get(int x, int y) const 
    {
        if (!pos_on_grid(x, y)) {
            throw std::out_of_range("Grid get: invalid position");
        }
        return data.at(calc_idx(x, y));
    }
    ElemType get(const Vec2<int>& pos) const {
        return get(pos.x, pos.y);
    }

    void set(int x, int y, ElemType e) 
    {
        if (!pos_on_grid(x, y)) {
            throw std::out_of_range("Grid set: invalid position");
        }
        data.at(calc_idx(x, y)) = e;
    }
    void set(const Vec2<int>& pos, ElemType e) {
        set(pos.x, pos.y, e);
    }

    bool try_set(int x, int y, ElemType e) 
    {
        if (!pos_on_grid(x, y)) {
            return false;
        }
        set(x, y);
        return true;
    }
    bool try_set(const Vec2<int>& pos, ElemType e) {
        return try_set(pos.x, pos.y);
    }

    ElemType& at(int x, int y) 
    {
        if (!pos_on_grid(x, y)) {
            throw std::out_of_range("Grid at: invalid position");
        }
        return data.at(calc_idx(x, y));
    }
    ElemType& at(const Vec2<int>& pos) {
        return at(pos.x, pos.y);
    }

    const ElemType& at(int x, int y) const
    {
        if (!pos_on_grid(x, y)) {
            throw std::out_of_range("Grid at: invalid position");
        }
        return data.at(calc_idx(x, y));
    }
    const ElemType& at(const Vec2<int>& pos) const {
        return at(pos.x, pos.y);
    }

    ElemType& operator[](const Vec2<int>& pos) 
    {
        assert(calc_idx(pos.x, pos.y) >= 0 && calc_idx(pos.x, pos.y) < std::ssize(data));
        return data[calc_idx(pos.x, pos.y)]; 
    }
    const ElemType& operator[](const Vec2<int>& pos) const 
    {
        assert(calc_idx(pos.x, pos.y) >= 0 && calc_idx(pos.x, pos.y) < std::ssize(data));
        return data[calc_idx(pos.x, pos.y)]; 
    }

    bool pos_on_grid(int x, int y) const {
        bool on_grid = x >= 0 && x < width_ && y >= 0 && y < height_; 
        assert(!(on_grid && (calc_idx(x, y) >= std::ssize(data) || calc_idx(x, y) < 0)));
        return on_grid;
    }
    bool pos_on_grid(const Vec2<int>& pos) const {
        return pos_on_grid(pos.x, pos.y);
    }

    int height() const {
        return height_;
    }
    int width() const {
        return width_;
    }

    GridIteratorMut begin() {
        return GridIteratorMut(0, 0, this);
    }
    GridIteratorMut end() {
        return GridIteratorMut(width(), height() - 1, this);
    }

    GridIteratorConst cbegin() const {
        return GridIteratorConst(0, 0, this);
    }
    GridIteratorConst cend() const {
        return GridIteratorConst(width(), height() - 1, this);
    }


    GridColIteratorMut begin_col(int col) {
        return GridColIteratorMut(col, 0, this);
    }

    GridColIteratorMut end_col(int col) {
        return GridColIteratorMut(col, height_, this);
    }

    GridColIteratorConst cbegin_col(int col) const {
        return GridColIteratorConst(col, 0, this);
    }

    GridColIteratorConst cend_col(int col) const {
        return GridColIteratorConst(col, height_, this);
    }


    GridIteratorMut begin_row(int row) {
        return GridIteratorMut(0, row, this);
    }

    GridIteratorMut end_row(int row) {
        return GridIteratorMut(0, row + 1, this);
    }

    GridIteratorConst cbegin_row(int row) const {
        return GridIteratorConst(0, row, this);
    }

    GridIteratorConst cend_row(int row) const {
        return GridIteratorConst(0, row + 1, this);
    }

    std::vector<Vec2<int>> find_elem_positions(const ElemType& elem) const
    {
        std::vector<Vec2<int>> positions;

        auto start_pos = cbegin(); 
        std::size_t i = 0; 
        while (true) {
            assert(i <= data.size());
            auto found_pos = std::find(start_pos, cend(), elem); 
            if (found_pos == cend()) {
                return positions;
            } else {
                int idx = found_pos - cbegin();
                assert(idx >= 0 && idx < std::ssize(data));
                positions.push_back(idx_to_pos(idx));
                start_pos = std::next(found_pos);
            }
            ++i; 
        }
        assert(false);
    }

    friend std::ostream& operator<<(std::ostream& os, const Grid<ElemType>& g) 
    {
        for (int row_n = 0; row_n < g.height(); ++row_n) {
            for (auto elem = g.cbegin_row(row_n); elem != g.cend_row(row_n); ++elem) {
                os << *elem; 
            }
            os << "\n";
        }
        return os;
    }
};

}