#!/usr/bin/env python3
"""
Test GPIO 5 power control
"""
import time
import os

# Try sysfs GPIO
def gpio_sysfs(pin, value):
    try:
        # Export
        with open('/sys/class/gpio/export', 'w') as f:
            f.write(str(pin))
        time.sleep(0.1)
        
        # Set direction
        with open(f'/sys/class/gpio/gpio{pin}/direction', 'w') as f:
            f.write('out')
        
        # Set value
        with open(f'/sys/class/gpio/gpio{pin}/value', 'w') as f:
            f.write(str(value))

        with open(f'/sys/class/gpio/unexport', 'w') as f:
            f.write(str(value))

        return True
    except Exception as e:
        print(f"GPIO sysfs error: {e}")
        return False

print("Testing GPIO 5 with sysfs...")

# Turn ON
print("Turning GPIO 5 ON...")
gpio_sysfs(5, 1)
time.sleep(2)

# Turn OFF
print("Turning GPIO 5 OFF...")
gpio_sysfs(5, 0)
time.sleep(1)

# Turn ON again
print("Turning GPIO 5 ON...")
gpio_sysfs(5, 1)
time.sleep(2)

print("Done - GPIO 5 should be ON now")