#!/usr/bin/env python3
from pathlib import Path
import sys

def load_manifest(manifest_path):
    content = manifest_path.read_text(encoding='utf-8')
    lines = content.splitlines()
    entries = []
    index = 0
    length = len(lines)
    while index < length:
        line = lines[index].strip()
        if line != '' and not line.startswith('#'):
            entries.append(line)
        index += 1
    unique_entries = sorted(set(entries))
    return unique_entries

def build_header(includes):
    header_lines = []
    header_lines.append('#ifndef FULL_LIBFT_HPP\n')
    header_lines.append('# define FULL_LIBFT_HPP\n\n')
    index = 0
    count = len(includes)
    while index < count:
        header_lines.append('#include "{}"\n'.format(includes[index]))
        index += 1
    header_lines.append('\n#endif\n')
    return ''.join(header_lines)

def main():
    script_path = Path(__file__).resolve()
    base_directory = script_path.parents[1]
    manifest_path = base_directory / 'full_libft_manifest.txt'
    header_path = base_directory / 'FullLibft.hpp'
    includes = load_manifest(manifest_path)
    header_text = build_header(includes)
    if len(sys.argv) > 1 and sys.argv[1] == '--check':
        if not header_path.exists():
            sys.stderr.write('FullLibft.hpp is missing.\n')
            return 1
        existing_text = header_path.read_text(encoding='utf-8')
        if existing_text != header_text:
            sys.stderr.write('FullLibft.hpp is out of date.\n')
            return 1
        return 0
    header_path.write_text(header_text, encoding='utf-8')
    return 0

if __name__ == '__main__':
    sys.exit(main())
