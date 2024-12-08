#pragma once
#include <thread>
#include <format>

namespace aocutil {

constexpr int PARALLEL_MIN_THREADS_DEFAULT = 2;

int get_num_threads_default()
{
    static const unsigned num_hardware_threads = std::thread::hardware_concurrency();
    if (num_hardware_threads >= PARALLEL_MIN_THREADS_DEFAULT && num_hardware_threads <= std::numeric_limits<int>::max()) {
        return num_hardware_threads;
    } else {
        return PARALLEL_MIN_THREADS_DEFAULT;
    }
}

void threadsafe_log(std::string_view msg)
{
    static std::mutex print_lock;
    std::lock_guard print_guard{print_lock};
    std::cout << msg;
}

template<class ForwardIt, class T, class BinaryOp, class UnaryOp>
T parallel_transform_reduce(ForwardIt first, ForwardIt last, T init, BinaryOp reduce, UnaryOp transform, int num_threads = 0)
{
    constexpr bool valid_iter_type = std::is_same<typename std::iterator_traits<ForwardIt>::iterator_category, std::contiguous_iterator_tag>::value    ||
                                     std::is_same<typename std::iterator_traits<ForwardIt>::iterator_category, std::random_access_iterator_tag>::value || 
                                     std::is_same<typename std::iterator_traits<ForwardIt>::iterator_category, std::bidirectional_iterator_tag>::value ||
                                     std::is_same<typename std::iterator_traits<ForwardIt>::iterator_category, std::forward_iterator_tag>::value;      
    constexpr bool iter_is_const = std::is_const<typename std::remove_pointer<typename std::iterator_traits<ForwardIt>::pointer>::type>::value; // cf. https://stackoverflow.com/a/10546077 (last retrieved 2024-12-07)
    
    static_assert(valid_iter_type); 
    static_assert(iter_is_const);

    if (num_threads < 0) {
        throw std::invalid_argument("parallel_transform_reduce: num_threads less than zero.");
    } else if (num_threads == 0) { // Use default.
        num_threads = get_num_threads_default();
        assert(num_threads > 0);
    }

    const std::ptrdiff_t num_elems = last - first;
    if (num_elems <= 0 || first == last) {
        return init;
    }

    T result{init};
    std::mutex result_lock;

    const auto result_update = [&result, &result_lock, &reduce](const T& sub_result) -> void {
        std::lock_guard guard{result_lock};
        result = reduce(result, sub_result);
    };

    const auto worker = [&reduce, &transform, &result_update](ForwardIt cbegin, ForwardIt cend) {
        if (cbegin == cend) {
            assert(false);
            return;
        }
        T sub_result = transform(*cbegin);
        for (auto it = cbegin + 1; it != cend; ++it) {
            sub_result = reduce(sub_result, transform(*it));
        }
        result_update(sub_result);
        // threadsafe_log("worker processed " + std::to_string(cend - cbegin) + " elements (sub-result: " + std::to_string(sub_result) + ")\n");
    };

    if (num_threads > num_elems) {
        num_threads = num_elems;
    }
    const ptrdiff_t elems_per_thread = num_elems / num_threads;
    ptrdiff_t remainder = num_elems % num_threads;
    assert(elems_per_thread > 0 || remainder > 0);
    assert((elems_per_thread * num_threads + remainder) == num_elems);
        
    std::vector<std::thread> workers;

    for (auto start_it = first; start_it != last;) {
        assert(std::ssize(workers) < num_threads);
        ptrdiff_t increment = elems_per_thread;
        if (remainder) {
            ++increment;
            --remainder;
        }
        assert(increment > 0);

        const auto end_it = std::next(start_it, increment);
        if (end_it == last) { // Run the last piece of work within this thread.
            worker(start_it, end_it);
            break;
        } else {
            workers.push_back(std::thread(worker, start_it, end_it));
        }
        start_it = end_it;
    }
    assert(std::ssize(workers) == (num_threads - 1));
    assert(remainder == 0);

    for (auto& w : workers) {
        w.join();
    }
    return result;
}

template<class ForwardIt, class T, class BinaryOp, class UnaryOp>
T parallel_transform_reduce(int num_threads, ForwardIt first, ForwardIt last, T init, BinaryOp reduce, UnaryOp transform)
{
  return parallel_transform_reduce(first, last, init, reduce, transform, num_threads);
}


// For ranged transform functions. 
template<class ForwardIt, class T, class BinaryOp, class RangeOp>
T parallel_ranged_transform_reduce(ForwardIt first, ForwardIt last, T init, BinaryOp reduce, RangeOp transform_range, int num_threads = 0)
{
    constexpr bool valid_iter_type = std::is_same<typename std::iterator_traits<ForwardIt>::iterator_category, std::contiguous_iterator_tag>::value    ||
                                     std::is_same<typename std::iterator_traits<ForwardIt>::iterator_category, std::random_access_iterator_tag>::value || 
                                     std::is_same<typename std::iterator_traits<ForwardIt>::iterator_category, std::bidirectional_iterator_tag>::value ||
                                     std::is_same<typename std::iterator_traits<ForwardIt>::iterator_category, std::forward_iterator_tag>::value;      
    constexpr bool iter_is_const = std::is_const<typename std::remove_pointer<typename std::iterator_traits<ForwardIt>::pointer>::type>::value; // cf. https://stackoverflow.com/a/10546077 (last retrieved 2024-12-07)
    
    static_assert(valid_iter_type); 
    static_assert(iter_is_const);

    if (num_threads < 0) {
        throw std::invalid_argument("parallel_transform_reduce: num_threads less than zero.");
    } else if (num_threads == 0) { // Use default.
        num_threads = get_num_threads_default();
        assert(num_threads > 0);
    }

    const std::ptrdiff_t num_elems = last - first;
    if (num_elems <= 0 || first == last) {
        return init;
    }

    T result{init};
    std::mutex result_lock;

    const auto result_update = [&result, &result_lock, &reduce](const T& sub_result) -> void {
        std::lock_guard guard{result_lock};
        result = reduce(result, sub_result);
    };

    const auto worker = [&transform_range, &result_update](ForwardIt cbegin, ForwardIt cend) {
        if (cbegin == cend) {
            assert(false);
            return;
        }
        T sub_result = transform_range(cbegin, cend);
        result_update(sub_result);
    };

    if (num_threads > num_elems) {
        num_threads = num_elems;
    }
    const ptrdiff_t elems_per_thread = num_elems / num_threads;
    ptrdiff_t remainder = num_elems % num_threads;
    assert(elems_per_thread > 0 || remainder > 0);
    assert((elems_per_thread * num_threads + remainder) == num_elems);
        
    std::vector<std::thread> workers;

    for (auto start_it = first; start_it != last;) {
        assert(std::ssize(workers) < num_threads);
        ptrdiff_t increment = elems_per_thread;
        if (remainder) {
            ++increment;
            --remainder;
        }
        assert(increment > 0);

        const auto end_it = std::next(start_it, increment);
        if (end_it == last) { // Run the last piece of work within this thread.
            worker(start_it, end_it);
            break;
        } else {
            workers.push_back(std::thread(worker, start_it, end_it));
        }
        start_it = end_it;
    }
    assert(std::ssize(workers) == (num_threads - 1));
    assert(remainder == 0);

    for (auto& w : workers) {
        w.join();
    }
    return result;
}

template<class ForwardIt, class T, class BinaryOp, class RangeOp>
T parallel_ranged_transform_reduce(int num_threads, ForwardIt first, ForwardIt last, T init, BinaryOp reduce, RangeOp transform_range)
{
    return parallel_ranged_transform_reduce(first, last, init, reduce, transform_range, num_threads);
}

}