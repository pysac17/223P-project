import pandas as pd
import matplotlib.pyplot as plt
import os
import glob

# Configuration
EXP1_DIR = 'exp1'  # Folder for Thread scaling experiments
EXP2_DIR = 'exp2'  # Folder for Contention experiments
OUTPUT_DIR = 'graphs'
DURATION = 10      # Duration used in experiments (seconds)

# Create output directory
os.makedirs(OUTPUT_DIR, exist_ok=True)

def load_data(folder):
    """Loads all CSVs from a folder into a single DataFrame with metadata."""
    all_data = []
    for file in glob.glob(os.path.join(folder, "*.csv")):
        df = pd.read_csv(file)
        
        # Parse filename: protocol_tX_workloadY.csv
        basename = os.path.basename(file)
        parts = basename.replace('.csv', '').split('_')
        
        protocol = parts[0].upper() # OCC or 2PL
        param = parts[1]            # e.g., "t8" or "c0.5"
        workload_id = parts[2]      # e.g., "workload1"
        
        df['protocol'] = protocol
        df['workload'] = workload_id
        
        if param.startswith('t'):
            df['threads'] = int(param[1:])
            df['contention'] = 0.5 
        elif param.startswith('c'):
            df['contention'] = float(param[1:])
            df['threads'] = 8 
            
        all_data.append(df)
    
    if not all_data:
        return pd.DataFrame()
    
    return pd.concat(all_data, ignore_index=True)

def plot_metric(df, x_col, y_col, title, xlabel, ylabel, filename):
    """Generates a line plot comparing OCC vs 2PL."""
    plt.figure(figsize=(8, 5))
    
    for protocol in ['OCC', '2PL']:
        subset = df[df['protocol'] == protocol]
        subset = subset.sort_values(x_col)
        
        # Calculate throughput if needed
        if y_col == 'throughput':
            grouped = subset.groupby(x_col).size().reset_index(name='count')
            grouped[y_col] = grouped['count'] / DURATION
            subset = grouped
            
        plt.plot(subset[x_col], subset[y_col], marker='o', label=protocol)

    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, filename))
    plt.close()
    print(f"Generated: {filename}")

# ==========================================
# Main Execution
# ==========================================

print("Loading Experiment Data...")

# Load Data
df_exp1 = load_data(EXP1_DIR)
df_exp2 = load_data(EXP2_DIR)

# Process Workload 1
w1_exp1 = df_exp1[df_exp1['workload'] == 'workload1']
w1_exp2 = df_exp2[df_exp2['workload'] == 'workload1']

if not w1_exp1.empty:
    print("\nGenerating Workload 1 Graphs...")
    plot_metric(w1_exp1, 'threads', 'throughput', 
                'Workload 1: Throughput vs Threads', 
                'Threads', 'Throughput (txns/sec)', 
                'w1_throughput_vs_threads.png')

    plot_metric(w1_exp1, 'threads', 'responseTimeMs', 
                'Workload 1: Latency vs Threads', 
                'Threads', 'Avg Response Time (ms)', 
                'w1_latency_vs_threads.png')

if not w1_exp2.empty:
    plot_metric(w1_exp2, 'contention', 'throughput', 
                'Workload 1: Throughput vs Contention', 
                'Contention', 'Throughput (txns/sec)', 
                'w1_throughput_vs_contention.png')

    plot_metric(w1_exp2, 'contention', 'responseTimeMs', 
                'Workload 1: Latency vs Contention', 
                'Contention', 'Avg Response Time (ms)', 
                'w1_latency_vs_contention.png')
    
    plot_metric(w1_exp2, 'contention', 'retryCount', 
                'Workload 1: Retries vs Contention', 
                'Contention', 'Avg Retry Count', 
                'w1_retries_vs_contention.png')

# Process Workload 2
w2_exp1 = df_exp1[df_exp1['workload'] == 'workload2']
w2_exp2 = df_exp2[df_exp2['workload'] == 'workload2']

if not w2_exp1.empty:
    print("\nGenerating Workload 2 Graphs...")
    plot_metric(w2_exp1, 'threads', 'throughput', 
                'Workload 2: Throughput vs Threads', 
                'Threads', 'Throughput (txns/sec)', 
                'w2_throughput_vs_threads.png')

    plot_metric(w2_exp1, 'threads', 'responseTimeMs', 
                'Workload 2: Latency vs Threads', 
                'Threads', 'Avg Response Time (ms)', 
                'w2_latency_vs_threads.png')

if not w2_exp2.empty:
    plot_metric(w2_exp2, 'contention', 'throughput', 
                'Workload 2: Throughput vs Contention', 
                'Contention', 'Throughput (txns/sec)', 
                'w2_throughput_vs_contention.png')

    plot_metric(w2_exp2, 'contention', 'responseTimeMs', 
                'Workload 2: Latency vs Contention', 
                'Contention', 'Avg Response Time (ms)', 
                'w2_latency_vs_contention.png')

    plot_metric(w2_exp2, 'contention', 'retryCount', 
                'Workload 2: Retries vs Contention', 
                'Contention', 'Avg Retry Count', 
                'w2_retries_vs_contention.png')

print("\nAll graphs generated in 'graphs/' directory.")