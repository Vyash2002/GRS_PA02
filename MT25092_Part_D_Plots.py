import matplotlib.pyplot as plt

# ==========================================
# 1. EMBEDDED DATA (From your latest CSV run)
# ==========================================

impl = [
    'A1','A1','A1','A1','A1','A1','A1','A1','A1','A1','A1','A1','A1','A1','A1','A1',
    'A2','A2','A2','A2','A2','A2','A2','A2','A2','A2','A2','A2','A2','A2','A2','A2',
    'A3','A3','A3','A3','A3','A3','A3','A3','A3','A3','A3','A3','A3','A3','A3','A3'
]

msg_size = [
    64, 64, 64, 64, 128, 128, 128, 128, 256, 256, 256, 256, 512, 512, 512, 512,
    64, 64, 64, 64, 128, 128, 128, 128, 256, 256, 256, 256, 512, 512, 512, 512,
    64, 64, 64, 64, 128, 128, 128, 128, 256, 256, 256, 256, 512, 512, 512, 512
]

threads = [
    1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8,
    1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8,
    1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8
]

throughput = [
    1.988947, 2.346643, 2.308835, 2.329572, 4.487547, 4.539118, 4.484677, 4.569504, 
    8.462994, 8.447707, 8.441916, 8.362343, 15.247406, 15.445923, 14.782142, 15.351279,
    1.369484, 1.378557, 1.374040, 1.374339, 2.705768, 2.721112, 2.720449, 2.708191,
    5.301410, 5.315622, 5.332481, 5.216792, 9.971945, 10.052717, 10.065405, 10.115656,
    0.257249, 0.234550, 0.233612, 0.234259, 0.483707, 0.489386, 0.479833, 0.478964,
    0.981562, 0.970425, 0.974634, 0.972535, 1.921885, 1.907193, 1.901326, 1.925711
]

latency = [
    0.219031, 0.216614, 0.219950, 0.218173, 0.224806, 0.224078, 0.224755, 0.222359,
    0.240232, 0.240794, 0.240843, 0.243138, 0.266529, 0.263281, 0.275084, 0.264998,
    0.371194, 0.368879, 0.369585, 0.369885, 0.375833, 0.373695, 0.373841, 0.375367,
    0.383509, 0.382417, 0.381536, 0.389962, 0.404854, 0.404680, 0.404138, 0.402274,
    1.974534, 2.151812, 2.176302, 2.170716, 2.086230, 2.062378, 2.118958, 2.119536,
    2.059264, 2.082302, 2.068888, 2.075045, 2.115120, 2.132291, 2.139398, 2.112223
]

cpu_cycles = [
    13091797773, 0, 7925058462, 14199712524, 5649661109, 14911716751, 11662842177, 15096364439,
    14616583863, 14293044478, 14182997003, 14697715549, 14771003862, 15443470412, 15255754784, 15571569694,
    16648984635, 16485993026, 16163243548, 16134620996, 16579056616, 10811654983, 16682299301, 10287869758,
    15700676464, 11444732045, 0, 0, 9076450664, 16017673162, 15566831926, 0,
    13096957844, 10977187664, 10765788717, 9326288825, 3914829662, 8286172198, 11736855723, 4237925603,
    6695590459, 7320618491, 8792282838, 11447763783, 7965258148, 9437093916, 9311592361, 10502945013
]

llc_misses = [
    3654138, 0, 9795983, 20514679, 1551353, 16839096, 3307139, 5823513,
    2808668, 2827303, 16817066, 18193328, 24173682, 11318284, 41634469, 8631041,
    2243176, 13520023, 8201774, 88113928, 2285483, 2234952, 2378038, 600713,
    11989815, 2212743, 0, 0, 5142521, 2773363, 7863287, 0,
    84446482, 37812898, 25923188, 31224478, 9389808, 25456473, 34777886, 7998485,
    22871053, 5231444, 18234657, 23761617, 28152469, 28639078, 17306725, 16604971
]

