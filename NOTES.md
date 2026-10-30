# Teaching Notes

## How the user wants to be taught
- Concept before code. Explain geometry: what an angle measures, what a vector points from and toward.
- Introduce a variable only with what it stores and why the system needs it. No "just in case" variables.
- One step per turn; wait for "builds and works" before moving on. Treat that report as verification.
- Hints, pseudocode and snippets; full code only when stuck or asked.
- Review their code critically and explain *why* something is wrong instead of rewriting it.
- Answer the user's questions before continuing the implementation.
- Don't overengineer. Don't redesign the state machine yet.

## Working notes
- 2026-10-30: Code review of aiController.cpp found the movement-facing yaw guard is inverted
  (`< 1e-6f` should be `> 1e-6f`): yaw is only written while stationary, and atan2(0,0) = 0 snaps the
  agent to face +Z. Raised in lesson 0001.
- 2026-10-30: Investigate scan complete and reviewed. User is implementing Search on their own; review it when they return and ask how they decided it differs from Investigate.
- Edge case for later: hearing a new sound mid-Investigate after arrival keeps hasReachedSoundLocation true (no recapture, timer not reset).
- Investigate scan plan: (1) angle += dir*speed*dt  (2) reverse at ±limit  (3) yaw = facing + angle.
