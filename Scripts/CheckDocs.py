import re
import sys
from pathlib import Path

def find_literalincludes(rst_dir):
    """Search every .rst file under rst_dir for banned literalinclude directives."""
    includes_found = []

    for rst_path in Path(rst_dir).rglob("*.rst"):
        with open(rst_path, encoding="utf-8") as f:
            for lineno, line in enumerate(f, start=1):
                if re.match(r"\s*\.\.\s+literalinclude::\s+(.*)", line):
                    includes_found.append((rst_path, lineno, line.strip()))
    return includes_found

if __name__ == "__main__":
    rst_dir = "Docs/Sphinx/source"

    includes = find_literalincludes(rst_dir)

    if includes:
        print(f"literalinclude is banned in {rst_dir} -- replace with a Doxygen link instead:\n")
        for rst_file, line, text in includes:
            print(f"{rst_file}:{line}: {text}")
        sys.exit(1)

    print(f"No literalinclude directives found under {rst_dir}.")
