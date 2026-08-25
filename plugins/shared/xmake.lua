-- plugins/shared: code shared across plugins (engine, worker_manager, ...). moduleonly targets so
-- several independent consumers can each compile the shared modules without the BMI-cache
-- double-credit bug that bites two sibling targets globbing the same .cppm (see the note on the old
-- worker_module target). First occupant: the `model` module (was plugins/engine/model), consumed by
-- both the engine and the manager/worker_external plugin.
target("shared_model")
set_kind("moduleonly")
-- -fPIC so these objects can link into shared plugin .so consumers (engine, worker_manager) —
-- moduleonly skips apply_common_layer_settings where other layers get -fPIC.
add_cxflags("-fPIC")
add_deps("congelado_sdk")
add_files("model/**.cppm", { public = true })
target_end()

-- shared_model_test: recompiles model/**.cppm with CONGELADO_TEST defined so each file's
-- inline boost::ut suite gets built. See apply_test_target in xmake/common.lua.
apply_test_target({
	name = "shared_model",
	deps = { "congelado_sdk" },
	files = { "model/**.cppm" },
})
