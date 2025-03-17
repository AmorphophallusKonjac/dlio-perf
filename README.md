# dlio-perf

## Build

```bash
mkdir build
cmake -DADD_G3LOG_UNIT_TEST:BOOL=OFF -DINSTALL_G3LOG:BOOL=OFF -S . -B build
cmake --build build --target master slave -j
```