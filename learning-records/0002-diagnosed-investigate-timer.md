# Diagnosed a timer that expired during travel

The user found on their own that Investigate's fixed timer ran out before the agent reached the sound, and moved the timer start to arrival. The partial fix (timer left at 0 on entry) caused a per-frame Investigate/Unaware flip, and the Search timer was moved into Engage by mistake. Lesson: state timers and the state's exit conditions in `updateDecision` must change together; worth reinforcing when Search gets its own scan.

**Evidence**: Change reviewed 2026-10-30 in `aiController.cpp` (`enterState`, Investigate arrival block).
