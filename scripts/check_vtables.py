#!/usr/bin/env python3
"""Compare C vtable structs with builder get_generic_vtable() initializers."""
import re
import sys
from pathlib import Path

ROOT = Path("/home/default/cc/congelado")

# builder cppm -> (C header, struct name to extract)
MODULES = {
    "include/cc/abi/builder/manager/manager.cppm": ("include/c/extern/manager/manager.h", "TF_WorkerManager"),
    "include/cc/abi/builder/search/search.cppm": ("include/c/extern/search/search.h", "TF_Search"),
    "include/cc/abi/builder/events/events.cppm": ("include/c/extern/events/events.h", "TF_Events"),
    "include/cc/abi/builder/worker/worker.cppm": ("include/c/extern/worker/worker.h", "TF_Worker"),
    "include/cc/abi/builder/serde/serde.cppm": ("include/c/extern/serde/serde.h", "TF_Serde"),
    "include/cc/abi/builder/payload/payload.cppm": ("include/c/extern/payload/payload.h", "TF_Payload"),
    "include/cc/abi/builder/protocol/protocol.cppm": ("include/c/extern/protocol/protocol.h", "TF_Protocol"),
    "include/cc/abi/builder/otel/otel.cppm": ("include/c/extern/otel/otel.h", "TF_Otel"),
    "include/cc/abi/builder/filesystem/filesystem.cppm": ("include/c/extern/filesystem/filesystem.h", "TF_Filesystem"),
    "include/cc/abi/builder/profiler/profiler.cppm": ("include/c/extern/profiler/profiler.h", "TF_Profiler"),
    "include/cc/abi/builder/cache/cache.cppm": ("include/c/extern/cache/cache.h", "TF_Cache"),
    "include/cc/abi/builder/logger/logger.cppm": ("include/c/extern/logger/logger.h", "TF_Logger"),
    "include/cc/abi/builder/orchestrator/orchestrator.cppm": ("include/c/extern/orchestrator/orchestrator.h", "TF_Orchestrator"),
    "include/cc/abi/builder/database/database.cppm": ("include/c/extern/database/database.h", "TF_Database"),
    "include/cc/abi/builder/intern/tensor.cppm": ("include/c/intern/tf_tensor.h", "TF_Tensor"),
    "include/cc/abi/builder/intern/buffer.cppm": ("include/c/intern/tf_buffer.h", "TF_Buffer"),
    "include/cc/abi/builder/cron/cron.cppm": ("include/c/extern/cron/cron.h", "TF_Cron"),
    "include/cc/abi/builder/intern/shape.cppm": ("include/c/intern/tf_shape.h", "TF_Shape"),
    "include/cc/abi/builder/intern/datatype.cppm": ("include/c/intern/tf_datatype.h", "TF_DataType"),
    "include/cc/abi/builder/io/io.cppm": ("include/c/extern/io/io.h", "TF_IO"),
    "include/cc/abi/builder/generator/generator.cppm": ("include/c/extern/generator/generator.h", "TF_Generator"),
}

COMMENT_RE = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
PRAGMA_RE = re.compile(r"#\s*if|#\s*endif|#\s*else|#\s*define")


def strip_comments(text):
    return COMMENT_RE.sub("", text)


def extract_struct_fields(header_text, struct_name):
    """Extract member names of `typedef struct NAME { ... } NAME;`."""
    text = strip_comments(header_text)
    # find the typedef struct struct_name {
    pat = re.compile(r"typedef\s+struct\s+" + re.escape(struct_name) + r"\s*\{")
    m = pat.search(text)
    if not m:
        return None
    start = m.end()
    # find matching closing brace at nesting depth 0
    depth = 1
    i = start
    while i < len(text) and depth > 0:
        c = text[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
        i += 1
    body = text[start : i - 1]
    fields = []
    # strip any preprocessor conditionals inside the body
    body = "\n".join(
        ln for ln in body.splitlines() if not PRAGMA_RE.search(ln)
    )
    for stmt in body.split(";"):
        stmt = stmt.strip()
        if not stmt:
            continue
        # function pointer member: type (*name)(...)
        m2 = re.search(r"\(\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)", stmt)
        if m2:
            fields.append(m2.group(1))
            continue
        # plain member (possibly array/bitfield): capture last identifier
        m3 = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^\]]*\])?\s*:\s*\d+\s*$", stmt)
        if m3:
            fields.append(m3.group(1))
            continue
        m4 = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^\]]*\])?\s*$", stmt)
        if m4:
            fields.append(m4.group(1))
            continue
        fields.append("<parse-error: " + stmt[:50] + ">")
    return fields


