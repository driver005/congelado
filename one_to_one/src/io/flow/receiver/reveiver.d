module io.flow.receiver.reveiver;
@nogc nothrow:

// Umbrella re-export for the receiver sub-modules.
// PORT-NOTE: C++ used partition imports (export import :async; export import :sync;).
// D public imports achieve the same visibility.

public import io.flow.receiver.async_;
public import io.flow.receiver.sync;
