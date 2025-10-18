# Development environment setup

This guide walks through configuring the toolchain required to build Libft,
execute the regression suite, and contribute patches across Linux, macOS, and
Windows Subsystem for Linux (WSL).

## Common requirements

1. **Compiler toolchain** – Clang 14+ or GCC 11+ with C++17 support.
2. **Build tools** – `make`, CMake 3.20+ (for the optional templates), and
   `pkg-config`.
3. **Python 3.9+** – Required for documentation generators, scaffolding
   utilities, and the manifest tooling.
4. **Git** – For cloning the repository and installing pre-commit hooks.

Verify the compiler version:

```bash
c++ --version
```

## Linux

1. Install dependencies using your package manager. Example for Debian/Ubuntu:

   ```bash
   sudo apt update
   sudo apt install build-essential clang cmake pkg-config python3 python3-pip git
   ```

2. Clone the repository and initialise submodules if present:

   ```bash
   git clone https://example.com/Libft.git
   cd Libft
   git submodule update --init --recursive
   ```

3. Build the library and run the tests:

   ```bash
   make
   make -C Test
   ./Test/libft_tests
   ```

## macOS

1. Install the Xcode command line tools if they are not already available:

   ```bash
   xcode-select --install
   ```

2. Use Homebrew to provision dependencies:

   ```bash
   brew install llvm cmake pkg-config python git
   ```

3. Ensure Homebrew's LLVM is first on the `PATH` so the correct `clang++`
   version is used:

   ```bash
   export PATH="/usr/local/opt/llvm/bin:$PATH"
   export LDFLAGS="-L/usr/local/opt/llvm/lib"
   export CPPFLAGS="-I/usr/local/opt/llvm/include"
   ```

4. Build and test using the same commands as Linux.

## Windows Subsystem for Linux

1. Install WSL 2 with an Ubuntu distribution following Microsoft's
   documentation.
2. After entering the WSL shell, follow the Linux steps above to install
   dependencies and build the project.
3. If you need to run graphical tooling (e.g., Doxygen HTML output), install an
   X server on Windows such as X410 or VcXsrv.

## Optional tooling

- **Pre-commit hooks** – Install the formatting and static analysis hooks using
  the helper script:

  ```bash
  ./tools/install_pre_commit_hooks.sh
  ```

- **Documentation** – Generate the API reference when working on headers:

  ```bash
  python3 tools/run_doxygen.py
  ```

- **Code generation** – Use `tools/generate_scaffold.py` to create module and
  test skeletons that follow the project's conventions.

## Verifying the setup

After installing the dependencies, run the full regression suite before opening
pull requests:

```bash
make -C Test
./Test/libft_tests
```

All tests should report `OK`. Investigate any failures before submitting
changes.
