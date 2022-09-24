
# AQuery++ Database
## Introduction

AQuery++ Database is a cross-platform, In-Memory Column-Store Database that incorporates compiled query execution.

# Installation
## Requirements
1. Recent version of Linux, Windows or MacOS, with recent C++ compiler that has C++17 (1z) support. (however c++20 is recommended if available for heterogeneous lookup on unordered containers)
     - GCC: 9.0 or above (g++ 7.x, 8.x fail to handle fold-expressions due to a compiler bug)
     - Clang: 5.0 or above (Recommended)
     - MSVC: 2019 or later (2022 or above is recommended)

2. Monetdb for Hybrid Engine
   - On windows, the required libraries and headers are already included in the repo.
   - On Linux, see [Monetdb Easy Setup](https://www.monetdb.org/easy-setup/) for instructions.
   - On MacOS, Monetdb can be easily installed in homebrew `brew install monetdb`.

3. Python 3.6 or above and install required packages in requirements.txt by `python3 -m pip install -r requirements.txt` 

## DOCKER: 
   - Alternatively, you can also use docker to run AQuery.
   - Type `make docker` to build the docker image from scratch. 
   - For Arm-based Mac users, you would need to build and run the x86_64 docker image because MonetDB doesn't offer official binaries for arm64 Linux. 
## Usage
`python3 prompt.py` will launch the interactive command prompt. The server binary will be automatically rebuilt and started.
#### Commands:
- `<sql statement>`: parse AQuery statement
- `f <filename>`: parse all AQuery statements in file
- `dbg` start debugging session 
- `print`: printout parsed AQuery statements

- `xexec`: execute last parsed statement(s) with Hybrid Execution Engine. Hybrid Execution Engine decouples the query into two parts. The standard SQL (MonetDB dialect) part is executed by an Embedded version of Monetdb and everything else is executed by a post-process module which is generated by AQuery++ Compiler in C++ and then compiled and executed.
- `save <OPTIONAL: filename>`: save current code snippet. will use random filename if not specified.
- `exit`: quit the prompt
- `exec`: execute last parsed statement(s) with AQuery Execution Engine (Old). AQuery Execution Engine executes query by compiling it to C++ code and then executing it.
- `r`: run the last generated code snippet
### Example:
   `f moving_avg.a` <br>
   `xexec`

See ./tests/ for more examples. 

## Notes for arm64 macOS users
- In theory, AQuery++ could work on both native arm64 and x86_64 through Rosetta. But for maximum performance, running native is preferred. 
- However, they can't be mixed up, i.e. make sure every component, `python` binary, `C++ compiler`, `monetdb` library and system commandline utilities such as `uname` should have the same architecture. 
- Because I can't get access to an arm-based mac to fully test this setup, there might still be issues. Please open an issue if you encounter any problems.

## Architecture 
![Architecture](./docs/arch-hybrid.svg)

### AQuery Compiler
- The query is first processed by the AQuery Compiler which is composed of a frontend that parses the query into AST and a backend that generates target code that delivers the query.
- Front end of AQuery++ Compiler is built on top of [mo-sql-parsing](https://github.com/klahnakoski/mo-sql-parsing) with modifications to handle AQuery dialect and extension.
- Backend of AQuery++ Compiler generates target code dependent on the Execution Engine. It can either be the C++ code for AQuery Execution Engine or sql and C++ post-processor for Hybrid Engine or k9 for the k9 Engine.
### Execution Engines
- AQuery++ supports different execution engines thanks to the decoupled compiler structure.
- AQuery Execution Engine: executes queries by compiling the query plan to C++ code. Doesn't support joins and udf functions. 
- Hybrid Execution Engine: decouples the query into two parts. The sql-compliant part is executed by an Embedded version of Monetdb and everything else is executed by a post-process module which is generated by AQuery++ Compiler in C++ and then compiled and executed.
- K9 Execution Engine: (discontinued).
  
## Roadmap
- [x] SQL Parser -> AQuery Parser (Front End)
- [x] AQuery-C++ Compiler (Back End)
   -  [x] Schema and Data Model 
   -  [x] Data acquisition/output from/to csv file
- [ ] Execution Engine
   -  [x] Projections and single-group Aggregations 
   -  [x] Group by Aggregations
   -  [x] Filters
   -  [x] Order by
   -  [x] Assumption
   -  [x] Flatten
   -  [x] UDFs (Hybrid Engine only)
   -  [x] User Module
   -  [ ] Triggers 
   -  [x] Join (Hybrid Engine only)
   -  [ ] Subqueries 
- [x] Query Optimization
  - [x] Selection/Order by push-down
  - [x] Join Optimization (Only in Hybrid Engine)

## Known Issues:

- [ ] Interval based triggers
- [ ] Hot reloading server binary
- [x] Bug fixes: type deduction misaligned in Hybrid Engine
- [ ] Investigation: Using postproc only for q1 in Hybrid Engine (make is_special always on)
- [x] Limitation: putting ColRefs back to monetdb. (Comparison)
- [ ] C++ Meta-Programming: Eliminate template recursions as much as possible.
- [ ] Functionality: Basic helper functions in aquery 
- [ ] Improvement: More DDLs, e.g. drop table, update table, etc.
- [ ] Bug: Join-Aware Column management
- [ ] Bug: Order By after Group By
