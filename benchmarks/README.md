# Benchmarks

Two benchmark binaries:

- `bench_parse` - parse a typical `.cco` file and report MB/s.
- `bench_serialize` - serialize a parsed result and report MB/s.

## Baseline environment

| Item | Value |
|---|---|
| CPU arch | x86_64 |
| Compiler | gcc 13 |
| Optimization | `-O2` |
| Parallelism | single core |
| Kernel | Linux 6.x |

## Thresholds

| Benchmark | Input | Target |
|---|---|---|
| `bench_parse` | 100 KB | < 1 ms (>= 100 MB/s) |
| `bench_serialize` | 100 KB | < 0.5 ms (>= 200 MB/s) |
| Peak memory | 100 KB input | < 500 KB |
| Static lib size (no `$format`, `-Os`) | - | < 100 KB |
| Static lib size (full) | - | < 200 KB |

Benchmarks are not a CI gate. CI runs a smoke variant (100 iterations)
to catch regressions.
