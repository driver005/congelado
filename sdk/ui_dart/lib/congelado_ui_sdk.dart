library congelado_ui_sdk;

export 'src/plugin_ui_contribution.dart';
export 'src/status_mapping.dart';
export 'src/theme.dart';

// Design components — Flutter equivalents of Medusa UI's component set
// (see sdk/ui_dart/lib/src/components/ for the per-component "which Medusa
// component this maps to" notes). Components that map 1:1 onto an existing
// Forui widget (alert, avatar, badge, button, calendar, checkbox,
// date-picker, divider, drawer, dropdown-menu, input, label, otp-input,
// popover, radio-group, select, skeleton, switch, tabs, textarea,
// time-input, toast, tooltip) are deliberately NOT wrapped here — just use
// the Forui widget directly, a passthrough wrapper would add nothing.
export 'src/components/code.dart';
export 'src/components/container.dart';
export 'src/components/copy_button.dart';
export 'src/components/currency_input.dart';
export 'src/components/data_table/data_table.dart';
export 'src/components/execution_timeline.dart';
export 'src/components/focus_modal.dart';
export 'src/components/hint.dart';
export 'src/components/icon_badge.dart';
export 'src/components/inline_tip.dart';
export 'src/components/kbd.dart';
export 'src/components/key_value_editor.dart';
export 'src/components/page_shell.dart';
export 'src/components/progress_accordion.dart';
export 'src/components/progress_tabs.dart';
export 'src/components/prompt.dart';
export 'src/components/status_badge.dart';
export 'src/components/toaster_scope.dart';
export 'src/components/typography.dart';
export 'src/components/workflow_graph.dart';
