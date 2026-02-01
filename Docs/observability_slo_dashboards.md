# Observability SLO Dashboards

This guide describes the standard dashboards that surface service-level objectives (SLOs) for Libft subsystems. The layouts focus on latency, errors, and saturation so operators can confirm that exported Prometheus or OpenTelemetry metrics stay within agreed budgets.

## Dashboard layout

Each dashboard groups widgets into three rows that emphasize the golden signals:

1. **Latency** – charts focus on request, processing, and dependency delays.
2. **Errors** – panels expose failure rates and mapped error codes.
3. **Saturation** – visuals cover capacity consumption and backlog pressure.

Organize the rows so the most critical service appears first (Networking, Storage, CMA, Logger). Keep panels consistent across services to simplify on-call scanning.

## Latency panels

Latency graphs read from histograms published by the modules:

- **HTTP client/server**: `nw_http_request_duration_seconds` (buckets) with `endpoint` and `method` labels.
- **Task scheduler**: `ts_task_total_duration_ms_bucket` tagged by `queue` and `priority`.
- **Compression**: `compression_stream_duration_ms_bucket` labelled with `algorithm` and `operation`.

Render percentile lines for the 50th, 90th, 95th, and 99th percentiles. Example PromQL for the 99th percentile:

```
histogram_quantile(
    0.99,
    sum(rate(nw_http_request_duration_seconds_bucket{service="gateway"}[5m])) by (le)
)
```

Highlight panels whenever percentile targets exceed the agreed budget (for example, 300 ms P99 for networking APIs).

## Error panels

Error rate widgets pull from counter metrics and error tagging hooks:

- **Networking**: `nw_http_client_errors_total` and `nw_http_server_errors_total` labelled by `error_code`.
- **Storage**: `storage_operation_errors_total` labelled by `operation` and `error_code`.
- **Logger**: `logger_sink_failures_total` labelled by `sink`.

Use per-service burn-rate queries that compare the short-term (5 minute) and long-term (1 hour) error ratios against the SLO target. Example PromQL for a 99.9% SLO:

```
max(
    rate(nw_http_client_errors_total[5m])
    /
    clamp_min(rate(nw_http_client_requests_total[5m]), 1)
)
```

Add tables that translate `error_code` values with the descriptions from `Errno/errno.hpp` so responders know the failure source immediately.

## Saturation panels

Saturation dashboards monitor resource usage:

- **CMA allocator**: `cma_bytes_in_use` and `cma_bytes_peak` gauges with the active limit overlayed.
- **Thread pools**: `ts_worker_active_count` and `ts_queue_depth` from the task scheduler bridge exporter.
- **Compression streams**: `compression_active_streams` with a configured maximum.

For each panel, configure alert thresholds tied to capacity budgets (for example, 80% of the CMA allocation limit).

## Alerting integration

Define multi-window, multi-burn alerts for each SLO. Pair the dashboards with notification rules that page when both the fast-burn and slow-burn error ratios cross the target (see Google SRE workbook patterns). Document the alert names and escalation policies directly inside the Grafana panel descriptions so responders do not need to look them up elsewhere.

## Operational runbooks

Link the dashboards to the existing runbooks:

- Networking playbooks in `Docs/module_overviews.md`.
- Logger sink diagnostics in `Docs/logger_logging_flags.md`.

Ensure each dashboard panel description references the relevant runbook section and the owner team channel.

## Publishing checklist

Before sharing the dashboards:

1. Export the Grafana JSON to the `Observability` folder in version control.
2. Confirm that staging environments emit all referenced metrics.
3. Validate dashboard variables (service, region, environment) match deployment labels.
4. Share annotated screenshots in release notes so adopters can import the dashboards quickly.

Follow this checklist for every major release that changes metrics or SLO targets.
