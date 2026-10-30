# Baseline: perception, memory and a basic state machine already built

The user built and tested FOV via dot product against (sin yaw, 0, cos yaw), physics line of sight, visual and auditory memory with Visible/Heard → Remembered → Unknown states, and an Unaware/Investigate/Engage/Search state machine with `enterState()` entry actions. They converted atan2 radians to degrees for yaw. Start new teaching from per-frame behavior (time-based rotation), not from perception basics.

**Evidence**: Project handoff of 2026-10-30 plus the code in `src/gameLayer/ai/aiController.cpp`.
