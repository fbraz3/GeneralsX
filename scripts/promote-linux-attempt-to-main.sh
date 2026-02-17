#!/usr/bin/env bash
# GeneralsX @build fbraz3 17/02/2026 Promote linux-attempt branch to main (archive old SDL2+Wine approach)

set -e

echo "🌿 GeneralsX Branch Promotion: linux-attempt → main"
echo "=================================================="
echo ""
echo "This will:"
echo "  1. Archive current 'main' as 'old-main-sdl2'"
echo "  2. Rename 'linux-attempt' to 'main'"
echo "  3. Force push new 'main' to origin"
echo ""
echo "⚠️  WARNING: This is a DESTRUCTIVE operation!"
echo "⚠️  Current 'main' (SDL2+Wine approach) will be replaced"
echo ""

# Safety check: Ensure we're in GeneralsX repo
if [[ ! -d .git ]] || [[ ! -f CMakeLists.txt ]]; then
    echo "❌ Error: Must run from GeneralsX repository root"
    exit 1
fi

# Safety check: Ensure linux-attempt exists
if ! git show-ref --verify --quiet refs/heads/linux-attempt; then
    echo "❌ Error: Branch 'linux-attempt' does not exist"
    exit 1
fi

# Safety check: Ensure main exists
if ! git show-ref --verify --quiet refs/heads/main; then
    echo "❌ Error: Branch 'main' does not exist"
    exit 1
fi

# Safety check: Clean working directory
if [[ -n $(git status --porcelain) ]]; then
    echo "❌ Error: Working directory has uncommitted changes"
    echo "   Commit or stash changes before promoting branches"
    git status --short
    exit 1
fi

# Confirmation prompt
read -p "👉 Type 'PROMOTE' to confirm: " confirm
if [[ "$confirm" != "PROMOTE" ]]; then
    echo "❌ Aborted. No changes made."
    exit 1
fi

echo ""
echo "📦 Step 1/5: Creating backup branch 'old-main-sdl2'..."
git branch old-main-sdl2 main
git push origin old-main-sdl2
echo "   ✅ Backup created and pushed"

echo ""
echo "📦 Step 2/5: Switching to linux-attempt branch..."
git checkout linux-attempt
echo "   ✅ Checked out linux-attempt"

echo ""
echo "📦 Step 3/5: Renaming linux-attempt → main (local)..."
git branch -M linux-attempt main
echo "   ✅ Local branch renamed"

echo ""
echo "📦 Step 4/5: Force pushing new main to origin..."
git push origin main --force-with-lease
echo "   ✅ Remote main updated"

echo ""
echo "📦 Step 5/5: Updating remote HEAD pointer..."
git remote set-head origin main
echo "   ✅ Remote HEAD updated"

echo ""
echo "✅ SUCCESS! Branch promotion complete"
echo ""
echo "📊 Summary:"
echo "   - Old main archived as: old-main-sdl2"
echo "   - New main is now: linux-attempt (SDL3+DXVK)"
echo "   - Remote updated: origin/main"
echo ""
echo "🎯 Next Steps:"
echo "   1. Update README.md with new build instructions"
echo "   2. Update GitHub repo default branch settings (if needed)"
echo "   3. Notify collaborators about branch change"
echo "   4. Continue with Phase 2 (OpenAL audio) development"
echo ""
echo "🔄 To verify:"
echo "   git log --oneline -5"
echo "   git branch -a | grep main"
echo ""
