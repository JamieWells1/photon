#!/usr/bin/env python3
"""
Add copyright notices to source files.
Only processes user's own source files, excludes third-party dependencies.
"""

import os
from pathlib import Path

COPYRIGHT_PYTHON = """# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

"""

COPYRIGHT_C = """/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

"""

# Directories to exclude (third-party, build artifacts, dependencies)
EXCLUDE_DIRS = {
    '.venv',
    'build',
    'tests/build',
    'pico-sdk',
    '_deps',
    'CMakeFiles',
    'examples',
    'test',
    'tests',
}

# Only include these directories for source files
INCLUDE_PATTERNS = [
    'src/**/*.c',
    'src/**/*.h',
    'main.c',
    'lwipopts.h',
    'proxy/*.py',
]

def should_exclude(filepath):
    """Check if file should be excluded based on path."""
    parts = Path(filepath).parts
    return any(exclude_dir in parts for exclude_dir in EXCLUDE_DIRS)

def has_copyright(content):
    """Check if file already has a copyright notice."""
    first_500_chars = content[:500].lower()
    return 'copyright' in first_500_chars

def add_copyright_to_file(filepath):
    """Add copyright notice to a file if it doesn't have one."""
    if should_exclude(filepath):
        return False

    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"⚠️  Could not read {filepath}: {e}")
        return False

    # Skip if already has copyright
    if has_copyright(content):
        print(f"⏭️  Skipping {filepath} (already has copyright)")
        return False

    # Determine copyright notice based on file extension
    ext = Path(filepath).suffix
    if ext == '.py':
        new_content = COPYRIGHT_PYTHON + content
    elif ext in ['.c', '.h']:
        new_content = COPYRIGHT_C + content
    else:
        return False

    # Write updated content
    try:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"✅ Added copyright to {filepath}")
        return True
    except Exception as e:
        print(f"❌ Could not write {filepath}: {e}")
        return False

def main():
    project_root = Path(__file__).parent
    os.chdir(project_root)

    files_to_process = []

    # Collect files matching include patterns
    for pattern in INCLUDE_PATTERNS:
        files_to_process.extend(project_root.glob(pattern))

    # Filter and process files
    modified_count = 0
    skipped_count = 0

    for filepath in sorted(files_to_process):
        if filepath.is_file():
            if add_copyright_to_file(filepath):
                modified_count += 1
            else:
                skipped_count += 1

    print(f"\n{'='*60}")
    print(f"✅ Modified: {modified_count} files")
    print(f"⏭️  Skipped: {skipped_count} files")
    print(f"{'='*60}")

if __name__ == '__main__':
    main()
