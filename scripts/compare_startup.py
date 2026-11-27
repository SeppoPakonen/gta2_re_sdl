#!/usr/bin/env python3
"""
compare_startup.py
Compare two capture directories (or one dir vs baseline) for flicker and color metrics.

Usage:
  python3 scripts/compare_startup.py <dir_a> <dir_b>
  python3 scripts/compute_startup.py <dir> --baseline <baseline_dir>

Reports:
  - Average per-frame luminance (0-255)
  - Average RGB per channel
  - Max/mean frame-to-frame delta (flicker metric: sum of absolute pixel differences between consecutive frames)
  - Flags if differences exceed thresholds
"""

import os
import sys
import argparse
import glob
import re
from typing import List, Tuple, Optional
import numpy as np


def load_ppm_frames(dir_path: str) -> List[np.ndarray]:
    """Load all frame_XXXX.ppm files from directory, sorted by frame number."""
    pattern = os.path.join(dir_path, 'frame_*.ppm')
    files = sorted(glob.glob(pattern), key=lambda x: int(re.search(r'frame_(\d+)\.ppm', x).group(1)))
    frames = []
    for f in files:
        with open(f, 'rb') as fp:
            # Read PPM header
            magic = fp.readline().strip()
            if magic != b'P6':
                print(f"Warning: {f} is not P6 PPM, skipping", file=sys.stderr)
                continue
            # Skip comments
            line = fp.readline()
            while line.startswith(b'#'):
                line = fp.readline()
            w, h = map(int, line.split())
            maxval = int(fp.readline().strip())
            # Read pixel data
            data = fp.read()
            expected = w * h * 3
            if len(data) != expected:
                print(f"Warning: {f} size mismatch ({len(data)} vs {expected}), skipping", file=sys.stderr)
                continue
            arr = np.frombuffer(data, dtype=np.uint8).reshape(h, w, 3)
            frames.append(arr)
    return frames


def compute_metrics(frames: List[np.ndarray]) -> dict:
    """Compute luminance, RGB averages, and flicker metrics for a sequence of frames."""
    if not frames:
        return {}
    # Average luminance per frame (0-255)
    # Using standard luminance: 0.299*R + 0.587*G + 0.114*B
    lum_per_frame = [np.mean(0.299*f[:,:,0] + 0.587*f[:,:,1] + 0.114*f[:,:,2]) for f in frames]
    avg_lum = np.mean(lum_per_frame)
    # Average RGB
    avg_r = np.mean([np.mean(f[:,:,0]) for f in frames])
    avg_g = np.mean([np.mean(f[:,:,1]) for f in frames])
    avg_b = np.mean([np.mean(f[:,:,2]) for f in frames])
    # Frame-to-frame delta (flicker)
    deltas = []
    for i in range(1, len(frames)):
        # Mean absolute difference per pixel per channel
        diff = np.mean(np.abs(frames[i].astype(np.int16) - frames[i-1].astype(np.int16)))
        deltas.append(diff)
    max_delta = np.max(deltas) if deltas else 0
    mean_delta = np.mean(deltas) if deltas else 0
    return {
        'num_frames': len(frames),
        'avg_luminance': avg_lum,
        'avg_r': avg_r, 'avg_g': avg_g, 'avg_b': avg_b,
        'max_frame_delta': max_delta,
        'mean_frame_delta': mean_delta,
    }


def diff_metrics(m1: dict, m2: dict) -> dict:
    """Compute differences between two metric dicts."""
    return {
        'luminance_diff': abs(m1.get('avg_luminance', 0) - m2.get('avg_luminance', 0)),
        'r_diff': abs(m1.get('avg_r', 0) - m2.get('avg_r', 0)),
        'g_diff': abs(m1.get('avg_g', 0) - m2.get('avg_g', 0)),
        'b_diff': abs(m1.get('avg_b', 0) - m2.get('avg_b', 0)),
        'max_delta_diff': abs(m1.get('max_frame_delta', 0) - m2.get('max_frame_delta', 0)),
        'mean_delta_diff': abs(m1.get('mean_frame_delta', 0) - m2.get('mean_frame_delta', 0)),
    }


