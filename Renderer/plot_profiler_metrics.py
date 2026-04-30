import argparse
import csv
import math
import sys
from collections import deque
from pathlib import Path
from typing import Dict, List, Tuple

import matplotlib.pyplot as plt


DEFAULT_FPS_SOURCE = "max"


def parse_float(value: str) -> float:
    if value is None:
        return math.nan
    value = value.strip()
    if value == "":
        return math.nan
    try:
        return float(value)
    except ValueError:
        return math.nan


def rolling_mean(values: List[float], window: int) -> List[float]:
    if window <= 1:
        return values[:]

    output: List[float] = []
    running_sum = 0.0
    running_count = 0
    queue: deque[Tuple[float, bool]] = deque()

    for value in values:
        finite = math.isfinite(value)
        queue.append((value, finite))
        if finite:
            running_sum += value
            running_count += 1

        if len(queue) > window:
            old_value, old_finite = queue.popleft()
            if old_finite:
                running_sum -= old_value
                running_count -= 1

        if running_count == 0:
            output.append(math.nan)
        else:
            output.append(running_sum / running_count)

    return output


def rolling_min(values: List[float], window: int) -> List[float]:
    if window <= 1:
        return values[:]

    output: List[float] = []
    queue: deque[float] = deque()

    for value in values:
        queue.append(value)
        if len(queue) > window:
            queue.popleft()

        data = finite_values(queue)
        output.append(min(data) if data else math.nan)

    return output


def rolling_std(values: List[float], window: int) -> List[float]:
    if window <= 1:
        return [0.0 if math.isfinite(v) else math.nan for v in values]

    output: List[float] = []
    queue: deque[float] = deque()

    for value in values:
        queue.append(value)
        if len(queue) > window:
            queue.popleft()

        data = finite_values(queue)
        if not data:
            output.append(math.nan)
            continue

        mean = sum(data) / len(data)
        variance = sum((v - mean) ** 2 for v in data) / len(data)
        output.append(math.sqrt(variance))

    return output


def finite_values(values) -> List[float]:
    return [v for v in values if math.isfinite(v)]


def safe_average(values: List[float]) -> float:
    data = finite_values(values)
    return sum(data) / len(data) if data else math.nan


def percentile(values: List[float], p: float) -> float:
    data = sorted(finite_values(values))
    if not data:
        return math.nan
    if len(data) == 1:
        return data[0]

    scaled = p * (len(data) - 1)
    low = int(math.floor(scaled))
    high = int(math.ceil(scaled))
    frac = scaled - low
    return data[low] + (data[high] - data[low]) * frac


def make_fps_source(cpu: List[float], gpu: List[float], source: str) -> List[float]:
    values: List[float] = []
    for cpu_value, gpu_value in zip(cpu, gpu):
        if source == "cpu":
            values.append(cpu_value)
        elif source == "gpu":
            values.append(gpu_value)
        else:
            candidates = [v for v in (cpu_value, gpu_value) if math.isfinite(v)]
            values.append(max(candidates) if candidates else math.nan)
    return values


def ms_to_fps(values: List[float]) -> List[float]:
    fps: List[float] = []
    for value in values:
        if math.isfinite(value) and value > 0.0:
            fps.append(1000.0 / value)
        else:
            fps.append(math.nan)
    return fps


def latest_finite(values: List[float]) -> float:
    for value in reversed(values):
        if math.isfinite(value):
            return value
    return math.nan


def split_segments(frames: List[float]) -> List[Tuple[int, int]]:
    segments: List[Tuple[int, int]] = []
    start = -1

    for i, frame in enumerate(frames):
        if not math.isfinite(frame):
            if start >= 0 and i > start:
                segments.append((start, i))
            start = -1
            continue

        if start < 0:
            start = i
            continue

        prev = frames[i - 1]
        if (not math.isfinite(prev)) or frame <= prev:
            if i > start:
                segments.append((start, i))
            start = i

    if start >= 0 and len(frames) > start:
        segments.append((start, len(frames)))

    return segments


