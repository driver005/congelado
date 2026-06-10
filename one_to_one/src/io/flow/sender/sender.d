module io.flow.sender.sender;
@nogc nothrow:

// Umbrella re-export for the sender sub-modules.
// PORT-NOTE: C++ used partition imports (export import :async; export import :sync;).

public import io.flow.sender.async_;
public import io.flow.sender.sync;
