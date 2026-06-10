module io.flow.socket.socket;
@nogc nothrow:

// Umbrella re-export for socket sub-modules.
// PORT-NOTE: C++ only exported :sync (async was empty).

public import io.flow.socket.sync;
public import io.flow.socket.async_;
