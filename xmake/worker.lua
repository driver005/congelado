-- Worker targets: compiled against congelado_lib, register tasks via CONGELADO_TASK(T).
-- Bundle mode:   worker("payments_worker", "tasks/payment.cc", "tasks/refund.cc")
-- Per-task mode: worker("email_worker",    "tasks/email.cc")
rule("congelado.worker")
on_load(function(target)
	target:add("deps", "congelado_lib")
	target:add("files", "$(projectdir)/src/worker_main.cc")
end)
rule_end()

function worker(name, ...)
	target(name)
	set_kind("binary")
	set_languages("c++26")
	set_policy("build.c++.modules", true)
	add_rules("congelado.worker")
	add_files(...)
	add_includedirs("include")
	if is_plat("linux", "macosx") then
		add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
	end
	target_end()
end

local workers = {}
for _, f in ipairs(os.files(path.join(os.projectdir(), "workers/**/*.cc"))) do
    local name = path.basename(path.directory(f))
    workers[name] = workers[name] or {}
    table.insert(workers[name], f)
end
for name, files in pairs(workers) do
    worker(name, table.unpack(files))
end
