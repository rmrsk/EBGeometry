import os
import subprocess
import re
from pathlib import Path

def get_changed_files(base_branch="main"):
    """Get list of changed .cpp, .hpp, .options, and .inputs files compared to base branch."""
    result = subprocess.run(
        ["git", "diff", "--name-only", base_branch],
        capture_output=True,
        text=True,
        check=True
    )
    files = [line.strip() for line in result.stdout.splitlines() if line.strip()]
    exts = (".cpp", ".hpp", ".options", ".inputs", ".md")
    return [str(Path(f).resolve()) for f in files if f.endswith(exts)]

def find_literalincludes(rst_dir, changed_files):
    """Search .rst files for literalinclude directives referencing changed files."""
    includes_found = []

    for rst_path in Path(rst_dir).rglob("*.rst"):
        with open(rst_path, encoding="utf-8") as f:
            for lineno, line in enumerate(f, start=1):
                match = re.match(r"\s*\.\.\s+literalinclude::\s+(.*)", line)
                if match:
                    included_file = match.group(1).strip()
                    # Resolve relative path from rst file location
                    included_path = (rst_path.parent / included_file).resolve()
                    if str(included_path) in changed_files:
                        includes_found.append(
                            (rst_path, lineno, included_file, included_path)
                        )
    return includes_found

if __name__ == "__main__":
    base_branch = "main"
    rst_dir = "Docs/Sphinx/source"

    changed_files = get_changed_files(base_branch)
    if not changed_files:
        print(f"No changed .cpp, .hpp, .options, .inputs, or .md files compared to {base_branch}")
        exit(0)

    includes = find_literalincludes(rst_dir, changed_files)

    if includes:
        print("Found literalincludes referencing changed files:\n")
        for rst_file, line, included, resolved in includes:
            print(f"{rst_file}:{line} -> includes '{included}' (resolved: {resolved})")
    else:
        print("No literalinclude directives reference changed files.")
