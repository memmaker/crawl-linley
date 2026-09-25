#!/bin/sh
# Upload web/dist to https://ruzzoli.de/roguelikes/crawl-linley/ (build first)
cd "$(dirname "$0")" && git fetch -q memmaker && [ -z "$(git status --porcelain)" ] && [ "$(git rev-parse @)" = "$(git rev-parse @{u})" ] || { echo "commit + push first"; exit 1; }
set -e
ssh ruzzoli.de 'sudo mkdir -p /var/www/ruzzoli.de/roguelikes/crawl-linley && sudo chown felix:www-data /var/www/ruzzoli.de/roguelikes/crawl-linley'
rsync -rtz --delete dist/ ruzzoli.de:/var/www/ruzzoli.de/roguelikes/crawl-linley/
