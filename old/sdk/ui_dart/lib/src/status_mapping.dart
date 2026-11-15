import 'components/status_badge.dart' show CStatusVariant;

/// Maps a `WorkflowStatus` value (`plugins/engine/src/model/workflow/status.cppm`
/// — `RUNNING`/`COMPLETED`/`FAILED`/`TIMED_OUT`/`PAUSED`/`TERMINATED`) onto a
/// [CStatusVariant]. Shared so every page showing an execution's status (list,
/// detail, DAG overlay) agrees on the same color — this used to be a private
/// `_variantFor` duplicated (and silently drifted) across pages.
CStatusVariant statusVariantForWorkflowStatus(String status) {
  switch (status) {
    case 'COMPLETED':
      return CStatusVariant.success;
    case 'FAILED':
    case 'TIMED_OUT':
    case 'TERMINATED':
      return CStatusVariant.danger;
    case 'RUNNING':
      return CStatusVariant.info;
    case 'PAUSED':
      return CStatusVariant.warning;
    default:
      return CStatusVariant.neutral;
  }
}

/// Maps a `TaskStatus` value (`plugins/engine/src/model/task/status.cppm` —
/// `SCHEDULED`/`IN_PROGRESS`/`COMPLETED`/`FAILED`/`TIMED_OUT`/`SKIPPED`/
/// `CANCELED`) onto a [CStatusVariant]. See [statusVariantForWorkflowStatus]
/// for why this is shared rather than hand-rolled per page.
CStatusVariant statusVariantForTaskStatus(String status) {
  switch (status) {
    case 'COMPLETED':
      return CStatusVariant.success;
    case 'FAILED':
    case 'TIMED_OUT':
    case 'CANCELED':
      return CStatusVariant.danger;
    case 'IN_PROGRESS':
      return CStatusVariant.info;
    case 'SKIPPED':
      return CStatusVariant.warning;
    default:
      return CStatusVariant.neutral;
  }
}
