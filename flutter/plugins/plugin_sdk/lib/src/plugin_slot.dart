/// Named mount points inside the host UI. A plugin declares which slot it
/// targets; [PluginHost] instances filter by slot.
enum PluginSlot {
  /// Primary content area.
  main,

  /// Sidebar / rail area.
  sidebar,

  /// Top toolbar / header strip.
  toolbar,

  /// Bottom status strip.
  footer,
}
