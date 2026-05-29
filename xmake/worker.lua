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

for _, workerdir in ipairs(os.dirs("workers/*")) do
    local name = path.basename(workerdir)
    local files = os.files(path.join(workerdir, "**.cc"))
    if #files > 0 then
        worker(name, table.unpack(files))
    end
end
