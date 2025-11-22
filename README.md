_The AstraNav probe is set to launch to chart methane plumes on Titan, but its core software stack is a relic: a 25‑year‑old C/C++ navigation binary bolted to a PHP/SQLite “mission console” hastily built during a funding sprint. A single crash or forged packet mid‑flight could waste fuel or disable critical systems, so Command assigns you to harden both layers in the narrow window before rollout._

_First, you tackle the flight binary. Using coverage‑guided fuzzing, you provoke crashes, capture minimal reproducible inputs, and patch at least six memory issues._

_Next, you audit the PHP console used by ground crew to upload maneuvers and read telemetry. You document five distinct exploits showing how each could misalign antennas or leak scientific data, then propose precise fixes._


## Problem 1: Hardening the mini-proto Library

You are given `p1/mini_proto.c` and `p1/mini_proto.h`, a tiny record-serialization library intentionally seeded with memory-unsafety.

### Goals

* Identify unsafe behavior in the library (stack/heap overflows, use-after-free, double free, format-string issues, integer overflow, etc.).
* For each issue, explain what kind of input triggers it **before** your fix.
* Patch **at least six distinct bugs** (each with a different root cause). After fixing six, you may stop; you are not required to prove there are no additional bugs.
* Fuzzing is optional; however, performing it can earn up to **25% bonus** for this component of the project. The bonus applies only to Project 2.

### Provided API & Build

`mini_proto.h` exposes three public functions:

```cy
size_t encode_record(const record_t *rec, uint8_t *out_buf, size_t out_cap);
size_t decode_record(const uint8_t *buf, size_t len, record_t **out);
void   free_record(record_t *rec);
```

Run `make` to build a static library (`libmini_proto.a`) and a demo program (`demo.bin`).  
Run `demo.bin` to observe default behavior.

If you fuzz, link `libmini_proto.a` into your harness (libFuzzer, AFL++, honggfuzz, etc.).

To enable sanitizers:

```bash
make SAN=1
```

### Recommended Workflow

* Read the code first - several bugs are visible on inspection.
* Use a decode-focused test harness (see `p1/demo.c`).
* If fuzzing, use `libmini_proto.a` as your target; triage with  
  `-fsanitize=address,undefined`.
* Save every crashing input **before** patching so you can demonstrate the exploit and the fix.

### Deliverables

Do **not** modify `p1/mini_proto.c` or `p1/mini_proto.h` directly in your submission.  
Place all artifacts under `p1/artifacts/`:

```
p1/artifacts/
  bugN/
    mini_proto.patch
  fuzzing/         # only if you used fuzzing
    report.md      # fuzzing setup, commands, seeds, minimized crashes, etc.
  report.md or report.pdf
```

Submit **only diff patches** for each fix, not full files.  
Generate a patch with:

```bash
diff -u original.c fixed.c > fixed.patch
```

### Bug Documentation Template

In `p1/artifacts/report.md`, repeat the block below for each bug:

```
### bugNN - <short title>
- Category: <heap overflow / integer overflow / use-after-free / etc.>
(You may reuse the same category across bugs, as long as each bug comes from a different root-cause location in the code)
- Trigger: <brief explanation or example input>
- Root cause: <2–3 sentences explaining the issue>
- Fix: <2–3 sentences explaining the patch and why it works>
```

You may add extra notes or links, but keep the format consistent so the grader can check your fixes quickly.

## Problem 2: Vulnerable Console 

This module provides a UI wrapper over legacy PHP code intended for hands‑on vulnerability discovery. Although modern styling hides it, the backend still trusts unvalidated input and leaks sensitive data. Your objective is to examine the code and interface, identify security flaws, and document each exploit clearly and concisely.

### Objectives

- Identify **five distinct vulnerabilities** (not necessarily from different categories, but occurring in different parts of the code).
- Provide a proof that you've exploited each one of the vulnerabilities (screenshots or request logs allowed).
- Explain the root cause and propose practical mitigation steps.
- Implement a solution patch to resolve the vulnerabilities.

### Docker 

This part is containerized using Docker. Docker is a platform that lets you package applications into lightweight, portable containers so they run the same way on any system.

To work with the code and use Docker, the simplest option is to work on Linux, where Docker is fully supported and easy to set up. If you are not already on Linux, consider running Ubuntu (a common distribution of Linux) inside a virtual machine. Docker is also available on Windows through WSL and on macOS through Docker Desktop.

To install docker check out `https://docs.docker.com/engine/install/`

### Run

```bash
cd p2
docker build -t vuln-app .
docker run -d -p 9090:80 vuln-app
```


Visit `http://localhost:9090/` and try to exploit it.

For shell access:

```bash
docker ps
docker exec -it <id> bash
```

### Submission Structure

```
p2/artifacts/
  exploits/
    vulN/
      fixed.patch # patched codebase (only the differences)
      report.md   # or .pdf
      evidence/
        vuln.png  # screenshot
```

Each `report.md` file:

```
### report.md
- Category: <type>
- Root cause: <short explanation w/ code reference>
- Fix: <mitigation>
```

Keep write‑ups minimal, direct, and technically precise.