def detect_phase_changes(values: List[float], window: int, threshold_sigma: float = 2.5) -> List[int]:
    if len(values) < max(window * 2, 10):
        return []

    smooth = rolling_mean(values, window)
    deltas: List[float] = [math.nan]
    for i in range(1, len(smooth)):
        a = smooth[i - 1]
        b = smooth[i]
        deltas.append(b - a if math.isfinite(a) and math.isfinite(b) else math.nan)

    finite_deltas = finite_values(deltas)
    if len(finite_deltas) < 10:
        return []

    mean_delta = safe_average(finite_deltas)
    std_delta = math.sqrt(sum((d - mean_delta) ** 2 for d in finite_deltas) / len(finite_deltas))
    if not math.isfinite(std_delta) or std_delta == 0:
        return []

    markers: List[int] = []
    last_marker = -window
    for i, d in enumerate(deltas):
        if not math.isfinite(d):
            continue
        if abs(d - mean_delta) >= threshold_sigma * std_delta and (i - last_marker) >= window:
            markers.append(i)
            last_marker = i
    return markers


def validate_metrics(metrics: Dict[str, List[float]]) -> None:
    required = ["frame", "cpu_ms", "gpu_ms", "draw_calls"]
    missing = [name for name in required if name not in metrics]
    if missing:
        raise ValueError(f"CSV is missing required columns: {', '.join(missing)}")

    lengths = {name: len(values) for name, values in metrics.items()}
    if len(set(lengths.values())) > 1:
        raise ValueError("CSV columns have different lengths; file appears malformed.")

    frame_values = metrics["frame"]
    if any(not math.isfinite(v) for v in frame_values):
        print("Warning: non-finite frame values found; charts may contain gaps.", file=sys.stderr)

    discontinuities = 0
    for i in range(1, len(frame_values)):
        prev = frame_values[i - 1]
        cur = frame_values[i]
        if math.isfinite(prev) and math.isfinite(cur) and cur <= prev:
            discontinuities += 1

    if discontinuities > 0:
        print(
            f"Warning: detected {discontinuities} non-increasing frame transitions; plotting in segments.",
            file=sys.stderr,
        )

    gpu_values = metrics["gpu_ms"]
    missing_gpu = sum(1 for v in gpu_values if not math.isfinite(v))
    if missing_gpu > 0:
        print(
            f"Warning: {missing_gpu} rows have missing GPU timings; those points are omitted from GPU stats.",
            file=sys.stderr,
        )


def plot_segments(
    ax: plt.Axes,
    frames: List[float],
    values: List[float],
    segments: List[Tuple[int, int]],
    *,
    label: str,
    color: str,
    alpha: float,
    linewidth: float,
    linestyle: str = "-",
) -> None:
    for i, (start, end) in enumerate(segments):
        ax.plot(
            frames[start:end],
            values[start:end],
            label=label if i == 0 else None,
            color=color,
            alpha=alpha,
            linewidth=linewidth,
            linestyle=linestyle,
        )


def add_phase_markers(ax: plt.Axes, frames: List[float], indices: List[int], label: str = "Phase change") -> None:
    for i, idx in enumerate(indices):
        if 0 <= idx < len(frames) and math.isfinite(frames[idx]):
            ax.axvline(
                frames[idx],
                color="#7f7f7f",
                linestyle=":",
                linewidth=1.1,
                alpha=0.85,
                label=label if i == 0 else None,
            )


def plot_frame_timeline(
    ax: plt.Axes,
    frames: List[float],
    cpu: List[float],
    gpu: List[float],
    cpu_roll: List[float],
    gpu_roll: List[float],
    segments: List[Tuple[int, int]],
    window: int,
    phase_changes: List[int],
) -> None:
    plot_segments(ax, frames, cpu, segments, label="CPU ms", color="#1f77b4", alpha=0.30, linewidth=1.1)
    plot_segments(ax, frames, cpu_roll, segments, label=f"CPU {window}-frame avg", color="#1f77b4", alpha=1.0, linewidth=2.1)
    plot_segments(ax, frames, gpu, segments, label="GPU ms", color="#d62728", alpha=0.30, linewidth=1.1)
    plot_segments(ax, frames, gpu_roll, segments, label=f"GPU {window}-frame avg", color="#d62728", alpha=1.0, linewidth=2.1)

    ax.axhline(16.67, color="#2ca02c", linestyle="--", linewidth=1.2, label="16.67 ms (60 FPS)")
    add_phase_markers(ax, frames, phase_changes)
    ax.set_title("Frame Time Timeline")
    ax.set_xlabel("Frame")
    ax.set_ylabel("Milliseconds")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper right")


