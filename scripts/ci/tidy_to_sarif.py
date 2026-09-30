#!/usr/bin/env python3
"""Converts clang-tidy text output to SARIF 2.1.0 for GitHub code scanning."""

import json
import os
import re
import sys


class TidySarifConverter:
    PATTERN = re.compile(r"^(?P<file>[^:\s]+):(?P<line>\d+):(?P<column>\d+): (?P<level>warning|error): (?P<message>.*?) \[(?P<rule>[^\]]+)\]$")

    @staticmethod
    def convert(input_path, output_path):
        workspace = os.getcwd() + "/"
        results = []
        rules = {}

        with open(input_path, encoding="utf-8", errors="replace") as handle:
            for line in handle:
                match = TidySarifConverter.PATTERN.match(line.strip())
                if not match:
                    continue

                rule = match["rule"].split(",")[0]
                rules.setdefault(rule, {"id": rule, "helpUri": f"https://clang.llvm.org/extra/clang-tidy/checks/list.html#{rule}"})
                results.append({
                    "ruleId": rule,
                    "level": "error" if match["level"] == "error" else "warning",
                    "message": {"text": match["message"]},
                    "locations": [{
                        "physicalLocation": {
                            "artifactLocation": {"uri": match["file"].removeprefix(workspace)},
                            "region": {"startLine": int(match["line"]), "startColumn": int(match["column"])},
                        }
                    }],
                })

        sarif = {
            "version": "2.1.0",
            "$schema": "https://json.schemastore.org/sarif-2.1.0.json",
            "runs": [{"tool": {"driver": {"name": "clang-tidy", "rules": list(rules.values())}}, "results": results}],
        }
        with open(output_path, "w", encoding="utf-8") as handle:
            json.dump(sarif, handle, indent=2)


if __name__ == "__main__":
    TidySarifConverter.convert(sys.argv[1], sys.argv[2])
