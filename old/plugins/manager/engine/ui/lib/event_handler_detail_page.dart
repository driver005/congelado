import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';

// Verified against Forui 0.25.0's real source: FTextFormField/FTextField's `controller`/
// `initialValue`/`onChange` moved into a `control:` FTextFieldControl object; FSelect now takes
// `items: Map<String, T>` + `control: FSelectControl<T>` instead of `format`/`initialValue`/
// `children: [FSelectItem(...)]`; FButton takes `variant:` (FButtonVariant enum) directly
// instead of `style: FButtonStyle.outline`.

const _actionTypes = [
  'START_WORKFLOW',
  'COMPLETE_TASK',
  'FAIL_TASK',
  'TERMINATE_WORKFLOW',
  'UPDATE_WORKFLOW_VARIABLES',
];

/// Expected `payload` keys per `EventActionType`, straight from
/// `plugins/engine/src/model/event/handler.cppm`'s own doc comment — shown
/// as a hint under the type picker rather than five bespoke sub-forms, since
/// the backend itself never validates payload keys either (its own
/// `validate()` only checks `name`/`event` are non-empty).
String _payloadHintFor(String type) {
  switch (type) {
    case 'START_WORKFLOW':
      return 'Expected keys: workflow_name, plus any variables to seed it with.';
    case 'COMPLETE_TASK':
    case 'FAIL_TASK':
      return 'Expected keys: exec_id, task_ref, plus any output_data to attach.';
    case 'TERMINATE_WORKFLOW':
      return 'Expected keys: exec_id, optional status (COMPLETED/FAILED/TIMED_OUT/TERMINATED).';
    case 'UPDATE_WORKFLOW_VARIABLES':
      return 'Expected keys: exec_id, plus any variables to merge in.';
    default:
      return '';
  }
}

class _ActionRow {
  _ActionRow({this.type = 'START_WORKFLOW', Map<String, String>? payload})
      : payload = payload ?? {};

  String type;
  Map<String, String> payload;

  Map<String, dynamic> toJson() => {'type': type, 'payload': payload};

  static _ActionRow fromJson(Map<String, dynamic> json) => _ActionRow(
        type: json['type'] as String? ?? 'START_WORKFLOW',
        payload: (json['payload'] as Map<String, dynamic>? ?? const {})
            .map((key, value) => MapEntry(key, '$value')),
      );
}

/// Create/edit form for a single `EventHandler`, bound by hand to its known
/// JSON shape (`name`, `event`, `condition`, `actions[]`, `active` — see
/// `plugins/engine/src/model/event/handler.cppm`). `name == null` means
/// create mode; pushed from [EventHandlersListPage].
class EventHandlerDetailPage extends StatefulWidget {
  const EventHandlerDetailPage({super.key, required this.name});

  /// Existing handler's name, or `null` to create a new one.
  final String? name;

  @override
  State<EventHandlerDetailPage> createState() => _EventHandlerDetailPageState();
}

class _EventHandlerDetailPageState extends State<EventHandlerDetailPage> {
  final _api = EngineApiClient();
  final _formKey = GlobalKey<FormState>();
  final _nameController = TextEditingController();
  final _eventController = TextEditingController();
  final _conditionController = TextEditingController();
  bool _active = true;
  List<_ActionRow> _actions = [];
  bool _loading = false;
  bool _saving = false;

  bool get _isCreate => widget.name == null;

  @override
  void initState() {
    super.initState();
    if (!_isCreate) _load();
  }

  Future<void> _load() async {
    setState(() => _loading = true);
    try {
      final handler = await _api.getEventHandler(widget.name!);
      _nameController.text = handler['name'] as String? ?? '';
      _eventController.text = handler['event'] as String? ?? '';
      _conditionController.text = handler['condition'] as String? ?? '';
      _active = handler['active'] as bool? ?? true;
      _actions = [
        for (final action in (handler['actions'] as List<dynamic>? ?? const []))
          _ActionRow.fromJson(action as Map<String, dynamic>),
      ];
      if (mounted) setState(() {});
    } finally {
      if (mounted) setState(() => _loading = false);
    }
  }

  @override
  void dispose() {
    _nameController.dispose();
    _eventController.dispose();
    _conditionController.dispose();
    super.dispose();
  }

