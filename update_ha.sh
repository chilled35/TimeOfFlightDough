#!/bin/bash
# update_ha.sh — pull latest changes and copy dashboard files into HA www.
# Run from the CC Terminal after any repo update.

set -e

REPO_DIR="/config/TimeOfFlightDough"
WWW_DIR="/config/www/dough_dashboard"

echo ">>> Pulling latest from GitHub..."
git -C "${REPO_DIR}" pull origin claude/tender-sagan-FsZTQ

echo ">>> Copying dashboard files to ${WWW_DIR}..."
mkdir -p "${WWW_DIR}"
# Preserve vendor/ directory (large files, not in repo)
cp -r "${REPO_DIR}/dashboard/css"  "${WWW_DIR}/"
cp -r "${REPO_DIR}/dashboard/js"   "${WWW_DIR}/"
cp    "${REPO_DIR}/dashboard/dough_dashboard.html" "${WWW_DIR}/"

echo ""
echo "=== Done. Hard-reload the HA dashboard to pick up changes. ==="
echo ""
