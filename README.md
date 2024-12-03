# Advent of Code 2024
My C++20 solutions to the puzzles of [Advent of Code 2024](https://adventofcode.com/2024) (to learn myself some C++ and cmake). I'm using my small header-only library ([aoclib](aoclib/)) which I created for [last year's Advent of Code](https://github.com/zeichensystem/advent-of-code-2023). 

As the creator of Advent of Code does not allow/encourage sharing your own puzzle inputs, I only put example puzzle inputs into [input/](input/) publicly, so you can at least run those if you don't have your own puzzle inputs.

My answers are correct for my puzzle inputs (and for the example inputs), but in my experience it is definitely possible to come up with *incorrect* code which may produce the *correct* answers for your own puzzle input, but which might not work for all other possible/legal puzzle inputs. (I put the answers to my puzzle inputs and to the example inputs as comments at the very top of the source-files for each day.)

## How to build and run
You need a C++20 compiler and cmake.

### 1. Configure cmake and generate build files
Navigate to the root of this repository and create a build directory, for instance: `mkdir -p build-release`. 

Navigate to your build directory (for instance: `cd build-release`) and run `cmake -DCMAKE_BUILD_TYPE=Release ..` to configure/generate the build system (use `-DCMAKE_BUILD_TYPE=Debug` to generate the build system for debug builds instead.)

(You can also set `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=` if you want to use a different output directory for the compiled binaries; by default, they will be put into the  `bin/` directory at the root of this repository). 

### 2. Build
In your build directory, run `cmake --build . --target day-nn` to build the executable for `day-nn` (or run `cmake --build .` to build all days, but this might take a while).  

The resulting executable will be called `day-nn` (or `day-nn_dbg` for debug builds). By default, it will be put into the  `bin/` directory at the root of this repository (or into the directory you specified with `-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=`.

For example: Within your build directory, run `cmake --build . --target day-03` to build the executable which solves Day 3. It will be put into `bin/day-03` (or `bin/day-03_dbg` for debug builds) by default.

### 3. Run
Run `bin/day-nn input_day_nn.txt` to compute and print the solutions (part 1 and part 2) for *day-nn* (with `input_day_nn.txt` being your puzzle input for that day). If you don't have your own puzzle inputs, you can use the example inputs from the [input/](input/) directory.

```
Usage: day-nn [-help] puzzle_input [-v]
	-v: use verbose output (optional)
	-help: print this help, ignore the rest, and quit (optional)
	puzzle_input: your puzzle input file (optional/ignored if -help is used)
```