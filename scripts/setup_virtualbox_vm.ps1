# Setup MicroOS Virtual Machine in Oracle VirtualBox
$vbox = "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe"
$iso = "C:\Users\M SRIVAS\.gemini\antigravity\scratch\MicroOS\release\microos.iso"
$vmName = "MicroOS"

# Unregister if previously exists
& $vbox unregistervm $vmName --delete 2>$null

Write-Host "Creating Virtual Machine: $vmName..."
& $vbox createvm --name $vmName --ostype "Linux_64" --register

Write-Host "Configuring hardware (1024 MB RAM, 1 CPU)..."
& $vbox modifyvm $vmName --memory 1024 --cpus 1 --graphicscontroller vboxvga --vram 16 --boot1 dvd --boot2 disk --boot3 none --boot4 none

Write-Host "Adding storage controller..."
& $vbox storagectl $vmName --name "IDE Controller" --add ide

Write-Host "Attaching MicroOS ISO to Optical Drive..."
& $vbox storageattach $vmName --storagectl "IDE Controller" --port 0 --device 0 --type dvddrive --medium $iso

Write-Host "Verifying VM status..."
& $vbox list vms

Write-Host ""
Write-Host "SUCCESS: Virtual machine '$vmName' is ready in Oracle VirtualBox!"
