#!/bin/bash

echo "Setting up the daemon..."

USER="alek"
GROUP="users"
DATABASE_PATH="/var/lib/"

APP_PATH="/home/$USER/weatherMonitor"
APP_PATH_EXECUTABLE="$APP_PATH/main"

SERVICE_FILE="/etc/systemd/system/weather-monitor.service"

chown "$USER":"$GROUP" "$DATABASE_PATH"
chmod 700 "$DATABASE_PATH"

make -C "$APP_PATH"

chmod +x "$APP_PATH_EXECUTABLE"

sudo bash -c "echo '[Unit]' > $SERVICE_FILE"
sudo bash -c "echo 'Description=Weather Monitor' >> $SERVICE_FILE"
sudo bash -c "echo 'After=network.target' >> $SERVICE_FILE"
sudo bash -c "echo 'StartLimitIntervalSec=0' >> $SERVICE_FILE"

sudo bash -c "echo '' >> $SERVICE_FILE"
sudo bash -c "echo '[Service]' >> $SERVICE_FILE"
sudo bash -c "echo 'Type=simple' >> $SERVICE_FILE"
sudo bash -c "echo 'Restart=always' >> $SERVICE_FILE"
sudo bash -c "echo 'RestartSec=1' >> $SERVICE_FILE"
sudo bash -c "echo 'User=$USER' >> $SERVICE_FILE"
sudo bash -c "echo 'ExecStart=$APP_PATH_EXECUTABLE' >> $SERVICE_FILE"

sudo bash -c "echo '' >> $SERVICE_FILE"
sudo bash -c "echo '[Install]' >> $SERVICE_FILE"
sudo bash -c "echo 'WantedBy=multi-user.target' >> $SERVICE_FILE"

sudo systemctl daemon-reload
sudo systemctl enable weather-monitor.service
sudo systemctl start weather-monitor.service
sudo systemctl status weather-monitor.service