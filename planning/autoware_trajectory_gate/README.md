# autoware_trajectory_gate

## Overview

This package subscribes to multiple trajectory, selects one and publish it.
The selector node monitors the interval of each trajectory topic, and publishes it as diagnostics.

## Parameters

| Name                      | Type      | Description                                  |
| ------------------------- | --------- | -------------------------------------------- |
| trajectory_warn_duration  | double    | Warning threshold of trajectory interval.    |
| trajectory_error_duration | double    | Error threshold of trajectory interval.      |
| source_ids                | list[int] | List of trajectory source IDs.               |
| source.&lt;id&gt;.name    | string    | Name of the corresponding trajectory source. |
