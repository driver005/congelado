import 'package:flutter/services.dart' show FilteringTextInputFormatter;
import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// Verified against Forui 0.25.0's real source: FTextField no longer takes a `controller:`
// param directly — the controller/initial-value/onChange trio moved into a `control:`
// FTextFieldControl object (FTextFieldControl.managed(controller: ...)).

/// A numeric text field prefixed with a currency symbol — Medusa UI's
/// `CurrencyInput` (`.../ui/src/components/currency-input`). Digit/decimal
/// filtering only (no locale-aware thousands-grouping/formatting — that's a
/// real feature, out of scope for a first pass).
class CCurrencyInput extends StatelessWidget {
  const CCurrencyInput({
    super.key,
    this.controller,
    this.currencySymbol = '\$',
    this.hint,
  });

  final TextEditingController? controller;
  final String currencySymbol;
  final String? hint;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return FTextField(
      control: FTextFieldControl.managed(controller: controller),
      hint: hint,
      keyboardType: const TextInputType.numberWithOptions(decimal: true),
      inputFormatters: [FilteringTextInputFormatter.allow(RegExp(r'[0-9.]'))],
      prefixBuilder: (context, style, _) => Padding(
        padding: const EdgeInsets.only(left: 10),
        child: Text(currencySymbol, style: TextStyle(color: colors.mutedForeground)),
      ),
    );
  }
}
