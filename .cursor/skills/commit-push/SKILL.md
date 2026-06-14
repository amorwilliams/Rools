---
name: commit-push
description: >-
  Stage changes, create a git commit, and push to remote. Use when the user
  asks to commit, push, commit & push, save to git, or sync with remote.
disable-model-invocation: true
---

# commit & push

commit & push to remote（Rools 项目）

## When to run

User asks to commit, push, or both. Do not commit unless explicitly requested.

## Workflow

### 1. Inspect (parallel)

```bash
git status
git diff && git diff --cached
git log -5 --oneline
```

Also check branch vs remote if pushing: `git status -sb` or `git rev-parse --abbrev-ref @{upstream}`.

### 2. Draft commit message

- Summarize **why**, not just what (1–2 sentences)
- Match recent repo commit style from `git log`
- Do not stage `.env`, credentials, or secrets; warn if user asked to commit them
- Skip empty commits (nothing to commit → push only if ahead of remote)

### 3. Commit

```bash
git add <relevant paths>
git commit -m "$(cat <<'EOF'
Subject line.

Optional body explaining why.
EOF
)"
git status
```

**Safety**

- Never `git config` changes
- Never `--no-verify` / skip hooks unless user asks
- Never force-push `main`/`master`; warn user
- `--amend` only if user requested AND HEAD is yours AND not pushed
- Hook failed → fix and **new** commit, never amend a failed commit

### 4. Push

```bash
git push origin HEAD
```

If no upstream: `git push -u origin HEAD`

Use `required_permissions: ["git_write", "full_network"]` for commit/push shell commands.

### 5. Report

Return commit hash, message summary, and remote branch status. If push failed, show error and next step.

## Rools 注意

- 含 `lib/libDaisy`、`lib/DaisySP` 改动时一并 `git add` submodule 指针
- 勿提交 `firmware/compile_commands.json`（已 gitignore）
- 克隆方需 `git clone --recursive`
