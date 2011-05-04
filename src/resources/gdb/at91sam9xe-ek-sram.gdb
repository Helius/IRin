# MCK initialization script for the AT91SAM9XE
#------------------------------------------------
# Configure master clock
echo Configuring the master clock...\n
# Enable main oscillator
set *0xFFFFFC20 = 0x00004001
while ((*0xFFFFFC68 & 0x1) == 0)
end

# Set PLLA to 200MHz
set *0xFFFFFC28 = 0x2060BF09
while ((*0xFFFFFC68 & 0x2) == 0)
end

# Select prescaler
set *0xFFFFFC30 = 0x00000100
while ((*0xFFFFFC68 & 0x8) == 0)
end

# Select master clock
set *0xFFFFFC30 = 0x00000102
while ((*0xFFFFFC68 & 0x8) == 0)
end

echo Master clock ok.\n

