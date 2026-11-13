"""Search-or-download plumbing shared by the SDK repository rules (SYCL, PTI, CUPTI, rocprofiler).

Every rule follows the same two routes:
  1. search: an explicit environment variable first, then well known install paths;
  2. download: a pinned, checksummed archive when nothing usable was found.
A route-1 hit is symlinked into the repository so the BUILD file is identical for both routes.
"""

def host_arch(ctx):
    """Normalised host architecture: "x86_64", "aarch64" or the raw value."""
    arch = ctx.os.arch
    if arch in ("amd64", "x86_64"):
        return "x86_64"
    if arch in ("aarch64", "arm64"):
        return "aarch64"
    return arch

def search_roots(ctx, env_names, fixed, parents, suffixes = [""]):
    """Candidate install roots, most specific first.

    Args:
      ctx: repository_ctx.
      env_names: environment variables that point at an install.
      fixed: absolute paths tried as they are.
      parents: directories whose children are tried newest name first (versioned installs).
      suffixes: appended to every candidate (for example "/compiler/latest" under an ONEAPI_ROOT).
    """
    roots = []
    for name in env_names:
        value = ctx.getenv(name)
        if value:
            for suffix in suffixes:
                roots.append(value + suffix)
    roots.extend(fixed)
    for parent in parents:
        directory = ctx.path(parent)
        if directory.exists:
            roots.extend(sorted([str(child) for child in directory.readdir()], reverse = True))
    return roots

def first_root(ctx, roots, all_of, any_of = []):
    """First root holding every `all_of` file and, if given, at least one `any_of` file."""
    for root in roots:
        if not all([ctx.path(root + "/" + marker).exists for marker in all_of]):
            continue
        if any_of and not any([ctx.path(root + "/" + marker).exists for marker in any_of]):
            continue
        return root
    return None

def link_existing(ctx, root, names):
    """Symlinks the listed children of `root` into the repository when they exist."""
    for name in names:
        source = ctx.path(root + "/" + name)
        if source.exists:
            ctx.symlink(source, name)

def write_status(ctx, sdk, source, detail):
    """STATUS.md records which route produced the repository, for humans and for `bazel query`."""
    ctx.file(
        "STATUS.md",
        "# {}\n\n- route: {}\n- detail: {}\n".format(sdk, source, detail),
    )
    ctx.file(
        "config.bzl",
        'SOURCE = "{}"\nDETAIL = "{}"\n'.format(source, detail),
    )

def missing(ctx, sdk, build_file, how_to_install):
    """Neither route worked: an empty but valid repository, so unrelated targets still build."""
    # buildifier: disable=print
    print("{}: not found and no download for this host; {}".format(sdk, how_to_install))
    write_status(ctx, sdk, "missing", how_to_install)
    ctx.file("BUILD.bazel", build_file)
