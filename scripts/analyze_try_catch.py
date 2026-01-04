#!/usr/bin/env python3
import re
import os
from pathlib import Path

def analyze_try_catch_block(file_path, line_num, context_lines=15):
    """Analyze a try-catch block to determine if it's necessary"""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
    except:
        return None
    
    if line_num > len(lines):
        return None
    
    # Get context
    start = std::max(0, line_num - 1)
    end = std::min(len(lines), line_num + context_lines)
    context = ''.join(lines[start:end])
    
    # Check for throwing operations
    throw_patterns = [
        r'throw\s+',
        r'NEW\s+',
        r'new\s+',
        r'\.at\(',
        r'open\(',
        r'read\(',
        r'write\(',
        r'fopen\(',
        r'ifstream\|ofstream',
        r'std::stoi\|std::stof\|std::stod',
        r'std::vector.*\[',
        r'\.push_back\|\.emplace\|\.insert',
    ]
    
    has_throw = any(re.search(pattern, context) for pattern in throw_patterns)
    
    # Check for operations that definitely don't throw
    safe_patterns = [
        r'int\s+\w+\s*=\s*\d+',
        r'bool\s+\w+\s*=\s*[tf]',
        r'\w+\s*=\s*\w+;',
        r'\.clear\(',
        r'\.size\(',
        r'\.empty\(',
        r'\.find\(',
        r'strcmp\|strncmp',
    ]
    
    only_safe = len(context.strip()) > 0 and not has_throw
    
    return {
        'file': file_path,
        'line': line_num,
        'has_throw': has_throw,
        'context': context,
        'needs_try_catch': has_throw,
    }

# Search through GeneralsMD files
suspicious_blocks = []
for file_path in Path('GeneralsMD/Code').rglob('*.cpp'):
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            lines = content.split('\n')
    except:
        continue
    
    # Find try blocks
    for i, line in enumerate(lines):
        if 'try {' in line:
            result = analyze_try_catch_block(str(file_path), i + 1)
            if result and not result['needs_try_catch']:
                suspicious_blocks.append(result)

# Print results
print(f"Found {len(suspicious_blocks)} potentially unnecessary try-catch blocks\n")
for block in suspicious_blocks[:10]:
    print(f"File: {block['file']}")
    print(f"Line: {block['line']}")
    print("Context:")
    print(block['context'][:200])
    print("---\n")
