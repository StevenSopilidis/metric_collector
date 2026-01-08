# High-Performance Metrics Collector (C++ + Go)

A production-style, high-throughput metrics ingestion system inspired by StatsD and Prometheus collectors.

This project demonstrates how to combine **low-level systems programming in C++** with a **high-level Go control plane** to build a fast, scalable, and observable metrics pipeline.

---

## 🚀 Why This Project?

Most metrics systems are written entirely in high-level languages. This project intentionally splits responsibilities:

- **C++** handles the *hot path*: UDP ingestion, parsing, and aggregation
- **Go** handles the *control plane*: querying, HTTP APIs, and observability

This mirrors real-world designs used in high-performance monitoring systems.

---