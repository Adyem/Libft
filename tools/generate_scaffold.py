#!/usr/bin/env python3
"""Generate module and test scaffolds that follow Libft conventions."""
import argparse
import sys
from pathlib import Path

MODULE_HELP = "Create paired header/source files for a new module class."
TEST_HELP = "Create a regression test skeleton that uses the FT_TEST harness."


def compute_include_guard(header_name: str) -> str:
    guard = header_name.upper()
    guard = guard.replace("/", "_")
    guard = guard.replace("-", "_")
    guard = guard.replace(".", "_")
    if not guard.endswith("_HPP"):
        guard = guard + "_HPP"
    return guard


def ensure_parent(path: Path) -> None:
    parent = path.parent
    if not parent.exists():
        parent.mkdir(parents=True, exist_ok=True)
    return None


def write_file(path: Path, content: str, force: bool) -> bool:
    if path.exists() and not force:
        return False
    ensure_parent(path)
    path.write_text(content)
    return True


def build_module_header(name: str) -> str:
    guard = compute_include_guard(name + ".hpp")
    return (
        "#ifndef "
        + guard
        + "\n# define "
        + guard
        + "\n\n#include \"../Errno/errno.hpp\"\n\nclass "
        + name
        + "\n{\n    private:\n        mutable int _error_code;\n\n        void set_error(int error_code) const;\n\n    public:\n        "
        + name
        + "();\n        ~"
        + name
        + "();\n\n        int get_error() const;\n        const char *get_error_str() const;\n};\n\n#endif\n"
    )


def build_module_source(name: str) -> str:
    return (
        "#include \""
        + name
        + ".hpp\"\n#include \"../Errno/errno.hpp\"\n\n"
        + name
        + "::"
        + name
        + "()\n    : _error_code(ER_SUCCESS)\n{\n    this->set_error(ER_SUCCESS);\n    return ;\n}\n\n"
        + name
        + "::~"
        + name
        + "()\n{\n    return ;\n}\n\nint "
        + name
        + "::get_error() const\n{\n    return (this->_error_code);\n}\n\nconst char *"
        + name
        + "::get_error_str() const\n{\n    return (ft_strerror(this->_error_code));\n}\n\nvoid "
        + name
        + "::set_error(int error_code) const\n{\n    this->_error_code = error_code;\n    ft_errno = error_code;\n    return ;\n}\n"
    )


def build_test_source(name: str, description: str) -> str:
    return (
        "#include \"../../System_utils/test_runner.hpp\"\n#include \"../../Errno/errno.hpp\"\n\nFT_TEST("
        + name
        + ", \""
        + description
        + "\")\n{\n    FT_ASSERT_EQ(ER_SUCCESS, ER_SUCCESS);\n    return (1);\n}\n"
    )


def handle_module(args: argparse.Namespace) -> int:
    header_name = args.name + ".hpp"
    source_name = args.name + ".cpp"
    directory = Path(args.directory)
    header_path = directory / header_name
    source_path = directory / source_name
    header_written = write_file(header_path, build_module_header(args.name), args.force)
    source_written = write_file(source_path, build_module_source(args.name), args.force)
    if not header_written or not source_written:
        sys.stderr.write("Skipped existing files. Use --force to overwrite.\n")
        return 1
    return 0


def handle_test(args: argparse.Namespace) -> int:
    directory = Path(args.directory)
    test_path = directory / ("test_" + args.name + ".cpp")
    written = write_file(test_path, build_test_source(args.test_name, args.description), args.force)
    if not written:
        sys.stderr.write("Skipped existing files. Use --force to overwrite.\n")
        return 1
    return 0


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate Libft scaffolds.")
    subparsers = parser.add_subparsers(dest="command")

    module_parser = subparsers.add_parser("module", help=MODULE_HELP)
    module_parser.add_argument("name", help="Snake_case class name for the scaffold.")
    module_parser.add_argument("--directory", default=".", help="Destination directory.")
    module_parser.add_argument("--force", action="store_true", help="Overwrite existing files.")

    test_parser = subparsers.add_parser("test", help=TEST_HELP)
    test_parser.add_argument("name", help="Short identifier appended to the test file name.")
    test_parser.add_argument("--test-name", default="test_placeholder_case", help="FT_TEST identifier.")
    test_parser.add_argument("--description", default="replace with description", help="FT_TEST description string.")
    test_parser.add_argument("--directory", default="Test/Test", help="Destination directory.")
    test_parser.add_argument("--force", action="store_true", help="Overwrite existing files.")

    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    if args.command == "module":
        return handle_module(args)
    if args.command == "test":
        return handle_test(args)
    sys.stderr.write("No command specified. Use --help for usage information.\n")
    return 1


if __name__ == "__main__":
    sys.exit(main())
