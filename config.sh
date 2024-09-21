#!/bin/bash

file="./main.c"
target_line=14
read -p "Enter the number of minutes to render the new image on the display: " minutes
seconds=$((minutes * 60))
new_line="static constexpr unsigned long SecondsBetweenImages = $seconds;"

if [ -f "$file" ]; then
    sed -i "${target_line}s|^.*$|$new_line|" "$file"
    echo "Updated line $target_line in $file with the new update interval of $minutes minutes ($seconds seconds)."
else
    echo "Error: File $file not found!"
    exit 1
fi

show_spinner() {
    local pid=$!
    local delay=0.1
    local spinstr='|/-\'
    echo "Compiling the project with 'make', please wait..."
    while [ "$(ps a | awk '{print $1}' | grep $pid)" ]; do
        local temp=${spinstr#?}
        printf " [%c]  " "$spinstr"
        spinstr=$temp${spinstr%"$temp"}
        sleep $delay
        printf "\b\b\b\b\b\b"
    done
    echo "Done!"
}

make & show_spinner

read -p "Would you like to add a cron job to launch PiArtFrame at login? (yes/no): " response

if [[ "$response" == "yes" ]]; then
    current_dir=$(pwd)
    cron_job="@reboot sleep 15 && cd $current_dir && ./piArtFrame"
    comment="##PiArtFrame"
    (sudo crontab -l | grep -q "$cron_job") || {
        echo "Adding PiArtFrame to the crontab for @reboot..."
        (sudo crontab -l 2>/dev/null; echo "$comment"; echo "$cron_job") | sudo crontab -
        echo "Cron job added successfully! After reboot, the system will wait 15 seconds and then launch PiArtFrame."
    }
elif [[ "$response" == "no" ]]; then
    echo "Okay, the cron job was not added. You can add it manually later if needed."
else
    echo "Invalid response. Please answer yes or no."
fi
