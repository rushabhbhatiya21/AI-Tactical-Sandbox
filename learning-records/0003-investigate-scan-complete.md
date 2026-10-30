# Investigate scan: time-based sweep with edge reversal

The user implemented a dt-scaled sweep (offset from a frozen centre yaw, reversed at ±limit with a direction check) and on their own added a pause at each edge, a believable-behavior design choice. They needed the yaw = centre + offset formula given, and placed the edge check before the update (so an overshot yaw was shown during the pause). Ordering of update → constrain → apply is worth re-testing when Search scanning is built.

**Evidence**: Working scan reported 2026-10-30; code in `updateInvestigateBehavior`.