# ==========================================
# 2. FILTERING AND CALCULATION HELPERS
# ==========================================

def get_series(target_impl, fixed_col, fixed_val, y_col_data):
    x, y = [], []
    for i in range(len(impl)):
        if impl[i] == target_impl:
            if fixed_col == 'threads' and threads[i] == fixed_val:
                x.append(msg_size[i])
                y.append(y_col_data[i])
            elif fixed_col == 'msg_size' and msg_size[i] == fixed_val:
                x.append(threads[i])
                y.append(y_col_data[i])
    return x, y

def calc_cpb_list(target_impl, fixed_col, fixed_val):
    x, cyc = get_series(target_impl, fixed_col, fixed_val, cpu_cycles)
    _, tput = get_series(target_impl, fixed_col, fixed_val, throughput)
    cpb = []
    DURATION = 5.0
    for c, t in zip(cyc, tput):
        if t <= 0 or c == 0:
            cpb.append(0)
        else:
            # Bytes = (Throughput in Gbps * 1e9 * Duration) / 8
            total_bytes = (t * 1e9 * DURATION) / 8.0
            cpb.append(c / total_bytes)
    return x, cpb

# ==========================================
# 3. PLOTTING CORE
# ==========================================

def plot_benchmark(x1, y1, x2, y2, x3, y3, title, xlabel, ylabel, filename, xticks):
    plt.figure(figsize=(10, 6))
    plt.plot(x1, y1, marker='o', linestyle='-', linewidth=2, label='A1 (Standard)')
    plt.plot(x2, y2, marker='s', linestyle='-', linewidth=2, label='A2 (Scatter/Gather)')
    plt.plot(x3, y3, marker='^', linestyle='-', linewidth=2, label='A3 (ZeroCopy)')
    
    plt.title(title, fontsize=14, fontweight='bold')
    plt.xlabel(xlabel, fontsize=12)
    plt.ylabel(ylabel, fontsize=12)
    plt.xticks(xticks)
    plt.grid(True, which='both', linestyle='--', alpha=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig(filename)
    print(f"Generated: {filename}")
    plt.close()

# ==========================================
# 4. GENERATING THE 4 REQUIRED PLOTS
# ==========================================

# Plot 1: Throughput vs Message Size
plot_benchmark(
    *get_series('A1', 'threads', 1, throughput),
    *get_series('A2', 'threads', 1, throughput),
    *get_series('A3', 'threads', 1, throughput),
    "Throughput vs Message Size (Threads=1)",
    "Message Size (Bytes)", "Throughput (Gbps)",
    "MT25092_Throughput_analysis.png", [64, 128, 256, 512]
)

# Plot 2: Latency vs Thread Count
plot_benchmark(
    *get_series('A1', 'msg_size', 512, latency),
    *get_series('A2', 'msg_size', 512, latency),
    *get_series('A3', 'msg_size', 512, latency),
    "Latency vs Thread Count (MsgSize=512)",
    "Threads", "Latency (μs)",
    "MT25092_Latency_analysis.png", [1, 2, 4, 8]
)

# Plot 3: LLC Misses vs Message Size
plot_benchmark(
    *get_series('A1', 'threads', 1, llc_misses),
    *get_series('A2', 'threads', 1, llc_misses),
    *get_series('A3', 'threads', 1, llc_misses),
    "LLC Cache Misses vs Message Size (Threads=1)",
    "Message Size (Bytes)", "LLC Misses",
    "MT25092_Cache_analysis.png", [64, 128, 256, 512]
)

# Plot 4: Cycles Per Byte (CPB) vs Message Size
plot_benchmark(
    *calc_cpb_list('A1', 'threads', 1),
    *calc_cpb_list('A2', 'threads', 1),
    *calc_cpb_list('A3', 'threads', 1),
    "CPU Efficiency (Cycles per Byte) vs Message Size",
    "Message Size (Bytes)", "Cycles Per Byte",
    "MT25092_Cycle_per_Bytes.png", [64, 128, 256, 512]
)