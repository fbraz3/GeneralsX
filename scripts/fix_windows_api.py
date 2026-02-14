#!/usr/bin/env python3
"""
Fix Windows-specific API calls in W3DShaderManager.cpp for cross-platform compatibility
Applies fighter19's pattern:
1. __int64 → Int64
2. HeapAlloc/HeapFree → new[]/delete[]
3. QueryPerformanceFrequency/Counter → #ifdef _WIN32 blocks
"""

import re
import sys

def fix_windows_api(filepath):
    """Apply all Windows API compatibility fixes"""
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Fix 1: __int64 → Int64 (simple replacement)
    count1 = content.count('__int64 W3DShaderManager::m_driverVersion;')
    content = content.replace(
        '__int64 W3DShaderManager::m_driverVersion;',
        '// TheSuperHackers @bugfix BenderAI 13/02/2026 Int64 for cross-platform (fighter19 pattern)\nInt64 W3DShaderManager::m_driverVersion;'
    )
    
    # Fix 2: HeapAlloc → new DWORD[]
    # Pattern: (DWORD*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize)
    # Replacement: new DWORD[dwFileSize / sizeof(DWORD)]()
    old_heap_alloc = '''const DWORD* pShader = (DWORD*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize);
	if (!pShader)
	{
		OutputDebugString( "Failed to allocate memory to load shader\\n " );
		return E_FAIL;
	}'''
    
    new_heap_alloc = '''// TheSuperHackers @bugfix BenderAI 13/02/2026 Use new[] instead of HeapAlloc (fighter19 pattern)
	const DWORD* pShader = new DWORD[dwFileSize / sizeof(DWORD)]();
	if (!pShader)
	{
		OutputDebugString( "Failed to allocate memory to load shader\\n " );
		return E_FAIL;
	}'''
    
    count2 = content.count(old_heap_alloc)
    content = content.replace(old_heap_alloc, new_heap_alloc)
    
    # Fix 3: HeapFree → delete[]
    count3 = content.count('HeapFree(GetProcessHeap(), 0, (void*)pShader);')
    content = content.replace(
        'HeapFree(GetProcessHeap(), 0, (void*)pShader);',
        '// TheSuperHackers @bugfix BenderAI 13/02/2026 Use delete[] instead of HeapFree (fighter19 pattern)\n	delete[] pShader;'
    )
    
    # Fix 4: QueryPerformanceFrequency/Counter - opening block
    old_query_start = '''__int64 endTime64,freq64,startTime64;
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq64);
	QueryPerformanceCounter((LARGE_INTEGER *)&startTime64);'''
    
    new_query_start = '''// TheSuperHackers @bugfix BenderAI 13/02/2026 Int64 and cross-platform timing (fighter19 pattern)
  	Int64 endTime64,freq64,startTime64;
#ifdef _WIN32
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq64);
	QueryPerformanceCounter((LARGE_INTEGER *)&startTime64);
#else
	freq64 = 1000;
	startTime64 = 0;
#endif'''
    
    count4 = content.count(old_query_start)
    content = content.replace(old_query_start, new_query_start)
    
    # Fix 5: QueryPerformanceCounter - ending block
    old_query_end = '''	QueryPerformanceCounter((LARGE_INTEGER *)&endTime64);
	return ((double)(endTime64-startTime64)/(double)(freq64));'''
    
    new_query_end = '''// TheSuperHackers @bugfix BenderAI 13/02/2026 Cross-platform timing end (fighter19 pattern)
#ifdef _WIN32
	QueryPerformanceCounter((LARGE_INTEGER *)&endTime64);
#else
	endTime64 = 1000;
#endif
	return ((double)(endTime64-startTime64)/(double)(freq64));'''
    
    count5 = content.count(old_query_end)
    content = content.replace(old_query_end, new_query_end)
    
    # Write back
    with open(filepath, 'w') as f:
        f.write(content)
    
    # Report
    total = count1 + count2 + count3 + count4 + count5
    if count1 > 0:
        print(f"Fixed {count1} __int64 declarations")
    if count2 > 0:
        print(f"Fixed {count2} HeapAlloc calls")
    if count3 > 0:
        print(f"Fixed {count3} HeapFree calls")
    if count4 > 0:
        print(f"Fixed {count4} QueryPerformance start blocks")
    if count5 > 0:
        print(f"Fixed {count5} QueryPerformance end blocks")
    
    print(f"Total Windows API fixes: {total}")
    
    return total

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python3 fix_windows_api.py <filepath>")
        sys.exit(1)
    
    filepath = sys.argv[1]
    total_fixed = fix_windows_api(filepath)
    
    if total_fixed > 0:
        print(f"\nSuccessfully updated {filepath}")
    else:
        print(f"\nNo Windows API changes needed in {filepath}")
