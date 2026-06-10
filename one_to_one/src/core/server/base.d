module core.server.base;
@nogc nothrow:

// Umbrella re-export — mirrors C++ `export import :router; ... :server;`
public import core.server.router;
public import core.server.builder;
public import core.server.handler;
public import core.server.middleware;
public import core.server.types;
public import core.server.server;
