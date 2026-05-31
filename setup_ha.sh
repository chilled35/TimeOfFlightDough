#!/bin/bash
# setup_ha.sh — one-time setup of the dashboard on a Home Assistant instance.
# Run this from the CC Terminal inside HA (or via SSH).
#
# After this runs, 'git pull' inside /config/TimeOfFlightDough is all you
# ever need to do to update the dashboard.

set -e

REPO_DIR="/config/TimeOfFlightDough"
WWW_LINK="/config/www/dough_dashboard"
VENDOR_DIR="${REPO_DIR}/dashboard/js/vendor"

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

# ---- 2. Symlink dashboard into HA www --------------------------------------
mkdir -p /config/www

if [ -L "${WWW_LINK}" ]; then
  echo ">>> Symlink already exists — skipping."
elif [ -d "${WWW_LINK}" ]; then
  echo ">>> WARNING: ${WWW_LINK} is a real directory (not a symlink)."
  echo ">>> Backing it up to ${WWW_LINK}.bak and replacing with symlink."
  mv "${WWW_LINK}" "${WWW_LINK}.bak"
  ln -s "${REPO_DIR}/dashboard" "${WWW_LINK}"
else
  echo ">>> Creating symlink: ${WWW_LINK} -> ${REPO_DIR}/dashboard"
  ln -s "${REPO_DIR}/dashboard" "${WWW_LINK}"
fi

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

# ---- Done -----------------------------------------------------------------
echo ""
echo "=== Setup complete ==="
echo ""
echo "Dashboard is live at: /local/dough_dashboard/dough_dashboard.html"
echo ""
echo "To update in future, just run:"
echo "  cd ${REPO_DIR} && git pull"
echo ""
