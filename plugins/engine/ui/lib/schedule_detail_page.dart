import 'dart:async' show unawaited;

import 'package:congelado_ui_sdk/congelado_ui_sdk.dart';
import 'package:flutter/material.dart';
import 'package:forui/forui.dart';

import 'api_client.dart';

// Verified against Forui 0.25.0's real source: FTextFormField's `controller` moved into a
// `control:` FTextFieldControl object; FSelect now takes `items: Map<String, T>` + `control:
// FSelectControl<T>` instead of `format`/`initialValue`/`children: [FSelectItem(...)]`; FButton
// takes `variant:` (FButtonVariant enum) directly instead of `style: FButtonStyle.outline`.

/// Create/edit form for a single `WorkflowSchedule`, bound by hand to its
/// known JSON shape (`name`, `workflow_name`, `workflow_version`,
/// `cron_expression`, `seed_variables`, `enabled`, `paused`, `last_fired_at`
/// — see `plugins/engine/src/model/schedule/definition.cppm`). `name ==
/// null` means create mode; pushed from [SchedulesListPage].
class ScheduleDetailPage extends StatefulWidget {
  const ScheduleDetailPage({super.key, required this.name});

  /// Existing schedule's name, or `null` to create a new one.
  final String? name;

  @override
  State<ScheduleDetailPage> createState() => _ScheduleDetailPageState();
}

class _ScheduleDetailPageState extends State<ScheduleDetailPage> {
  final _api = EngineApiClient();
  final _formKey = GlobalKey<FormState>();
  final _nameController = TextEditingController();
  final _versionController = TextEditingController(text: '1');
  final _cronController = TextEditingController();
  List<String> _workflowNames = [];
  String? _workflowName;
  Map<String, String> _seedVariables = {};
  bool _enabled = true;
  bool _paused = false;
  String? _lastFiredAt;
  List<Map<String, dynamic>> _nextRuns = [];
  bool _loading = false;
  bool _saving = false;
  bool _pausing = false;
  bool _loadingRuns = false;

  bool get _isCreate => widget.name == null;

  @override
  void initState() {
    super.initState();
    _init();
  }

  Future<void> _init() async {
    setState(() => _loading = true);
    try {
      final names = await _api.listWorkflows();
      _workflowNames = [for (final w in names) w['name'] as String? ?? ''];
      if (!_isCreate) {
        final schedule = await _api.getSchedule(widget.name!);
        _nameController.text = schedule['name'] as String? ?? '';
        _workflowName = schedule['workflow_name'] as String?;
        _versionController.text = '${schedule['workflow_version'] ?? 1}';
        _cronController.text = schedule['cron_expression'] as String? ?? '';
        _seedVariables = (schedule['seed_variables'] as Map<String, dynamic>? ?? const {})
            .map((key, value) => MapEntry(key, '$value'));
        _enabled = schedule['enabled'] as bool? ?? true;
        _paused = schedule['paused'] as bool? ?? false;
        _lastFiredAt = schedule['last_fired_at'] as String?;
        unawaited(_loadNextRuns());
      }
      if (mounted) setState(() {});
    } finally {
      if (mounted) setState(() => _loading = false);
    }
  }

  Future<void> _loadNextRuns() async {
    setState(() => _loadingRuns = true);
    try {
      final runs = await _api.nextFewRuns(widget.name!);
      if (mounted) setState(() => _nextRuns = runs);
    } catch (_) {
      // Best-effort preview panel — a cron_expression that doesn't parse (422) or a schedule
      // that isn't saved yet just leaves this panel empty, no need to surface it as a page error.
    } finally {
      if (mounted) setState(() => _loadingRuns = false);
    }
  }

  @override
  void dispose() {
    _nameController.dispose();
    _versionController.dispose();
    _cronController.dispose();
    super.dispose();
  }

