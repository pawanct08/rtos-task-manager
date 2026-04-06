"""
Synthetic latency histogram generator.
Representative of ARM Cortex-M4 @ 25 MHz FreeRTOS context switch measurements
on QEMU mps2-an386.  Run once to produce docs/images/latency_histogram.png.
"""
import os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

np.random.seed(42)

# ── Representative distribution ────────────────────────────────────────────
# Bulk: Gamma(k=2.2, θ=4.5) → roughly 5-20 µs with a mode near 8 µs
bulk     = np.random.gamma(shape=2.2, scale=4.5, size=210)
# Spikes: occasional 20-50 µs from interrupt serialisation
spikes   = np.random.gamma(shape=3.0, scale=10.0, size=35)
# Outliers: rare >50 µs (cache miss + IRQ storm)
outliers = np.random.gamma(shape=2.0, scale=55.0, size=9)

samples = np.concatenate([bulk, spikes, outliers]).clip(0.5, 350.0)
N       = len(samples)

# ── Bucket edges & labels (µs) ──────────────────────────────────────────────
edges  = [0, 5, 10, 20, 50, 100, 200, 400]
labels = ['0-5', '5-10', '10-20', '20-50', '50-100', '100-200', '200+']
counts, _ = np.histogram(samples, bins=edges)

# ── Stats ────────────────────────────────────────────────────────────────────
p50  = np.percentile(samples, 50)
p95  = np.percentile(samples, 95)
p99  = np.percentile(samples, 99)
mean = samples.mean()

print(f"N={N}  min={samples.min():.1f}µs  mean={mean:.1f}µs  "
      f"p50={p50:.1f}µs  p95={p95:.1f}µs  p99={p99:.1f}µs  max={samples.max():.1f}µs")
print("Bucket counts:", dict(zip(labels, counts)))

# ── Palette: green → amber → red per severity ────────────────────────────────
bar_colors = ['#4CAF50', '#8BC34A', '#FFC107', '#FF9800', '#F44336', '#9C27B0', '#3F51B5']

fig, axes = plt.subplots(1, 2, figsize=(13, 5))
fig.suptitle(
    'FreeRTOS Context Switch Latency — QEMU mps2-an386 (ARM Cortex-M4 @ 25 MHz)',
    fontsize=13, fontweight='bold', y=1.02
)

# ── Left panel: histogram ───────────────────────────────────────────────────
ax = axes[0]
bars = ax.bar(labels, counts, color=bar_colors, edgecolor='white', linewidth=0.9, zorder=3)
ax.set_xlabel('Latency bucket (µs)', fontsize=11)
ax.set_ylabel('Sample count', fontsize=11)
ax.set_title(f'Histogram  (N = {N} samples)', fontsize=12)
ax.grid(axis='y', alpha=0.4, zorder=0)
ax.set_axisbelow(True)

for bar, cnt in zip(bars, counts):
    if cnt > 0:
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 1.5,
                str(cnt), ha='center', va='bottom', fontsize=9, fontweight='bold')

stats_txt = (f"N = {N}         min ≈ {samples.min():.0f} µs\n"
             f"mean ≈ {mean:.1f} µs     max ≈ {samples.max():.0f} µs\n"
             f"p50  ≈ {p50:.1f} µs     p95 ≈ {p95:.1f} µs\n"
             f"p99  ≈ {p99:.1f} µs")
ax.text(0.97, 0.97, stats_txt, transform=ax.transAxes,
        fontsize=9, va='top', ha='right', fontfamily='monospace',
        bbox=dict(boxstyle='round,pad=0.45', facecolor='lightyellow',
                  edgecolor='#aaa', alpha=0.92))

legend_patches = [
    mpatches.Patch(color='#4CAF50', label='< 10 µs  (nominal)'),
    mpatches.Patch(color='#FFC107', label='10-50 µs  (moderate)'),
    mpatches.Patch(color='#F44336', label='> 50 µs  (spike)'),
]
ax.legend(handles=legend_patches, fontsize=9, loc='upper center')

# ── Right panel: per-sample time series ────────────────────────────────────
ax2 = axes[1]
t   = np.arange(N) * 10           # 10 ms Probe_Task period → t in ms
sc  = ax2.scatter(t, samples, c=samples, cmap='RdYlGn_r',
                  s=13, alpha=0.75, vmin=0, vmax=80, zorder=3)
ax2.axhline(p95, color='darkorange', lw=1.5, linestyle='--',
            label=f'p95 = {p95:.1f} µs')
ax2.axhline(p99, color='crimson',    lw=1.5, linestyle='--',
            label=f'p99 = {p99:.1f} µs')
ax2.set_xlabel('Elapsed time (ms)', fontsize=11)
ax2.set_ylabel('Latency (µs)', fontsize=11)
ax2.set_title('Latency Over Time  (Probe_Task @ 10 ms)', fontsize=12)
ax2.legend(fontsize=9, loc='upper right')
ax2.grid(alpha=0.35, zorder=0)
ax2.set_axisbelow(True)
cb = plt.colorbar(sc, ax=ax2)
cb.set_label('Latency (µs)', fontsize=9)

plt.tight_layout()

out_path = os.path.join(os.path.dirname(__file__), 'images', 'latency_histogram.png')
os.makedirs(os.path.dirname(out_path), exist_ok=True)
plt.savefig(out_path, dpi=150, bbox_inches='tight')
print(f'Saved → {out_path}')
