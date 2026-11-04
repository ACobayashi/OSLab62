#!/usr/bin/env python3
"""Prepare an isolated kernel for exception tests, never edit submission sources."""

import argparse
from pathlib import Path
import re
import shutil


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("build", type=Path, help="new, disposable build directory")
args = parser.parse_args()
support = Path(__file__).resolve().parent
source = support.parents[1] / "code"
build = args.build.resolve()
if build.exists():
    parser.error("build directory already exists; choose a fresh directory")
if build == source or source in build.parents:
    parser.error("build directory must be outside submission code")

shutil.copytree(source, build, ignore=shutil.ignore_patterns("bin", "obj", ".qemu*"))
testdir = build / "kern" / "test"
testdir.mkdir()
for name in ("lab3_exception_test.c", "lab3_exception_test.h", "probes.S"):
    shutil.copyfile(support / name, testdir / name)

entry = build / "kern" / "trap" / "trapentry.S"
if "YOUR CODE" in entry.read_text():
    shutil.copyfile(entry, entry.with_name("trapentry-skeleton.inc"))
    shutil.copyfile(support / "reference-entry.S", entry)
    print("TEST ONLY: using reference Trap entry/return; teammate code is unfinished")

trap = build / "kern" / "trap" / "trap.c"
if not re.search(r"void\s+interrupt_handler\s*\([^)]*\)\s*\{", trap.read_text()):
    shutil.copyfile(support / "interrupt-stub.c", testdir / "interrupt-stub.c")
    print("TEST ONLY: unexpected interrupts abort; no timer implementation is supplied")

makefile = build / "Makefile"
text = makefile.read_text()
anchor = "KCFLAGS\t\t+= $(addprefix -I,$(KINCLUDE))"
if text.count(anchor) != 1:
    raise RuntimeError("Makefile include anchor is missing or ambiguous")
text = text.replace(anchor, "KINCLUDE += kern/test\nKSRCDIR += kern/test\n\n" + anchor)
makefile.write_text(text)

init = build / "kern" / "init" / "init.c"
text = init.read_text()
anchor = "#include <dtb.h>"
if text.count(anchor) != 1:
    raise RuntimeError("init include anchor is missing or ambiguous")
text = text.replace(anchor, anchor + "\n#include <lab3_exception_test.h>")
matches = list(re.finditer(r"(?m)^\s*idt_init\(\);[^\n]*\n", text))
if len(matches) != 2:
    raise RuntimeError("expected exactly two idt_init() calls")
position = matches[-1].end()
text = text[:position] + "    lab3_test_exceptions();\n" + text[position:]
init.write_text(text)
print("Prepared exception-only build:", build)
print("Official grading files and submission sources were not modified")