def plot_fps_stability(
    ax: plt.Axes,
    frames: List[float],
    fps: List[float],
    fps_roll: List[float],
    fps_floor: List[float],
    fps_source_name: str,
    segments: List[Tuple[int, int]],
    window: int,
    phase_changes: List[int],
) -> None:
    plot_segments(ax, frames, fps, segments, label=f"Instant FPS ({fps_source_name})", color="#17becf", alpha=0.25, linewidth=1.0)
    plot_segments(ax, frames, fps_roll, segments, label=f"{window}-frame avg FPS", color="#17becf", alpha=1.0, linewidth=2.0)
    plot_segments(ax, frames, fps_floor, segments, label=f"{window}-frame worst FPS", color="#bcbd22", alpha=1.0, linewidth=1.8)
    add_phase_markers(ax, frames, phase_changes)

    ax.axhline(60.0, color="#2ca02c", linestyle="--", linewidth=1.2, label="60 FPS")
    ax.axhline(30.0, color="#ff7f0e", linestyle="--", linewidth=1.2, label="30 FPS")
    ax.set_title("FPS Stability")
    ax.set_xlabel("Frame")
    ax.set_ylabel("Frames / Second")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper right")


def plot_draw_calls_and_cpu(
    ax_draws: plt.Axes,
    frames: List[float],
    draws: List[float],
    cpu_roll: List[float],
    segments: List[Tuple[int, int]],
    phase_changes: List[int],
) -> None:
    ax_cpu = ax_draws.twinx()

    plot_segments(ax_draws, frames, draws, segments, label="Draw calls", color="#9467bd", alpha=1.0, linewidth=1.6)
    plot_segments(ax_cpu, frames, cpu_roll, segments, label="CPU ms avg", color="#1f77b4", alpha=0.8, linewidth=1.8)
    add_phase_markers(ax_draws, frames, phase_changes)

    ax_draws.set_title("Draw Calls And CPU Trend")
    ax_draws.set_xlabel("Frame")
    ax_draws.set_ylabel("Draw Calls")
    ax_cpu.set_ylabel("CPU ms")
    ax_draws.grid(True, alpha=0.3)

    left_handles, left_labels = ax_draws.get_legend_handles_labels()
    right_handles, right_labels = ax_cpu.get_legend_handles_labels()
    ax_draws.legend(left_handles + right_handles, left_labels + right_labels, loc="upper right")


def plot_gpu_sections(
    ax: plt.Axes,
    metrics: Dict[str, List[float]],
    frames: List[float],
    section_cols: List[str],
    segments: List[Tuple[int, int]],
    phase_changes: List[int],
) -> None:
    if not section_cols:
        ax.text(0.5, 0.5, "No per-section GPU columns found", ha="center", va="center")
        ax.set_title("GPU Section Breakdown")
        ax.set_axis_off()
        return

    section_labels = [name.replace("_gpu_ms", "") for name in section_cols]
    for i, (start, end) in enumerate(segments):
        x = frames[start:end]
        section_values = [
            [0.0 if not math.isfinite(v) else v for v in metrics[name][start:end]]
            for name in section_cols
        ]
        ax.stackplot(x, section_values, labels=section_labels if i == 0 else None, alpha=0.9)

    add_phase_markers(ax, frames, phase_changes)
    ax.set_title("GPU Section Breakdown (Stacked)")
    ax.set_xlabel("Frame")
    ax.set_ylabel("Milliseconds")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper right", fontsize=8)


