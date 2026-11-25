#!/usr/bin/env python3
"""
verify_screen_not_black.py
Programmatic verification that GTA2 runs in attract demo mode for >=300 frames
without crashing and renders non-black pixels to the OpenGL framebuffer.
"""

import os
import subprocess
import sys
import tempfile

def main():
    bin_path = sys.argv[1] if len(sys.argv) > 1 else "./build_debug32/gta2"
    if not os.path.isfile(bin_path):
        print(f"Error: binary '{bin_path}' not found", file=sys.stderr)
        return 1

    screenshot_path = os.path.join(tempfile.gettempdir(), "gta2_verify_screen.ppm")
    if os.path.exists(screenshot_path):
        os.remove(screenshot_path)

    env = os.environ.copy()
    env["GTA2_DEMO_FRAMES"] = "300"
    env["GTA2_SCREENSHOT_PATH"] = screenshot_path

    print(f"Running {bin_path} --demo for 300 frames...")
    res = subprocess.run([bin_path, "--demo"], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    if res.returncode != 0:
        print(f"FAILED: executable exited with returncode {res.returncode}", file=sys.stderr)
        print("STDERR:", res.stderr, file=sys.stderr)
        return 1

    print("Executable finished with returncode 0.")

    if not os.path.isfile(screenshot_path):
        print(f"FAILED: screenshot file '{screenshot_path}' was not generated", file=sys.stderr)
        return 1

    with open(screenshot_path, "rb") as f:
        header = f.readline().decode().strip()
        dims = f.readline().decode().strip()
        maxval = f.readline().decode().strip()
        data = f.read()

    w, h = map(int, dims.split())
    pixel_count = len(data) // 3
    non_black = sum(1 for i in range(0, len(data), 3) if data[i] != 0 or data[i+1] != 0 or data[i+2] != 0)
    total_lum = sum(data)
    avg_lum = total_lum / len(data)

    print(f"Screenshot resolution: {w}x{h} ({pixel_count} pixels)")
    print(f"Non-black pixels: {non_black} ({non_black / pixel_count * 100:.2f}%)")
    print(f"Average channel brightness: {avg_lum:.2f}")

    if non_black == 0:
        print("FAILED: Screen is completely BLACK!", file=sys.stderr)
        return 1

    print("SUCCESS: Screen verification PASSED (non-black rendering confirmed)!")
    return 0

if __name__ == "__main__":
    sys.exit(main())
