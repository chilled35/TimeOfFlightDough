#!/bin/bash
# setup_ha.sh — one-time setup of the dashboard on a Home Assistant instance.
# Run this from the CC Terminal inside HA (or via SSH).
#
# After this runs, run update_ha.sh to pull latest changes.

set -e

REPO_DIR="/config/TimeOfFlightDough"
WWW_DIR="/config/www/dough_dashboard"
VENDOR_DIR="${WWW_DIR}/js/vendor"

# ---- 1. Clone or update the repo ------------------------------------------
if [ -d "${REPO_DIR}/.git" ]; then
  echo ">>> Repo already exists — pulling latest..."
  git -C "${REPO_DIR}" pull origin claude/tender-sagan-FsZTQ
else
  echo ">>> Cloning repo..."
  git clone \
    --branch claude/tender-sagan-FsZTQ \
    --single-branch \
    https://github.com/chilled35/TimeOfFlightDough.git \
    "${REPO_DIR}"
fi

# ---- 2. Copy dashboard files into HA www (NOT a symlink) ------------------
# aiohttp static server won't serve files outside /config/www/ via symlink.
echo ">>> Copying dashboard files to ${WWW_DIR}..."
mkdir -p "${WWW_DIR}"
cp -r "${REPO_DIR}/dashboard/"* "${WWW_DIR}/"

# ---- 3. Download Three.js vendor files (only if missing) ------------------
mkdir -p "${VENDOR_DIR}"

if [ ! -f "${VENDOR_DIR}/three.module.js" ]; then
  echo ">>> Downloading three.module.js..."
  curl -fsSL -o "${VENDOR_DIR}/three.module.js" \
    https://cdn.jsdelivr.net/npm/three@0.163.0/build/three.module.js
else
  echo ">>> three.module.js already present — skipping."
fi

if [ ! -f "${VENDOR_DIR}/OrbitControls.js" ]; then
  echo ">>> Downloading OrbitControls.js..."
  curl -fsSL -o "${VENDOR_DIR}/OrbitControls.js" \
    https://cdn.jsdelivr.net/npm/three@0.163.0/examples/jsm/controls/OrbitControls.js
else
  echo ">>> OrbitControls.js already present — skipping."
fi

echo ""
echo "=== Setup complete ==="
echo "Dashboard: /local/dough_dashboard/dough_dashboard.html"
echo "To update: run update_ha.sh"
echo ""
