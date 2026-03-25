#!/usr/bin/env python3
"""
scripts/plot_latency.py — Week 2
Parses UART output from the RTOS task manager and plots a context switch
latency histogram.

Usage:
  # 1. Run QEMU and capture output:
  ./scripts/run_qemu.sh | tee uart.log
  # 2. Type 'latency' in the CLI to trigger CSV dump, then Ctrl+A X to quit.
  # 3. Plot:
  python3 scripts/plot_latency.py uart.log
  # 4. Save PNG instead of showing:
  python3 scripts/plot_latency.py uart.log --output latency.png
"""

import sys
import argparse
import re
from pathlib import Path


def parse_csv(text):
    match = re.search(
        r"LATENCY_CSV_START\r?\nsample_us\r?\n(.*?)LATENCY_CSV_END",
        text, re.DOTALL
    )
    if not match:
        print("ERROR: No LATENCY_CSV_START/END block found.")
        print("  Type 'latency' in the UART CLI before capturing.")
        sys.exit(1)

    samples = []
    for line in match.group(1).strip().splitlines():
        line = line.strip()
        if line.isdigit():
            samples.append(int(line))
    return samples


def print_ascii_histogram(samples):
    buckets = [(0,5),(5,10),(10,20),(20,50),(50,100),(100,200),(200,10**9)]
    labels  = ["  0-5us"," 5-10us","10-20us","20-50us","50-100u","100-200"," 200us+"]
    counts  = [sum(1 for s in samples if lo <= s < hi) for lo,hi in buckets]
    total   = len(samples)
    mx      = max(counts) if counts else 1

    print(f"\nContext Switch Latency  (n={total}  "
          f"min={min(samples)}us  max={max(samples)}us  "
          f"avg={sum(samples)//total}us)\n")
    print(f"{'Bucket':>9}  {'Count':>6}  {'%':>4}  Histogram")
    print("-" * 55)
    for label, count in zip(labels, counts):
        bar = "█" * int(count * 35 / mx) if mx else ""
        pct = count * 100 // total if total else 0
        print(f"{label:>9}  {count:>6}  {pct:>3}%  {bar}")
    print()


def plot_matplotlib(samples, output):
    import matplotlib.pyplot as plt
    import numpy as np

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
    fig.suptitle("RTOS Context Switch Latency — FreeRTOS / Cortex-M4 (QEMU)",
                 fontsize=13, fontweight="bold")

    # Histogram
    bins = [0, 5, 10, 20, 50, 100, 200, max(samples) + 1]
    ax1.hist(samples, bins=bins, color="#4C8BF5", edgecolor="white", linewidth=0.6)
    ax1.set_xlabel("Latency (µs)"); ax1.set_ylabel("Count")
    ax1.set_title("Distribution")
    ax1.set_xticks([0, 5, 10, 20, 50, 100, 200])
    ax1.set_xticklabels(["0","5","10","20","50","100","200µs"])

    # Time series + rolling average
    window = max(1, len(samples) // 20)
    rolling = np.convolve(samples, np.ones(window)/window, mode="valid")
    ax2.plot(samples, color="#B0C4DE", lw=0.5, alpha=0.6, label="Raw")
    ax2.plot(range(window-1, len(samples)), rolling,
             color="#E8593C", lw=1.8, label=f"Rolling avg ({window})")
    ax2.axhline(sum(samples)/len(samples), color="#27ae60",
                ls="--", lw=1, label="Mean")
    ax2.set_xlabel("Sample #"); ax2.set_ylabel("Latency (µs)")
    ax2.set_title("Over time"); ax2.legend(fontsize=9)

    # Stats box
    p95 = int(np.percentile(samples, 95))
    p99 = int(np.percentile(samples, 99))
    stats = (f"n   = {len(samples)}\n"
             f"min = {min(samples)} µs\n"
             f"max = {max(samples)} µs\n"
             f"avg = {sum(samples)//len(samples)} µs\n"
             f"p95 = {p95} µs\n"
             f"p99 = {p99} µs")
    ax2.text(0.02, 0.97, stats, transform=ax2.transAxes, fontsize=9,
             va="top", bbox=dict(boxstyle="round,pad=0.4",
                                 facecolor="#f0f4ff", alpha=0.85))

    plt.tight_layout()
    if output:
        plt.savefig(output, dpi=150, bbox_inches="tight")
        print(f"Saved -> {output}")
    else:
        plt.show()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("logfile", nargs="?", help="Captured UART log file")
    parser.add_argument("--output", "-o", help="Save PNG to this path")
    parser.add_argument("--ascii", action="store_true", help="ASCII only")
    args = parser.parse_args()

    if not args.logfile:
        parser.print_help(); sys.exit(1)

    samples = parse_csv(Path(args.logfile).read_text(errors="replace"))
    print(f"Parsed {len(samples)} samples.")
    print_ascii_histogram(samples)

    if not args.ascii:
        try:
            plot_matplotlib(samples, args.output)
        except ImportError:
            print("Install matplotlib: pip3 install matplotlib numpy")

if __name__ == "__main__":
    main()
