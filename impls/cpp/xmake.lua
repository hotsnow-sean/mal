add_rules("mode.debug", "mode.release")

set_languages("c++20")
add_requires("fmt", "linenoise")
add_rules("plugin.compile_commands.autoupdate")
add_rules("postcpp")

rule("postcpp")
  after_link(function(target)
    os.cp(target:targetfile(), ".")
  end)
  after_clean(function(target)
    os.tryrm(target:name())
  end)

target("step0_repl")
  set_kind("binary")
  add_files("src/step0_repl.cpp")
  add_packages("linenoise", "fmt")

target("step1_read_print")
  set_kind("binary")
  add_files("src/step1_read_print.cpp", "src/types.cpp", "src/reader.cpp", "src/printer.cpp")
  add_packages("linenoise", "fmt")
