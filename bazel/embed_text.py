#!/usr/bin/env python3
"""Embeds text files as std::string_view constants in a generated C++ header."""

import json
import os
import re
import sys


def to_identifier(path: str) -> str:
    stem = os.path.splitext(os.path.basename(path))[0]
    return re.sub(r"[^0-9A-Za-z_]", "_", stem)


def main() -> None:
    output_path = sys.argv[1]
    namespace = "cc_templating_generated"
    input_paths = []

    for arg in sys.argv[2:]:
        if arg.startswith("--namespace="):
            namespace = arg[len("--namespace="):]
        else:
            input_paths.append(arg)

    lines = [
        "#pragma once",
        "",
        "#include <string_view>",
        "",
        f"namespace {namespace} {{",
        "",
    ]

    for input_path in input_paths:
        with open(input_path, "r", encoding="utf-8") as input_file:
            content = input_file.read()

        identifier = to_identifier(input_path)
        lines.append(f"inline constexpr std::string_view k_{identifier} = {json.dumps(content)};")

    lines.append("")
    lines.append(f"}} // namespace {namespace}")
    lines.append("")

    with open(output_path, "w", encoding="utf-8") as output_file:
        output_file.write("\n".join(lines))


if __name__ == "__main__":
    main()
