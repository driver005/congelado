/// Bundled spec plugins (web fallback — no filesystem). Mirrors the
/// `plugin.yaml` files in flutter/plugins/; keep in sync when those change.
const Map<String, String> bundledPluginSpecs = {
  'example_hello': '''
id: example_hello
name: Hello Plugin
version: 0.1.0
author: Congelado Team
description: A greeting card loaded from plugin.yaml at runtime — the minimal spec plugin.
tags: [demo, greeting, main]
slot: main
widgets:
  type: card
  props:
    title: Hello from a runtime plugin
  children:
    - type: text
      props:
        text: Bundled spec fallback (web has no filesystem).
    - type: button
      props:
        label: Say hi
        action: hello.greet
''',
  'example_metrics': '''
id: example_metrics
name: Metrics Widget
version: 0.1.0
author: Congelado Team
description: Engine metrics snapshot for the sidebar slot, declared as a spec plugin.
tags: [metrics, sidebar, demo]
slot: sidebar
widgets:
  type: card
  props:
    title: Engine metrics
  children:
    - type: text
      props:
        text: 'Requests: 1,284'
    - type: text
      props:
        text: 'Latency p95: 42ms'
        color: green
''',
};
