"""Check the application-only Arduino bus bridge without hardware."""
import os
from pathlib import Path
import subprocess
import tempfile

here = Path(__file__).resolve().parent
with tempfile.TemporaryDirectory(prefix="st25-bridge-") as temporary:
    binary = str(Path(temporary) / "bridge-test")
    subprocess.run([
        os.environ.get("CXX", "c++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=address,undefined", "-DARDUINO_ARCH_ESP32",
        "-DOPENKNX_NFC_ST25R3916_I2C", "-I" + str(here / "stubs"),
        "-I" + str(here.parents[2] / "src"), str(here / "bridge_test.cpp"),
        "-o", binary,
    ], check=True)
    subprocess.run([binary], check=True)
