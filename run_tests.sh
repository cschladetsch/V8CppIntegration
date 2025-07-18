#!/bin/bash

# Top-level test runner that calls the main test script in Shell/
# This provides a convenient way to run tests from the project root

exec ./shell_tests/run_tests.sh "$@"