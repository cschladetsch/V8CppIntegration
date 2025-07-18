print('Testing Fibonacci DLL...');

// Load the Fibonacci DLL
try {
  loadDll('./Bin/Fib.so');
  print('Fibonacci DLL loaded successfully\!');
  
  // Test the fib function
  print('fib(5) =', fib(5));
  print('fib(10) =', fib(10));
  
} catch (e) {
  print('Error:', e);
}
EOF < /dev/null