#!/usr/bin/env bash
set -euo pipefail

# Resolve macOS runner selection ("local" or "github") from CI_RUNNER_CONFIG_URL YAML.
# Any error, missing key, non-"local" value, or external fork PR safely falls back to "github".

OUTPUT_FILE="${GITHUB_OUTPUT:-/dev/null}"
SELECTED="github"

if [ "${IS_EXTERNAL_FORK:-false}" = "true" ]; then
  echo "External fork pull request detected; forcing macos runner to github."
elif [ -z "${CI_RUNNER_CONFIG_URL:-}" ]; then
  echo "CI_RUNNER_CONFIG_URL is not set; defaulting macos runner to github."
else
  if YAML_CONTENT="$(curl -fsSL --max-time 5 "$CI_RUNNER_CONFIG_URL" 2>/dev/null)"; then
    PARSED=""
    if command -v yq >/dev/null 2>&1; then
      PARSED="$(printf '%s\n' "$YAML_CONTENT" | yq -r '.runners.mac // "github"' 2>/dev/null || true)"
    elif command -v ruby >/dev/null 2>&1; then
      PARSED="$(printf '%s\n' "$YAML_CONTENT" | ruby -ryaml -e '
data = YAML.safe_load($stdin.read)
val = data.is_a?(Hash) && data["runners"].is_a?(Hash) ? data["runners"]["mac"] : nil
puts(val.is_a?(String) ? val : "github")
' 2>/dev/null || true)"
    elif command -v python3 >/dev/null 2>&1; then
      PARSED="$(printf '%s\n' "$YAML_CONTENT" | python3 -c '
import sys
try:
    import yaml
    data = yaml.safe_load(sys.stdin) or {}
    val = (data.get("runners") or {}).get("mac", "github")
    print(val if isinstance(val, str) else "github")
except Exception:
    print("github")
' 2>/dev/null || true)"
    fi

    PARSED="$(printf '%s' "$PARSED" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
    if [ "$PARSED" = "local" ]; then
      SELECTED="local"
    fi
    echo "Resolved runners.mac='${PARSED:-<empty>}' -> selected='$SELECTED'."
  else
    echo "Failed to fetch CI_RUNNER_CONFIG_URL; falling back to github."
  fi
fi

echo "runner=$SELECTED" >> "$OUTPUT_FILE"
