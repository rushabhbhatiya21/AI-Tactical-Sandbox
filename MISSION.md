# Mission: Game AI Engineering (Tactical AI Sandbox)

## Why
Build a systems-focused C++/OpenGL portfolio project, the Tactical AI Sandbox, that shows I can design autonomous agents: how they perceive, remember, decide and act. I want to be able to design these systems on my own, not just get them to compile.

## Success looks like
- Agents perceive (vision, FOV, line of sight, hearing) and remember (last known positions, sounds), and I can explain every line of that code.
- A state machine (Unaware / Investigate / Engage / Search) whose behaviors look different and believable.
- Combat, cover/positioning and multi-agent coordination built on top of it.
- In-game debug visualisation that makes the AI's internal reasoning visible.

## Constraints
- One small step at a time: concept first, code second; I write the code myself and report back whether it builds and works.
- Keep the existing architecture (Game / World / GameObject / AIController / PhysicsWorld, Jolt CharacterVirtual movement). No `shared` folder.
- Yaw convention: degrees, 0 = +Z, forward = (sin yaw, 0, cos yaw).

## Out of scope
- Networking / multiplayer.
- A general-purpose engine or content-heavy game.
- Real audio playback (for now; sound events stand in for it).
