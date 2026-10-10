# Control-flow test target

`control_flow.ll` is included as an input for a future control-flow
optimization milestone. The current custom pass does not simplify constant
branches. Once a dedicated CFG transformation is implemented, add an automated
assertion that the unreachable `no` block is removed and validate the resulting
IR with `opt -passes=verify`.