def main():
    parser = argparse.ArgumentParser(description='Compare GTA2 capture directories for flicker/color metrics')
    parser.add_argument('dir_a', help='First capture directory (or main directory)')
    parser.add_argument('dir_b', nargs='?', help='Second capture directory (optional if --baseline used)')
    parser.add_argument('--baseline', help='Baseline directory to compare dir_a against')
    parser.add_argument('--threshold', type=float, default=2.0, help='Difference threshold for flagging (default 2.0)')
    args = parser.parse_args()

    if args.baseline:
        dir_a = args.dir_a
        dir_b = args.baseline
    elif args.dir_b:
        dir_a = args.dir_a
        dir_b = args.dir_b
    else:
        print("Error: need either <dir_b> or --baseline", file=sys.stderr)
        return 1

    frames_a = load_ppm_frames(dir_a)
    frames_b = load_ppm_frames(dir_b)

    if not frames_a:
        print(f"Error: no frames found in {dir_a}", file=sys.stderr)
        return 1
    if not frames_b:
        print(f"Error: no frames found in {dir_b}", file=sys.stderr)
        return 1

    m1 = compute_metrics(frames_a)
    m2 = compute_metrics(frames_b)
    diff = diff_metrics(m1, m2)

    print(f"\n=== Capture A: {dir_a} ===")
    print(f"Frames: {m1['num_frames']}")
    print(f"Avg Luminance: {m1['avg_luminance']:.2f}")
    print(f"Avg RGB: R={m1['avg_r']:.2f} G={m1['avg_g']:.2f} B={m1['avg_b']:.2f}")
    print(f"Max Frame Delta: {m1['max_frame_delta']:.4f}")
    print(f"Mean Frame Delta: {m1['mean_frame_delta']:.4f}")

    print(f"\n=== Capture B: {dir_b} ===")
    print(f"Frames: {m2['num_frames']}")
    print(f"Avg Luminance: {m2['avg_luminance']:.2f}")
    print(f"Avg RGB: R={m2['avg_r']:.2f} G={m2['avg_g']:.2f} B={m2['avg_b']:.2f}")
    print(f"Max Frame Delta: {m2['max_frame_delta']:.4f}")
    print(f"Mean Frame Delta: {m2['mean_frame_delta']:.4f}")

    print(f"\n=== Differences (A vs B) ===")
    print(f"Luminance diff: {diff['luminance_diff']:.4f}")
    print(f"RGB diff: R={diff['r_diff']:.4f} G={diff['g_diff']:.4f} B={diff['b_diff']:.4f}")
    print(f"Max Delta diff: {diff['max_delta_diff']:.4f}")
    print(f"Mean Delta diff: {diff['mean_delta_diff']:.4f}")

    # Flag if any diff exceeds threshold
    flagged = []
    if diff['luminance_diff'] > args.threshold:
        flagged.append(f"Luminance diff {diff['luminance_diff']:.4f} > {args.threshold}")
    if diff['r_diff'] > args.threshold:
        flagged.append(f"R diff {diff['r_diff']:.4f} > {args.threshold}")
    if diff['g_diff'] > args.threshold:
        flagged.append(f"G diff {diff['g_diff']:.4f} > {args.threshold}")
    if diff['b_diff'] > args.threshold:
        flagged.append(f"B diff {diff['b_diff']:.4f} > {args.threshold}")
    if diff['max_delta_diff'] > args.threshold:
        flagged.append(f"Max frame delta diff {diff['max_delta_diff']:.4f} > {args.threshold}")
    if diff['mean_delta_diff'] > args.threshold:
        flagged.append(f"Mean frame delta diff {diff['mean_delta_diff']:.4f} > {args.threshold}")

    if flagged:
        print("\n⚠ FLAGGED (exceeds threshold):")
        for f in flagged:
            print(f"  - {f}")
        return 2
    else:
        print("\n✓ All metrics within threshold")
        return 0


if __name__ == '__main__':
    sys.exit(main())