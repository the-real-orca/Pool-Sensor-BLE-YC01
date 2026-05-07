# Gemini CLI Agent Setup

> Purpose: help Gemini CLI and future agents use the generated documentation and this skill safely
> Status: draft generated from static inventory; verify and complete against source code.

## Detected Gemini CLI / Agent Artifacts

| Kind | Path | Detail |
| --- | --- | --- |
| ignore | .geminiignore | Gemini file access ignore rules |
| context | GEMINI.md | Gemini context file |
| settings | .gemini/settings.json | Gemini settings |
| extension_manifest | .gemini/extensions/codebase-documentation-analyzer/gemini-extension.json | name=codebase-documentation-analyzer; version=1.0.0; context=GEMINI.md |
| context | .gemini/extensions/codebase-documentation-analyzer/GEMINI.md | Gemini context file |
| custom_command | .gemini/extensions/codebase-documentation-analyzer/commands/docs/analyze-codebase.toml | command=/docs:analyze-codebase |
| custom_command | .gemini/extensions/codebase-documentation-analyzer/commands/docs/gap-review.toml | command=/docs:gap-review |
| custom_command | .gemini/extensions/codebase-documentation-analyzer/commands/docs/update-codebase-docs.toml | command=/docs:update-codebase-docs |
| agent_skill | .gemini/extensions/codebase-documentation-analyzer/skills/codebase-documentation-analyzer/SKILL.md | skill=codebase-documentation-analyzer |
| agent_skill | .gemini/skills/pool-sensor-guidelines/SKILL.md | skill=pool-sensor-guidelines |
| agent_skill | skills/pool-sensor-guidelines/SKILL.md | skill=pool-sensor-guidelines |
## Recommended Gemini CLI Workflow

1. Start Gemini CLI from the repository root.
2. Invoke the `codebase-documentation-analyzer` skill for documentation, onboarding, extension planning, test-case generation, or refactoring preparation.
3. Run the inventory and scaffold scripts only from the activated skill directory. Do not run project application code unless explicitly requested.
4. Use `docs/README.md` as the documentation index and `docs/ai-agent-knowledge-base.md` as the compact source map.
5. If `GEMINI.md` or command files change, run `/memory refresh`, `/memory show`, `/skills reload`, or `/commands reload` as appropriate.

## Suggested Project `GEMINI.md`

```markdown
# Project context for Gemini CLI

Use `docs/README.md` as the documentation index and `docs/ai-agent-knowledge-base.md` as the compact source map for future work. Prefer source files over generated docs when they conflict. Do not expose secrets. Before implementing changes, check the FSD, TSD, configuration reference, interface reference, runtime states, traceability matrix, and tests map.
```

## Suggested Custom Command

Place this at `.gemini/commands/docs/analyze.toml` to create `/docs:analyze`.

```toml
description = "Generate source-grounded FSD, TSD, API, config, state, diagram, and AI-agent docs."
prompt = """
Use the codebase-documentation-analyzer skill to analyze the current repository.

Requirements:
- Write documentation under docs/.
- Run only the skill's read-only inventory/scaffold scripts unless explicitly requested otherwise.
- Produce or update FSD, TSD, configuration reference, interface/API examples, runtime states, diagrams, traceability, testing map, and ai-agent knowledge base.
- Redact all secrets.
- Treat user arguments as additional constraints: {{args}}
"""
```

## Future-Agent Prompts

```text
Use docs/README.md and docs/ai-agent-knowledge-base.md as the starting context. Verify claims against source before changing code.
```

## Troubleshooting

- Skill not found: verify the `SKILL.md` path and run `/skills reload`.
- Workspace skill ignored: check workspace trust settings.
- Context stale: run `/memory refresh` and inspect `/memory show`.
- Command missing: verify TOML syntax and run `/commands reload`.
- Files unavailable: check `.geminiignore`, hidden-file policy, and workspace roots.
