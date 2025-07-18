#!/bin/bash

set -e

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Parse arguments
FORCE_UPDATE=0
SKIP_UPDATE=0
while [[ $# -gt 0 ]]; do
    case $1 in
        --force)
            FORCE_UPDATE=1
            shift
            ;;
        --skip-update)
            SKIP_UPDATE=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--force] [--skip-update]"
            echo "  --force       Force update V8 even if already present"
            echo "  --skip-update Skip V8 updates if already present"
            exit 1
            ;;
    esac
done

echo "Setting up V8 build environment..."

# Function to run gclient sync with minimal download options
gclient_sync() {
    local extra_args="$@"
    echo "Running gclient sync (minimal download)..."
    gclient sync --no-history --shallow $extra_args
}

# Check if we need to install dependencies
NEED_DEPS=0
for pkg in git curl python3 pkg-config lsb-release ninja-build build-essential; do
    if ! dpkg -l | grep -q "^ii  $pkg "; then
        NEED_DEPS=1
        break
    fi
done

if [ "$NEED_DEPS" -eq 1 ]; then
    echo "Installing dependencies (requires sudo)..."
    sudo apt-get update
    sudo apt-get install -y \
        git \
        curl \
        python3 \
        pkg-config \
        lsb-release \
        ninja-build \
        build-essential
else
    echo "All dependencies are already installed."
fi

# Change to project root
cd "$PROJECT_ROOT"

# Drop sudo privileges for depot_tools operations
if [ "$EUID" -eq 0 ]; then
    echo "Dropping root privileges for depot_tools operations..."
    REAL_USER=$(who am i | awk '{print $1}')
    if [ -z "$REAL_USER" ]; then
        REAL_USER=$SUDO_USER
    fi
    if [ -z "$REAL_USER" ]; then
        echo "Warning: Could not determine real user, continuing as root (not recommended)"
    else
        # Run depot_tools operations as the real user
        su -c "$(cat <<EOF
set -e
cd "$PROJECT_ROOT"
SKIP_UPDATE=$SKIP_UPDATE
FORCE_UPDATE=$FORCE_UPDATE

# Create depot_tools directory
if [ ! -d "depot_tools" ]; then
    echo "Cloning depot_tools..."
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

# Add depot_tools to PATH
export PATH="$PROJECT_ROOT/depot_tools:\$PATH"

# Clean up any stale lock files that might cause issues
echo "Cleaning up any stale lock files..."
find "$PROJECT_ROOT/depot_tools" -name "*.locked" -type f -delete 2>/dev/null || true
pkill -f gclient 2>/dev/null || true
pkill -f gsutil 2>/dev/null || true

# Fetch V8
# Handle various states: no v8 dir, partial checkout, full checkout
if [ ! -d "v8" ]; then
    echo "V8 directory not found..."
    # Check if we have a .gclient file (partial checkout)
    if [ -f ".gclient" ] || gclient root >/dev/null 2>&1; then
        echo "Found existing .gclient file. Running gclient sync to restore V8..."
        gclient_sync --force --no-history --shallow
        # If v8 dir still doesn't exist, we need to recreate .gclient
        if [ ! -d "v8" ]; then
            echo "gclient sync failed to create v8 directory. Recreating checkout..."
            rm -f .gclient .gclient_entries
            fetch --no-history v8
            cd v8
            gclient_sync --no-history --shallow
        else
            cd v8
        fi
    else
        echo "No existing checkout found. Fetching V8 (minimal download)..."
        fetch --no-history v8
        echo "Running gclient sync..."
        cd v8
        gclient_sync --no-history --shallow
    fi
else
    cd v8
    if [ "$SKIP_UPDATE" -eq 1 ]; then
        echo "V8 directory exists, skipping update (--skip-update flag set)"
    elif [ "$FORCE_UPDATE" -eq 1 ]; then
        echo "Forcing V8 update (--force flag set)..."
        # Handle detached HEAD state
        git fetch origin
        # Check if we're on a branch or detached HEAD
        if git symbolic-ref -q HEAD >/dev/null 2>&1; then
            # On a branch, pull normally
            git pull
        else
            # Detached HEAD, checkout main branch first
            echo "V8 is in detached HEAD state, checking out main branch..."
            git checkout main || git checkout master
            git pull
        fi
        gclient_sync
        # Update timestamp
        date > .v8_last_updated
    else
        # Check if update is needed (default: update if older than 7 days)
        if [ -f ".v8_last_updated" ]; then
            LAST_UPDATE=\$(date -r .v8_last_updated +%s 2>/dev/null || echo 0)
            CURRENT_TIME=\$(date +%s)
            DAYS_SINCE_UPDATE=\$(( (CURRENT_TIME - LAST_UPDATE) / 86400 ))
            if [ \$DAYS_SINCE_UPDATE -lt 7 ]; then
                echo "V8 was updated \$DAYS_SINCE_UPDATE days ago, skipping update"
                echo "(Use --force to force update or --skip-update to always skip)"
            else
                echo "V8 was last updated \$DAYS_SINCE_UPDATE days ago, updating..."
                git fetch origin
                if git symbolic-ref -q HEAD >/dev/null 2>&1; then
                    git pull
                else
                    echo "V8 is in detached HEAD state, checking out main branch..."
                    git checkout main || git checkout master
                    git pull
                fi
                gclient_sync
                date > .v8_last_updated
            fi
        else
            echo "No update timestamp found, updating V8..."
            git fetch origin
            if git symbolic-ref -q HEAD >/dev/null 2>&1; then
                git pull
            else
                echo "V8 is in detached HEAD state, checking out main branch..."
                git checkout main || git checkout master
                git pull
            fi
            gclient_sync
            date > .v8_last_updated
        fi
    fi
