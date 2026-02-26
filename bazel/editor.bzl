load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

def setup_editor():
  refresh_compile_commands(
      name = "refresh_editor",
      targets = {
          "//:congelado_exec": "--features=-header_modules", 
      },
      exclude_external_sources = True, 
  )
