# Token pipeline (extract → normalize → generate → verify)

The entire token surface of `congelado_hero_ui` is generated from pinned
sources — never hand-copied.

## Stages

| Stage | Script | Input | Output |
| --- | --- | --- | --- |
| Extract | `extract_tokens.py` | `sources/heroui-variables.css` (pinned `@heroui/styles@3.2.4`) | `build/raw-tokens.json` (gitignored) |
| Normalize | `normalize_tokens.py` | raw tokens + `authored/hero-component-tokens.json` | `tokens/hero-tokens.normalized.json` (COMMITTED) |
| Generate | `generate_tokens.py` | normalized snapshot | `../lib/src/tokens/generated/hero_tokens.g.dart` (COMMITTED) |
| Verify | `verify_generated.py` | everything above | pass/fail (read-only) |

## Run

```bash
cd flutter/ui

# Read-only check (CI-ready; pure Python stdlib, no network, no installs):
python3 tool/verify_generated.py

# Full regeneration:
python3 tool/extract_tokens.py
python3 tool/normalize_tokens.py
python3 tool/generate_tokens.py
```

`verify_generated.py` re-runs extract + normalize in a scratch copy of `tool/`
(sans `build/`) and regenerates the Dart into a temp dir, then diffs both
against the committed artifacts byte-for-byte. It also scans the generated
file for provenance headers, unparsed CSS units, and invalid colors.

## Sources

Pinned artifacts live in `sources/` with hashes in `sources/SHA256SUMS`:

- `@heroui/styles@3.2.4` npm tarball:
  - `themes/default/variables.css` — light/dark role colors, shadows,
    radius base, spacing, durations (tier-1 extraction);
  - `themes/shared/theme.css` — radius scale, easing curves;
  - `dist/components/*.css` — component anatomy (tier-2/3 transcription,
    cited in `authored/hero-component-tokens.json`).

## Upgrade procedure

1. Re-download the pinned tarball (or bump the version) and refresh
   `sources/*.css` + `SHA256SUMS`.
2. Re-run the four stages.
3. Review the diffs: `tokens/hero-tokens.normalized.json` and
   `lib/src/tokens/generated/hero_tokens.g.dart`.
4. Update inventory assertions in `tests` if the token surface grew, and
   `HeroSourceManifest` counts follow automatically.

## Determinism rules

- `stableStringify` (deep-sorted keys), floats rounded to 4 decimals;
- colors emitted as `0xAARRGGBB` (OKLCH and `color-mix(in oklab, …)` are
  resolved to sRGB at extract time — the exact CSS Color 4/5 math, including
  premultiplied-alpha interpolation and remainder-weight inference);
- no clock, no randomness, no network at generate/verify time.
