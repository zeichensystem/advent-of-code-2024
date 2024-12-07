# Advent of Code 2024
My C++20 solutions to the puzzles of [Advent of Code 2024](https://adventofcode.com/2024) (to learn myself some C++ and cmake). I'm using my small header-only library ([aoclib](aoclib/)) which I created for [last year's Advent of Code](https://github.com/zeichensystem/advent-of-code-2023). 

As the creator of Advent of Code does not allow/encourage sharing your own puzzle inputs, I only put example puzzle inputs into [input/](input/) publicly, so you can at least run those if you don't have your own puzzle inputs.

My answers are correct for my puzzle inputs (and for the example inputs), but in my experience it is definitely possible to come up with *incorrect* code which may produce the *correct* answers for your own puzzle input, but which might not work for all other possible/legal puzzle inputs. (I put the answers to my puzzle inputs and to the example inputs as comments at the very top of the source-files for each day.)

## How to build and run
You need a C++20 compiler and cmake.

### 1. Configure cmake and generate build systems
Navigate to the top level of this repository (the directory where [CMakeLists.txt](CMakeLists.txt) is located) and create your build directories, for instance: 

`mkdir -p build/Release build/Debug`

Then, run:

`cmake -DCMAKE_BUILD_TYPE=Release -S . -B build/Release`

`cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build/Debug`

to generate the release build system into your build directory `build/Release` (and the debug build system into your build directory `build/Debug`). (If you do not run the commands at the top level of this repository, you have to run them with `-S path-to-this-repo` instead of `-S .` like above.)

You can also set `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=` if you want to use a custom output directory for the compiled binaries; by default, they will be put into the `bin/` directory at the root of this repository.

### 2. Build
Run 

`cmake --build build/Release --target day-nn` 

to build the release-mode executable for `day-nn` (or run `cmake --build build/Release` to build the release-mode executables for all days, but this might take a while).  

The resulting executable will be called `day-nn` (or `day-nn_dbg` for debug builds). By default, it will be put into the `bin/` directory at the root of this repository (or into the directory you specified with `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=` in step 1).

(For example: Run `cmake --build build/Release --target day-03` to build the release-mode executable which solves Day 3. It will be put into `bin/day-03` by default. Or run `cmake --build build/Debug --target day-03` to build the debug-mode executable `bin/day-03_dbg` instead, assuming you generated the debug build system into 'build/Debug' in step 1.)

### 3. Run
Run `bin/day-nn input_day_nn.txt` to compute and print the solutions (part 1 and part 2) for *day-nn* (with `input_day_nn.txt` being your puzzle input for that day). If you don't have your own puzzle inputs, you can use the example inputs from the [input/](input/) directory.

```
Usage: day-nn [-help] puzzle_input [-v]
	-v: use verbose output (optional)
	-help: print this help, ignore the rest, and quit (optional)
	puzzle_input: your puzzle input file (optional/ignored if -help is used)
```

You can also build and run *day-nn* in one step with `cmake --build build/Release --target run-day-nn`, but this assumes you have saved your puzzle input for *day-nn* as `input/day-nn.txt` within this repository. 

If you don't have your own puzzle inputs but still want to test the executable for *day-nn*, you can build and run with `cmake --build build/Release --target run-day-nn-example` (which uses the puzzle's example input `input/day-nn-example.txt` automatically). 

For example: `cmake --build build/Release --target run-day-06-example` to build and run the release-mode executable for *day-06* with the example input [input/day-06-example.txt](input/day-06-example.txt)