def plot_timing_distribution(ax: plt.Axes, cpu: List[float], gpu: List[float]) -> None:
    cpu_hist = finite_values(cpu)
    gpu_hist = finite_values(gpu)

    if cpu_hist:
        ax.hist(cpu_hist, bins=30, alpha=0.45, color="#1f77b4", label="CPU ms")
    if gpu_hist:
        ax.hist(gpu_hist, bins=30, alpha=0.45, color="#d62728", label="GPU ms")

    cpu_p95 = percentile(cpu, 0.95)
    if math.isfinite(cpu_p95):
        ax.axvline(cpu_p95, color="#1f77b4", linestyle="--", linewidth=1.2, alpha=0.9, label="CPU p95")

    gpu_p95 = percentile(gpu, 0.95)
    if math.isfinite(gpu_p95):
        ax.axvline(gpu_p95, color="#ff7f0e", linestyle=":", linewidth=1.8, alpha=0.9, label="GPU p95")

    ax.set_title("CPU and GPU Time Distribution")
    ax.set_xlabel("Milliseconds")
    ax.set_ylabel("Frame Count")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper right", fontsize=8)


def build_summary_text(
    metrics: Dict[str, List[float]],
    frames: List[float],
    cpu: List[float],
    gpu: List[float],
    draws: List[float],
    fps: List[float],
    section_cols: List[str],
    fps_source_name: str,
) -> str:
    cpu_avg_csv = metrics.get("cpu_avg_ms", [])
    cpu_p95_csv = metrics.get("cpu_p95_ms", [])
    cpu_p99_csv = metrics.get("cpu_p99_ms", [])
    gpu_avg_csv = metrics.get("gpu_avg_ms", [])
    gpu_p95_csv = metrics.get("gpu_p95_ms", [])
    gpu_p99_csv = metrics.get("gpu_p99_ms", [])

    cpu_avg = latest_finite(cpu_avg_csv) if cpu_avg_csv else safe_average(cpu)
    cpu_p95 = latest_finite(cpu_p95_csv) if cpu_p95_csv else percentile(cpu, 0.95)
    cpu_p99 = latest_finite(cpu_p99_csv) if cpu_p99_csv else percentile(cpu, 0.99)
    gpu_avg = latest_finite(gpu_avg_csv) if gpu_avg_csv else safe_average(gpu)
    gpu_p95 = latest_finite(gpu_p95_csv) if gpu_p95_csv else percentile(gpu, 0.95)
    gpu_p99 = latest_finite(gpu_p99_csv) if gpu_p99_csv else percentile(gpu, 0.99)

    draw_avg = safe_average(draws)
    latest_cpu = latest_finite(cpu)
    latest_gpu = latest_finite(gpu)
    latest_draws = latest_finite(draws)
    latest_fps = latest_finite(fps)
    gpu_missing = sum(1 for v in gpu if not math.isfinite(v))

    section_lines: List[str] = []
    for name in section_cols:
        values = metrics[name]
        avg_value = safe_average(values)
        latest_value = latest_finite(values)
        label = name.replace("_gpu_ms", "")
        if math.isfinite(avg_value) or math.isfinite(latest_value):
            section_lines.append(f"{label}: latest {latest_value:.2f} ms | avg {avg_value:.2f} ms")

    return "\n".join(
        [
            f"Frames analyzed: {len(frames)}",
            f"Latest CPU: {latest_cpu:.2f} ms",
            f"Latest GPU: {latest_gpu:.2f} ms",
            f"Latest FPS ({fps_source_name}): {latest_fps:.1f}",
            f"Latest draw calls: {latest_draws:.0f}",
            "",
            f"CPU avg / p95 / p99: {cpu_avg:.2f} / {cpu_p95:.2f} / {cpu_p99:.2f} ms",
            f"GPU avg / p95 / p99: {gpu_avg:.2f} / {gpu_p95:.2f} / {gpu_p99:.2f} ms",
            f"Draw call avg: {draw_avg:.1f}",
            f"Missing GPU samples: {gpu_missing}",
            "",
            "GPU section stats:",
            *section_lines[:8],
        ]
    )


