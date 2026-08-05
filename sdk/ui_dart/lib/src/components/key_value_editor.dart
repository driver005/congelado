import 'package:flutter/material.dart' show Icons;
import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// Verified against Forui 0.25.0's real source: FTextField's `initialValue`/`onChange` params
// moved into a `control:` FTextFieldControl object, and FButton takes `variant:` (FButtonVariant
// enum) directly instead of `style: FButtonStyle.outline`.

/// An editable flat `Map<String, String>` — one row of key/value text fields
/// per entry, plus an "Add" row. Fully controlled (caller owns `value`, gets
/// changes via `onChanged`), same philosophy as `CDataTable`'s own state
/// model. Three real call sites need this: `EventAction.payload`
/// (`plugins/engine/src/model/event/handler.cppm`), `WorkflowSchedule.
/// seed_variables` (`plugins/engine/src/model/schedule/definition.cppm`),
/// and a workflow's execution-start seed variables — worth sharing rather
/// than duplicating the add/remove-row logic three times.
class CKeyValueEditor extends StatelessWidget {
  const CKeyValueEditor({
    super.key,
    required this.value,
    required this.onChanged,
    this.keyHint = 'Key',
    this.valueHint = 'Value',
  });

  final Map<String, String> value;
  final ValueChanged<Map<String, String>> onChanged;
  final String keyHint;
  final String valueHint;

  @override
  Widget build(BuildContext context) {
    final entries = value.entries.toList();
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        for (final entry in entries)
          Padding(
            padding: const EdgeInsets.only(bottom: 8),
            child: Row(
              children: [
                Expanded(
                  child: FTextField(
                    hint: keyHint,
                    control: FTextFieldControl.managed(
                      initial: TextEditingValue(text: entry.key),
                      onChange: (change) {
                        final newKey = change.text;
                        final next = {...value};
                        next.remove(entry.key);
                        next[newKey] = entry.value;
                        onChanged(next);
                      },
                    ),
                  ),
                ),
                const SizedBox(width: 8),
                Expanded(
                  child: FTextField(
                    hint: valueHint,
                    control: FTextFieldControl.managed(
                      initial: TextEditingValue(text: entry.value),
                      onChange: (change) => onChanged({...value, entry.key: change.text}),
                    ),
                  ),
                ),
                FButton.icon(
                  variant: FButtonVariant.outline,
                  onPress: () {
                    final next = {...value}..remove(entry.key);
                    onChanged(next);
                  },
                  child: const Icon(Icons.close),
                ),
              ],
            ),
          ),
        FButton(
          variant: FButtonVariant.outline,
          onPress: () {
            // A blank-string key is a real, if odd, map key — bumps a counter into the key
            // instead so repeated "Add" presses don't just silently collide/overwrite the same
            // empty-string entry.
            var candidate = '';
            var suffix = 1;
            while (value.containsKey(candidate)) {
              candidate = 'key_$suffix';
              suffix++;
            }
            onChanged({...value, candidate: ''});
          },
          child: const Text('Add'),
        ),
      ],
    );
  }
}
