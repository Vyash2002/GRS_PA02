# GRS_PA02 – Analysis of Network I/O Primitives using perf
### Author: Yash Verma
### Roll Number: MT25092

## Course Information
### Course: Graduate Systems (CSE638)
### Assignment: Programming Assignment 02 (PA02)
### Topic: Analysis of Network I/O primitives using perf

# Objective

The objective of this assignment is to experimentally study the cost of data movement in network I/O by implementing and comparing:

  ### A1: Two-copy socket communication (baseline)

  ### A2: One-copy optimized socket communication

  ### A3: Zero-copy socket communication

The comparison is performed using application-level measurements and OS-level profiling via the Linux perf tool.

# Directory Structure

GRS_PA02/
  
  ├── makefile

  ├── MT25092_Part_A1_Client.c
  
  ├── MT25092_Part_A1_Server.c
  
  ├── MT25092_Part_A2_Client.c
  
  ├── MT25092_Part_A2_Server.c
  
  ├── MT25092_Part_A3_Client.c
  
  ├── MT25092_Part_A3_Server.c
  
  ├── MT25092_Part_C_RunExperiments.sh
  
  ├── MT25092_Part_D_Plots.py
  
  ├── MT25092_results.csv
  
  ├── MT25092_Throughput_analysis.png
  
  ├── MT25092_Latency_analysis.png
  
  ├── MT25092_Cache_analysis.png
  
  ├── MT25092_Cycle_per_Bytes.png

# Part A – Socket Implementations
### A1: Two-Copy Implementation

      Uses standard send() / recv() socket APIs
      Data is copied:
          1. User → Kernel
          2. Kernel → User

### A2: One-Copy Implementation

    Optimizes data movement to reduce one memory copy

    Uses sendmsg() with structured buffers

### A3: Zero-Copy Implementation

    Minimizes data copies using kernel-assisted zero-copy mechanisms

    Reduces CPU overhead and memory bandwidth usage

#### Each implementation:

    Uses a multithreaded server (one thread per client)

    Sends fixed-size messages

    Client runs for a fixed duration

# Part B: Profiling and Measurement
### Objective

The objective of Part B is to quantitatively evaluate and compare the performance of three network I/O implementations:

    A1: Two-copy socket communication

    A2: One-copy optimized communication

    A3: Zero-copy communication

The measurements focus on both application-level performance and micro-architectural behavior using the Linux perf tool.

### Experimental Setup

Platform: Linux (kernel 6.x)

    Client–Server Model: TCP-based multithreaded application

    Execution: Client and server run on the same host using separate processes

    Duration per experiment: Fixed (5 seconds)

    Threading: One thread per client connection

Each experiment is executed automatically using the script
MT25092_Part_C_RunExperiments.sh.

| Metric                     | Description                                          | Measurement Method                                              |
| -------------------------- | ---------------------------------------------------- | --------------------------------------------------------------- |
| **Throughput (Gbps)**      | Amount of data transferred per second                | Computed at the application level using total bytes transferred |
| **Latency (µs)**           | Average time taken per message transfer              | Computed at the application level                               |
| **CPU Cycles**             | Total number of CPU cycles consumed during execution | Measured using `perf stat`                                      |
| **Cache Misses (L1, LLC)** | Number of cache miss events at L1 and LLC levels     | Measured using `perf stat`                                      |
| **Context Switches**       | Number of context switches during execution          | Measured using `perf stat`                                      |

# Part C – Automated Experiment Script

### Experiments are fully automated using:
    bash 
    sudo ./MT25092_Part_C_RunExperiments.sh

### Script Functionality

    Compiles and runs all implementations

    Iterates over:

      Message sizes: 64, 128, 256, 512 bytes

      Thread counts: 1, 2, 4, 8

    Profiles client execution using perf stat

    Stores all results in a single CSV file
