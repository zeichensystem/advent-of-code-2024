#pragma once
#include <thread>
#include <future>
#include <chrono>
#include <iostream>
#include <concepts>
#include <utility>

namespace aocutil 
{ 
constexpr int PARALLEL_MIN_THREADS_DEFAULT = 4;

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
}

// Case 1: The worker-lambda for the case where TransformOp operates on a single element.
template<class ForwardIt, class T, class ReduceOp, class TransformOp>
constexpr auto select_worker_fn(ForwardIt first, const auto& reduce, const auto& transform_elem, const auto& result_update) 
    requires std::invocable<TransformOp&, decltype(*first)> 
    // (If C++20 <concepts> are not available, remove the "requires" line above and add the function parameter "typename std::enable_if_t<std::is_invocable_v<TransformOp&, decltype(*first)>, int> = 0" to compile.)
{ 
     // Uncomment if c++20 <concepts> is not available. s
    return [&reduce, &transform_elem, &result_update](ForwardIt cbegin, ForwardIt cend) {
        if (cbegin == cend) {
            assert(false);
            return;
        }
        T sub_result = transform_elem(*cbegin);
        for (auto it = cbegin + 1; it != cend; ++it) {
            sub_result = reduce(sub_result, transform_elem(*it));
        }
        result_update(sub_result);
        // aocutil::threadsafe_log("worker processed " + std::to_string(cend - cbegin) + " elements (sub-result: " + std::to_string(sub_result) + ")\n");
    };
}

// Case 2: The worker-lambda for the case where TransformOp operates on a range. 
template<class ForwardIt, class T, class ReduceOp, class TransformOp>
constexpr auto select_worker_fn(ForwardIt first, const auto& reduce, const auto& transform_range, const auto& result_update) 
    requires std::invocable<TransformOp&, ForwardIt, ForwardIt>
    // (If C++20 <concepts> are not available, remove the "requires" line above and add the function parameter "typename std::enable_if_t<std::is_invocable_v<TransformOp&, ForwardIt, ForwardIt>, int> = 0" to compile.)
{ 
    return [&transform_range, &result_update](ForwardIt cbegin, ForwardIt cend) {
        if (cbegin == cend) {
            assert(false);
            return;
        }
        T sub_result = transform_range(cbegin, cend);
        result_update(sub_result);
        // aocutil::threadsafe_log("worker processed " + std::to_string(cend - cbegin) + " elements (sub-result: " + std::to_string(sub_result) + ")\n");
    };
} 

namespace aocutil 
{
template<class ForwardIt, class T, class ReduceOp, class TransformOp>
T parallel_transform_reduce(ForwardIt first, ForwardIt last, T init, ReduceOp reduce, TransformOp transform, int num_threads = 0)
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

    const std::ptrdiff_t num_elems = std::distance(first, last);
    if (num_elems <= 0 || first == last) {
        return init;
    }

    T result{init};
    std::mutex result_lock;

    const auto result_update = [&result, &result_lock, &reduce](const T& sub_result) -> void {
        std::lock_guard guard{result_lock};
        result = reduce(result, sub_result);
    };

    const auto worker = select_worker_fn<ForwardIt, T, ReduceOp, TransformOp>(first, reduce, transform, result_update); // Select the correct worker lambda at compile time (concepts/SFINAE).

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

template<class ForwardIt, class T, class ReduceOp, class TransformOp>
T parallel_transform_reduce(int num_threads, ForwardIt first, ForwardIt last, T init, ReduceOp reduce, TransformOp transform) // Allows "int num_threads" to be the first parameter.
{
  return parallel_transform_reduce(first, last, init, reduce, transform, num_threads);
}

// template<class ForwardIt, class WorkFn, class WorkFnResultOptional>
// void parallel_search_first(ForwardIt begin, ForwardIt end, WorkFn work_fn, std::chrono::milliseconds busy_loop_sleep = 0)
// {
//     std::promise<void> done_promise;
//     std::shared_future<void> done_future = done_promise.get_future(); 

//     const auto worker = [done_future](ForwardIt begin, ForwardIt end, std::promise<WorkFnResultOptional> result_promise) 
//     {
//         for (auto it = begin; it != end; ++it) {
//             if (done_future.wait_for(std::chrono::nanoseconds{0}) == std::future_status::ready) {
//                 return;
//             }
//             WorkFnResultOptional result = work_fn(*begin);
//             if (result.has_value()) {
//                 result_promise.set_value(result);
//                 return;
//             }
//         }
//         result_promise.set_exception(WorkFnResultOptional{});
//     };

//     if (num_threads > num_elems) {
//         num_threads = num_elems;
//     }

//     const ptrdiff_t elems_per_thread = num_elems / num_threads;
//     ptrdiff_t remainder = num_elems % num_threads;
//     assert(elems_per_thread > 0 || remainder > 0);
//     assert((elems_per_thread * num_threads + remainder) == num_elems);
        
//     std::vector<std::thread> workers;
//     std::vector<std::future<WorkFnResultOptional>> worker_futures;
//     std::vector<std::future<WorkFnResultOptional>> worker_results;

//     for (auto start_it = first; start_it != last;) {
//         assert(std::ssize(workers) < num_threads);
//         ptrdiff_t increment = elems_per_thread;
//         if (remainder) {
//             ++increment;
//             --remainder;
//         }
//         assert(increment > 0);

//         const auto end_it = std::next(start_it, increment);

//         std::promise<WorkFnResultOptional> worker_promise; 
//         worker_futures.push_back(worker_promise.get_future());
//         assert(worker_futures.back().valid());
//         workers.push_back(std::thread(worker, start_it, end_it, std::move(worker_promise)));

//         start_it = end_it;
//     }
//     assert(std::ssize(workers) == num_threads);
//     assert(remainder == 0);
//     assert(worker_futures.size() == workers.size());

//     const auto is_first_result = [&worker_futures, &worker_results](size_t idx)
//     {
//         const auto& future = worker_futures.at(i);
//         bool is_first_res = true;
//         for (int j = 0; j < i; ++j) {
//             if (worker_futures.at(j).valid() || worker_results.at(j).has_value()) {
//                 is_first_res = false;
//                 break;
//             }
//         }
//         return is_first_res;
//     }

//     WorkFnResultOptional result = {};
//     for (;;) {
//         size_t not_found = 0;
//         for (size_t i = 0; i < worker_futures.size(); ++i) {
//             auto& wrk_future = worker_futures.at(i);
//             auto& wrk_result = worker_results.at(i);

//             const bool is_first_res = is_first_result(i);

//             const auto is_done = [is_first_res, &wrk_future, &result, &wrk_result, &not_found, &done_promise]() -> bool {
//                 if (!wrk_results.has_value() || !is_first_res) {
//                     ++not_found;
//                     return false;
//                 } else {
//                     result = wrk_result;
//                     done_promise.set_value(); // Signal to all workers we found a solution.
//                     return true;
//                 }
//             };

//             if (!future.valid() && is_done()) {
//                 assert(result.has_value());
//                 goto end;
//             } else if (future.valid() && future.wait_for(busy_loop_sleep) == std::future_status::ready) {
//                 wrk_result = future.get();
//                 assert(!future.valid());
//                 if (is_done()) {
//                     assert(result.has_value());
//                     goto end;
//                 }
//             }
//         }

//         if (not_found == worker_futures.size()) {
//             assert(!result.has_value());
//             goto end;
//         }
//     }

//     end:
//     for (auto& w: workers) {
//         w.join();
//     }
//     return result;
// }

}