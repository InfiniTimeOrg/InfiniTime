# InfiniTime Tests

This directory contains unit tests for InfiniTime components.

## SessionStatistics Tests

The `test_session_statistics.cpp` file contains unit tests for the SessionStatistics class used by the Pomodoro Timer app.

### Running the tests

```bash
cd tests
make clean && make && make run
```

### Test Coverage

The tests cover:
- Daily session counting logic
- Midnight reset behavior  
- Work time accumulation accuracy
- Overflow protection
- Manual reset functionality

All tests use a mock DateTime controller to simulate day changes and test the midnight reset functionality.