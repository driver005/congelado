-- Part of the merged plugins/ project — see plugins/xmake.lua. The `auth` app: a bundle of task
-- workers (IWorker plugins) plus its workflow/task def files. The workers build into build/workers
-- like any other worker plugin; the def files get copied into build/apps/auth/{taskdefs,workflows}
-- so the worker host (sdk/worker/worker_main.cc) finds and registers them with the engine on load.
-- The workers stage this app's def files (see workers/jwt_sign/xmake.lua's after_build) next to the
-- build output so the worker host reads them the same way under local `xmake run` and the docker
-- image (both resolve the apps dir relative to the binary).
includes("workers/pw_hash")
includes("workers/jwt_sign")
includes("workers/db_query")
