print('Hello from V8Console\!');
console.log('Testing console.log');

const result = 2 + 2;
print('2 + 2 =', result);

// Test available functions
print('\nAvailable functions:');
const funcs = Object.keys(this).filter(k => typeof this[k] === 'function');
funcs.forEach(f => print('  -', f));

// Test DLL functions
print('\nDLL Management functions:');
print('  - loadDll:', typeof loadDll);
print('  - unloadDll:', typeof unloadDll);
print('  - reloadDll:', typeof reloadDll);
print('  - listDlls:', typeof listDlls);

// List current DLLs
const dlls = listDlls();
print('\nCurrently loaded DLLs:', dlls.length);

print('\nTest completed successfully\!');
EOF < /dev/null