  Future<void> _save() async {
    if (!_formKey.currentState!.validate()) return;
    setState(() => _saving = true);
    try {
      final body = {
        'name': _nameController.text,
        'event': _eventController.text,
        'condition': _conditionController.text.isEmpty ? null : _conditionController.text,
        'active': _active,
        'actions': [for (final action in _actions) action.toJson()],
      };
      if (_isCreate) {
        await _api.createEventHandler(body);
      } else {
        await _api.updateEventHandler(widget.name!, body);
      }
      if (mounted) Navigator.of(context).pop(true);
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text('Save failed: $e')));
    } finally {
      if (mounted) setState(() => _saving = false);
    }
  }

  Future<void> _delete() async {
    final confirmed = await showCPrompt(
      context,
      title: 'Delete event handler?',
      description: 'This removes the "${widget.name}" event handler.',
      confirmLabel: 'Delete',
    );
    if (!confirmed) return;
    try {
      await _api.deleteEventHandler(widget.name!);
      if (mounted) Navigator.of(context).pop(true);
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text('Delete failed: $e')));
    }
  }

  Widget _buildActionRow(int index) {
    final action = _actions[index];
    return CContainer(
      padding: const EdgeInsets.all(12),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        mainAxisSize: MainAxisSize.min,
        children: [
          Row(
            children: [
              Expanded(
                child: FSelect<String>(
                  items: {for (final type in _actionTypes) type: type},
                  control: FSelectControl<String>.managed(
                    initial: action.type,
                    onChange: (value) => setState(() => action.type = value ?? action.type),
                  ),
                ),
              ),
              FButton.icon(
                variant: FButtonVariant.outline,
                onPress: () => setState(() => _actions.removeAt(index)),
                child: const Icon(Icons.close),
              ),
            ],
          ),
          const SizedBox(height: 4),
          CHint(_payloadHintFor(action.type)),
          const SizedBox(height: 8),
          CKeyValueEditor(
            value: action.payload,
            onChanged: (value) => setState(() => action.payload = value),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: ['Event Handlers', _isCreate ? 'New' : widget.name!],
        actions: [
          if (!_isCreate)
            IconButton(onPressed: _delete, icon: const Icon(Icons.delete_outline)),
        ],
      ),
      child: _loading
          ? const Center(child: FCircularProgress())
          : SingleChildScrollView(
              padding: const EdgeInsets.all(16),
              child: Form(
                key: _formKey,
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    FTextFormField(
                      control: FTextFieldControl.managed(controller: _nameController),
                      label: const Text('Name'),
                      enabled: _isCreate,
                      validator: (value) =>
                          (value == null || value.isEmpty) ? 'Required' : null,
                    ),
                    const SizedBox(height: 12),
                    FTextFormField(
                      control: FTextFieldControl.managed(controller: _eventController),
                      label: const Text('Event'),
                      validator: (value) =>
                          (value == null || value.isEmpty) ? 'Required' : null,
                    ),
                    const SizedBox(height: 12),
                    FTextField.multiline(
                      control: FTextFieldControl.managed(controller: _conditionController),
                      label: const Text('Condition (optional)'),
                      hint: 'Lua boolean expression — leave blank to always fire',
                      minLines: 1,
                      maxLines: 3,
                    ),
                    const SizedBox(height: 12),
                    Row(
                      children: [
                        const Text('Active'),
                        const SizedBox(width: 8),
                        FSwitch(value: _active, onChange: (value) => setState(() => _active = value)),
                      ],
                    ),
                    const SizedBox(height: 16),
                    CHeading('Actions', level: CHeadingLevel.h4),
                    const SizedBox(height: 8),
                    for (var i = 0; i < _actions.length; i++)
                      Padding(
                        padding: const EdgeInsets.only(bottom: 8),
                        child: _buildActionRow(i),
                      ),
                    FButton(
                      variant: FButtonVariant.outline,
                      onPress: () => setState(() => _actions.add(_ActionRow())),
                      child: const Text('Add action'),
                    ),
                    const SizedBox(height: 16),
                    FButton(
                      onPress: _saving ? null : _save,
                      child: _saving
                          ? const SizedBox(width: 16, height: 16, child: FCircularProgress())
                          : const Text('Save'),
                    ),
                  ],
                ),
              ),
            ),
    );
  }
}
