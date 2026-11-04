#!/usr/bin/env python3
"""Create a fresh isolated HYC verification copy without editing submission code."""

import argparse
import hashlib
from pathlib import Path
import re
import shutil


def main():
    support = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build", type=Path, help="new directory outside submission code")
    parser.add_argument("--mode", choices=("context", "exception"), default="context")
    parser.add_argument("--source", type=Path, default=support.parents[2] / "code")
    args = parser.parse_args()
    source, build = args.source.resolve(), args.build.resolve()
    if not (source / "Makefile").is_file():
        parser.error("source must be the lab3 code directory")
    if build.exists():
        parser.error("build directory already exists; choose a fresh directory")
    if source == build or source in build.parents or build in source.parents:
        parser.error("build and source must not contain one another")
    if build == support or support in build.parents or build in support.parents:
        parser.error("build must be outside the report evidence directory")

    entry = (source / "kern/trap/trapentry.S").read_bytes()
    if entry != (support / "trapentry.S.snapshot").read_bytes():
        parser.error("trapentry.S differs from the reported snapshot; review before retesting")
    trap = (source / "kern/trap/trap.c").read_text(encoding="utf-8")
    if not re.search(r"void\s+exception_handler\s*\([^)]*\)\s*\{", trap):
        parser.error("completed exception_handler is required")
    init = (source / "kern/init/init.c").read_text(encoding="utf-8")
    includes = "#include <dtb.h>"
    idts = list(re.finditer(r"(?m)^\s*idt_init\(\);[^\n]*\n", init))
    if init.count(includes) != 1 or len(idts) != 2:
        parser.error("init.c anchors changed; expected dtb include and two idt_init calls")
    if (source / "kern/test").exists():
        parser.error("source already contains kern/test; review test configuration")
    init = init[:idts[-1].end()] + "    lab3_test_exceptions();\n" + init[idts[-1].end():]
    init = init.replace(includes, includes + "\n#include <lab3_exception_test.h>")
    makefile = (source / "Makefile").read_text(encoding="utf-8")
    anchor = "KCFLAGS\t\t+= $(addprefix -I,$(KINCLUDE))"
    if makefile.count(anchor) != 1:
        parser.error("Makefile include anchor changed")
    makefile = makefile.replace(anchor, "KINCLUDE += kern/test\nKSRCDIR += kern/test\n\n" + anchor)

    if args.mode == "context":
        signature = "void trap(struct trapframe *tf) {"
        dispatch = "    trap_dispatch(tf);"
        if trap.count(signature) != 1 or trap.count(dispatch) != 1:
            parser.error("trap observer anchors changed")
        trap = trap.replace(signature,
                            "extern void hyc_check_frame(struct trapframe *tf);\n"
                            "extern void hyc_check_return(struct trapframe *tf);\n"
                            + signature + "\n    hyc_check_frame(tf);")
        trap = trap.replace(dispatch, dispatch + "\n    hyc_check_return(tf);")

    # Validate all anchors before creating the disposable copy; never remove directories.
    shutil.copytree(source, build, ignore=shutil.ignore_patterns("obj", "bin", ".qemu*"))
    testdir = build / "kern/test"
    testdir.mkdir()
    fixtures = support / "fixtures"
    for name in ("lab3_exception_test.h", "probes.S"):
        shutil.copyfile(fixtures / name, testdir / name)
    driver = "lab3_exception_test.c" if args.mode == "context" else "exception_integration.c"
    shutil.copyfile(fixtures / driver, testdir / "lab3_exception_test.c")
    if args.mode == "context":
        shutil.copyfile(fixtures / "hyc_edit.S", testdir / "hyc_edit.S")
    stub = not re.search(r"void\s+interrupt_handler\s*\([^)]*\)\s*\{", trap)
    if stub:
        shutil.copyfile(fixtures / "interrupt-stub.c", testdir / "interrupt-stub.c")
    (build / "Makefile").write_text(makefile, encoding="utf-8")
    (build / "kern/init/init.c").write_text(init, encoding="utf-8")
    (build / "kern/trap/trap.c").write_text(trap, encoding="utf-8")
    copied = (build / "kern/trap/trapentry.S").read_bytes()
    assert copied == entry
    print("MODE=" + args.mode)
    print("BUILD=" + str(build))
    print("TRAPENTRY_SHA256=" + hashlib.sha256(copied).hexdigest())
    print("REFERENCE_ENTRY_USED=no")
    print("INTERRUPT_STUB=" + ("unexpected-interrupt-panic" if stub else "not-needed"))
    print("Submission sources and grading files were not modified.")


if __name__ == "__main__":
    main()
