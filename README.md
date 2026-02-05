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


