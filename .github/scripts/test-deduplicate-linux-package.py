"""Exercise package cleanup against real ELF libraries and loader dependencies."""

import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest

spec = importlib.util.spec_from_file_location(
    "deduplicate", Path(__file__).with_name("deduplicate-linux-package.py")
)
cleanup = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cleanup)


class PackageTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.package = Path(self.temp.name)
        self.lib = self.package / "lib"
        self.lib.mkdir()
        self.target = self.lib / "libsample.so.1.2"
        (self.lib / "libsample.so.1").symlink_to(self.target.name)
        (self.lib / "libsample.so").symlink_to("libsample.so.1")

    def compile_library(self, soname):
        subprocess.run(
            ["cc", "-shared", "-fPIC", "-x", "c", "-",
             f"-Wl,-soname,{soname}", "-o", str(self.target)],
            input="int sample(void) { return 0; }", text=True, check=True,
        )

    def compile_consumer(self):
        binary = self.package / "game"
        subprocess.run(
            ["cc", "-x", "c", "-", "-L", str(self.lib), "-lsample",
             "-Wl,-rpath,$ORIGIN/lib", "-o", str(binary)],
            input="int sample(void); int main(void) { return sample(); }",
            text=True, check=True,
        )
        return binary

    def test_flattens_chain_and_preserves_runtime_loading(self):
        self.compile_library("libsample.so.1")
        binary = self.compile_consumer()
        before = cleanup.upload_size(self.package)
        size = self.target.stat().st_size
        cleanup.deduplicate(self.package)
        self.assertEqual({p.name for p in self.lib.iterdir()}, {"libsample.so.1"})
        self.assertFalse((self.lib / "libsample.so.1").is_symlink())
        self.assertEqual(before - cleanup.upload_size(self.package), 2 * size)
        subprocess.run([str(binary)], check=True)
        cleanup.deduplicate(self.package)  # Repeated cleanup is harmless.
        subprocess.run([str(binary)], check=True)

    def test_preserves_explicit_dependency_on_versioned_alias(self):
        self.compile_library("libsample.so.1.2")
        binary = self.compile_consumer()
        self.compile_library("libsample.so.1")
        before = cleanup.upload_size(self.package)
        cleanup.deduplicate(self.package)
        self.assertEqual(before, cleanup.upload_size(self.package))
        subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    unittest.main()