def load_metrics(csv_path: Path) -> Dict[str, List[float]]:
    with csv_path.open("r", newline="", encoding="utf-8") as file:
        reader = csv.DictReader(file)
        if reader.fieldnames is None:
            raise ValueError("CSV file does not contain a header.")

        columns: Dict[str, List[float]] = {name: [] for name in reader.fieldnames}
        for row in reader:
            for name in reader.fieldnames:
                columns[name].append(parse_float(row.get(name, "")))

    if not columns or len(next(iter(columns.values()), [])) == 0:
        raise ValueError("CSV file has no data rows.")

    return columns


def build_charts(metrics: Dict[str, List[float]], window: int, title: str, fps_source_name: str) -> plt.Figure:
    frames = metrics.get("frame")
    cpu = metrics.get("cpu_ms")
    gpu = metrics.get("gpu_ms")
    draws = metrics.get("draw_calls")

    if frames is None or cpu is None or gpu is None or draws is None:
        raise ValueError("CSV must contain frame,cpu_ms,gpu_ms,draw_calls columns.")

    segments = split_segments(frames)
    section_cols = sorted([name for name in metrics.keys() if name.endswith("_gpu_ms") and name != "gpu_ms"])

    timing_source = make_fps_source(cpu, gpu, fps_source_name)
    fps = ms_to_fps(timing_source)
    cpu_roll = rolling_mean(cpu, window)
    gpu_roll = rolling_mean(gpu, window)
    fps_roll = rolling_mean(fps, window)
    fps_floor = rolling_min(fps, window)
    phase_changes = detect_phase_changes(timing_source, max(window // 2, 10))

    fig = plt.figure(figsize=(18, 16), constrained_layout=True)
    grid = fig.add_gridspec(3, 2, height_ratios=[1.15, 1.0, 1.0])

    ax_fps = fig.add_subplot(grid[0, 0])
    ax_draws = fig.add_subplot(grid[0, 1])
    ax_frame = fig.add_subplot(grid[1, 0])
    ax_sections = fig.add_subplot(grid[1, 1])
    ax_hist = fig.add_subplot(grid[2, 0])
    ax_summary = fig.add_subplot(grid[2, 1])

    plot_frame_timeline(ax_frame, frames, cpu, gpu, cpu_roll, gpu_roll, segments, window, phase_changes)
    plot_fps_stability(ax_fps, frames, fps, fps_roll, fps_floor, fps_source_name, segments, window, phase_changes)
    plot_draw_calls_and_cpu(ax_draws, frames, draws, cpu_roll, segments, phase_changes)
    plot_gpu_sections(ax_sections, metrics, frames, section_cols, segments, phase_changes)
    plot_timing_distribution(ax_hist, cpu, gpu)

    summary_text = build_summary_text(metrics, frames, cpu, gpu, draws, fps, section_cols, fps_source_name)
    ax_summary.axis("off")
    ax_summary.set_title("Profiler Summary")
    ax_summary.text(0.01, 0.98, summary_text, ha="left", va="top", fontsize=10, family="monospace")

    fig.suptitle(title, fontsize=15)
    return fig


def main() -> int:
    parser = argparse.ArgumentParser(description="Plot renderer profiler metrics from CSV.")
    parser.add_argument("--csv", default="profiler_metrics.csv", help="Path to profiler CSV file.")
    parser.add_argument("--window", type=int, default=60, help="Rolling average window size in frames.")
    parser.add_argument("--fps-source", choices=["cpu", "gpu", "max"], default=DEFAULT_FPS_SOURCE, help="Which timing source should drive FPS plots.")
    parser.add_argument("--save", default="profiler_charts.png", help="Output image path.")
    parser.add_argument("--no-show", action="store_true", help="Do not open an interactive window.")
    args = parser.parse_args()

    csv_path = Path(args.csv)
    if not csv_path.exists():
        raise FileNotFoundError(f"CSV file not found: {csv_path}")

    metrics = load_metrics(csv_path)
    validate_metrics(metrics)
    title = f"Profiler Metrics ({csv_path.name})"
    figure = build_charts(metrics, max(args.window, 1), title, args.fps_source)

    if args.save:
        output_path = Path(args.save)
        figure.savefig(output_path, dpi=150)
        print(f"Saved chart image to: {output_path.resolve()}")

    if not args.no_show:
        plt.show()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