  Future<void> _save() async {
    if (!_formKey.currentState!.validate()) return;
    if (_workflowName == null || _workflowName!.isEmpty) {
      ScaffoldMessenger.of(context)
          .showSnackBar(const SnackBar(content: Text('Pick a workflow')));
      return;
    }
    setState(() => _saving = true);
    try {
      final body = {
        'name': _nameController.text,
        'workflow_name': _workflowName,
        'workflow_version': int.tryParse(_versionController.text) ?? 1,
        'cron_expression': _cronController.text,
        'seed_variables': _seedVariables,
        'enabled': _enabled,
      };
      if (_isCreate) {
        await _api.createSchedule(body);
      } else {
        await _api.updateSchedule(widget.name!, body);
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
      title: 'Delete schedule?',
      description: 'This removes the "${widget.name}" schedule.',
      confirmLabel: 'Delete',
    );
    if (!confirmed) return;
    try {
      await _api.deleteSchedule(widget.name!);
      if (mounted) Navigator.of(context).pop(true);
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text('Delete failed: $e')));
    }
  }

  Future<void> _togglePause() async {
    setState(() => _pausing = true);
    try {
      if (_paused) {
        await _api.resumeSchedule(widget.name!);
      } else {
        await _api.pauseSchedule(widget.name!);
      }
      setState(() => _paused = !_paused);
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text('Failed: $e')));
    } finally {
      if (mounted) setState(() => _pausing = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return CPageShell(
      topbar: CPageTopbar(
        breadcrumbs: ['Schedules', _isCreate ? 'New' : widget.name!],
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
                    FSelect<String>(
                      hint: 'Workflow',
                      items: {for (final name in _workflowNames) name: name},
                      control: FSelectControl<String>.managed(
                        initial: _workflowName,
                        onChange: (value) => setState(() => _workflowName = value),
                      ),
                    ),
                    const SizedBox(height: 12),
                    FTextFormField(
                      control: FTextFieldControl.managed(controller: _versionController),
                      label: const Text('Workflow version'),
                      keyboardType: TextInputType.number,
                    ),
                    const SizedBox(height: 12),
                    FTextFormField(
                      control: FTextFieldControl.managed(controller: _cronController),
                      label: const Text('Cron expression'),
                      validator: (value) =>
                          (value == null || value.isEmpty) ? 'Required' : null,
                    ),
                    const CHint('5-field cron format, e.g. "0 */15 * * *" for every 15 minutes.'),
                    const SizedBox(height: 12),
                    CHeading('Seed variables', level: CHeadingLevel.h5),
                    const SizedBox(height: 4),
                    CKeyValueEditor(
                      value: _seedVariables,
                      onChanged: (value) => setState(() => _seedVariables = value),
                    ),
                    const SizedBox(height: 12),
                    Row(
                      children: [
                        const Text('Enabled'),
                        const SizedBox(width: 8),
                        FSwitch(value: _enabled, onChange: (value) => setState(() => _enabled = value)),
                      ],
                    ),
                    if (!_isCreate) ...[
                      const SizedBox(height: 12),
                      Text('Last fired: ${_lastFiredAt ?? 'Never'}'),
                      const SizedBox(height: 8),
                      FButton(
                        variant: _paused ? FButtonVariant.primary : FButtonVariant.outline,
                        onPress: _pausing ? null : _togglePause,
                        child: _pausing
                            ? const SizedBox(width: 16, height: 16, child: FCircularProgress())
                            : Text(_paused ? 'Resume schedule' : 'Pause schedule'),
                      ),
                      const SizedBox(height: 16),
                      CHeading('Next few runs', level: CHeadingLevel.h5),
                      const CHint(
                        'Computed from now, not from the last actual fire — a preview, not a guarantee.',
                      ),
                      const SizedBox(height: 4),
                      if (_loadingRuns)
                        const FCircularProgress()
                      else if (_nextRuns.isEmpty)
                        const Text('No upcoming runs computed.')
                      else
                        for (final run in _nextRuns) Text('${run['at']}'),
                      const SizedBox(height: 4),
                      FButton(
                        variant: FButtonVariant.outline,
                        onPress: _loadingRuns ? null : _loadNextRuns,
                        child: const Text('Refresh'),
                      ),
                    ],
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