def extract_init_fields(cppm_text, struct_ret):
    """Extract designated-initializer member names from get_generic_vtable body."""
    text = strip_comments(cppm_text)
    pat = re.compile(r"get_generic_vtable\s*\(\s*\)")
    m = pat.search(text)
    if not m:
        return None
    # from the return type name backwards... simpler: find the opening brace after
    # "get_generic_vtable()" and the matching close at depth 0. The body contains
    # `static TF_X vtable = { ... };` plus `return &vtable;`
    brace = text.find("{", m.end())
    depth = 1
    i = brace + 1
    while i < len(text) and depth > 0:
        c = text[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
        i += 1
    body = text[brace + 1 : i - 1]
    fields = re.findall(r"(?:^|[{,])\s*\.([A-Za-z_][A-Za-z0-9_]*)\s*=", body, re.M)
    return fields


def extract_positional_elements(cppm_text, struct_ret):
    """For positional (non-designated) initializers: return the top-level elements.

    Each element is returned as a (kind, snippet) pair where kind is "sizeof" or
    "lambda" or "other". Top-level = brace depth 1 relative to the vtable's own
    `{`, ignoring commas inside parens and nested braces.
    """
    text = strip_comments(cppm_text)
    pat = re.compile(r"get_generic_vtable\s*\(\s*\)")
    m = pat.search(text)
    if not m:
        return None
    # locate the `static ... vtable = {` initializer: first '{' that is not the
    # function body's own '{'. Walk from the get_generic_vtable '(' to the first
    # "= {" after it.
    eq = text.find("= {", m.end())
    if eq < 0:
        return None
    brace = text.find("{", eq)
    depth = 0
    paren = 0
    j = brace
    seps = []
    while j < len(text):
        c = text[j]
        if c == "(":
            paren += 1
        elif c == ")":
            paren -= 1
        elif c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                break
        elif c == "," and depth == 1 and paren == 0:
            seps.append(j)
        j += 1
    bounds = [brace] + seps + [j]
    elements = []
    for k in range(len(bounds) - 1):
        seg = text[bounds[k] : bounds[k + 1]].strip()
        # segments are delimited by commas/braces — shed the delimiter itself
        seg = seg.lstrip("{").lstrip(",").strip()
        if not seg:
            continue
        if re.match(r"sizeof\s*\(", seg):
            elements.append(("sizeof", seg))
        elif re.match(r"\[\]\s*\(", seg):
            elements.append(("lambda", seg))
        else:
            elements.append(("other", seg))
    return elements


def main():
    ok = True
    for cppm_rel, (hdr_rel, struct_name) in MODULES.items():
        cppm = ROOT / cppm_rel
        hdr = ROOT / hdr_rel
        if not cppm.exists():
            print(f"[MISSING FILE] {cppm_rel}")
            ok = False
            continue
        if not hdr.exists():
            print(f"[MISSING FILE] {hdr_rel}")
            ok = False
            continue
        struct_fields = extract_struct_fields(hdr.read_text(), struct_name)
        if struct_fields is None:
            print(f"[ERROR] could not parse struct {struct_name} in {hdr_rel}")
            ok = False
            continue
        init_fields = extract_init_fields(cppm.read_text(), struct_name)
        if init_fields is None:
            print(f"[ERROR] could not parse get_generic_vtable in {cppm_rel}")
            ok = False
            continue
        if init_fields:
            # designated-initializer style
            missing = [f for f in struct_fields if f not in init_fields]
            extra = [f for f in init_fields if f not in struct_fields]
            dupes = [f for f in set(init_fields) if init_fields.count(f) > 1]
            status = "OK" if not missing and not extra and not dupes else "ISSUES"
            if status == "ISSUES":
                ok = False
            print(f"[{status}] {cppm_rel}")
            print(
                f"    struct {struct_name}: {len(struct_fields)} fields, designated init: {len(init_fields)}"
            )
            if missing:
                print(f"    MISSING IN INIT (nullptr at runtime!): {missing}")
            if extra:
                print(f"    INITIALIZED BUT NOT IN STRUCT: {extra}")
            if dupes:
                print(f"    DUPLICATE INITIALIZERS: {dupes}")
            continue

        # positional-initializer style: every struct field must be covered in order
        elements = extract_positional_elements(cppm.read_text(), struct_name)
        if elements is None:
            print(f"[ERROR] could not parse positional init in {cppm_rel}")
            ok = False
            continue
        kinds = [k for k, _ in elements]
        issues = []
        if len(elements) != len(struct_fields):
            issues.append(
                f"count mismatch: {len(elements)} initializers vs {len(struct_fields)} fields"
            )
        if kinds and kinds[0] != "sizeof":
            issues.append(f"first element is not sizeof(...): {kinds[0]!r}")
        bad = [i for i, k in enumerate(kinds) if i > 0 and k != "lambda"]
        if bad:
            issues.append(f"non-lambda initializers at positions {bad}")
        if not issues:
            status = "OK"
        else:
            status = "ISSUES"
            ok = False
        print(f"[{status}] {cppm_rel}")
        print(
            f"    struct {struct_name}: {len(struct_fields)} fields, positional init: {len(elements)} ({kinds})"
        )
        for issue in issues:
            print(f"    {issue}")
    print()
    print("ALL OK" if ok else "ISSUES FOUND")


if __name__ == "__main__":
    main()