fi
EOF
)" "$REAL_USER"
    fi
else
    # Not running as root, proceed normally
    # Create depot_tools directory
    if [ ! -d "depot_tools" ]; then
        echo "Cloning depot_tools..."
        git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    fi

    # Add depot_tools to PATH
    export PATH="$PROJECT_ROOT/depot_tools:$PATH"

    # Clean up any stale lock files that might cause issues
    echo "Cleaning up any stale lock files..."
    find "$PROJECT_ROOT/depot_tools" -name "*.locked" -type f -delete 2>/dev/null || true
    pkill -f gclient 2>/dev/null || true
    pkill -f gsutil 2>/dev/null || true

    # Fetch V8
    # Handle various states: no v8 dir, partial checkout, full checkout
    if [ ! -d "v8" ]; then
        echo "V8 directory not found..."
        # Check if we have a .gclient file (partial checkout)
        if [ -f ".gclient" ] || gclient root >/dev/null 2>&1; then
            echo "Found existing .gclient file. Running gclient sync to restore V8..."
            gclient_sync --force --no-history --shallow
            # If v8 dir still doesn't exist, we need to recreate .gclient
            if [ ! -d "v8" ]; then
                echo "gclient sync failed to create v8 directory. Recreating checkout..."
                rm -f .gclient .gclient_entries
                fetch v8
                cd v8
                gclient_sync
            else
                cd v8
            fi
        else
            echo "No existing checkout found. Fetching V8 (minimal download)..."
            fetch --no-history v8
            echo "Running gclient sync..."
            cd v8
            gclient_sync --no-history --shallow
        fi
    else
        cd v8
        if [ "$SKIP_UPDATE" -eq 1 ]; then
            echo "V8 directory exists, skipping update (--skip-update flag set)"
        elif [ "$FORCE_UPDATE" -eq 1 ]; then
            echo "Forcing V8 update (--force flag set)..."
            # Handle detached HEAD state
            git fetch origin
            # Check if we're on a branch or detached HEAD
            if git symbolic-ref -q HEAD >/dev/null 2>&1; then
                # On a branch, pull normally
                git pull
            else
                # Detached HEAD, checkout main branch first
                echo "V8 is in detached HEAD state, checking out main branch..."
                git checkout main || git checkout master
                git pull
            fi
            gclient_sync
            # Update timestamp
            date > .v8_last_updated
        else
            # Check if update is needed (default: update if older than 7 days)
            if [ -f ".v8_last_updated" ]; then
                LAST_UPDATE=$(date -r .v8_last_updated +%s 2>/dev/null || echo 0)
                CURRENT_TIME=$(date +%s)
                DAYS_SINCE_UPDATE=$(( (CURRENT_TIME - LAST_UPDATE) / 86400 ))
                if [ $DAYS_SINCE_UPDATE -lt 7 ]; then
                    echo "V8 was updated $DAYS_SINCE_UPDATE days ago, skipping update"
                    echo "(Use --force to force update or --skip-update to always skip)"
                else
                    echo "V8 was last updated $DAYS_SINCE_UPDATE days ago, updating..."
                    git fetch origin
                    if git symbolic-ref -q HEAD >/dev/null 2>&1; then
                        git pull
                    else
                        echo "V8 is in detached HEAD state, checking out main branch..."
                        git checkout main || git checkout master
                        git pull
                    fi
                    gclient_sync
                    date > .v8_last_updated
                fi
            else
                echo "No update timestamp found, updating V8..."
                git fetch origin
                if git symbolic-ref -q HEAD >/dev/null 2>&1; then
                    git pull
                else
                    echo "V8 is in detached HEAD state, checking out main branch..."
                    git checkout main || git checkout master
                    git pull
                fi
                gclient_sync
                date > .v8_last_updated
            fi
        fi
    fi
fi

echo "V8 setup complete!"
echo "To build V8, run: build_v8.sh"