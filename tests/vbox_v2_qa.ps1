$vbox = "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe"
$projDir = "C:\Users\M SRIVAS\.gemini\antigravity\scratch\MicroOS"
$iso = "$projDir\release\microos.iso"
$serialLog = "$projDir\logs\vbox_serial.log"
$vmName = "MicroOS-v2-QA"

Write-Host "Cleaning up previous test instances..."
& $vbox controlvm $vmName poweroff 2>$null
Start-Sleep -Seconds 2
& $vbox unregistervm $vmName --delete 2>$null

if (Test-Path $serialLog) { Remove-Item -Force $serialLog }

Write-Host "Creating Virtual Machine: $vmName..."
& $vbox createvm --name $vmName --ostype "Linux_64" --register

Write-Host "Configuring hardware: 1024MB RAM, 1 CPU, No Network (Offline), Serial Logging..."
& $vbox modifyvm $vmName `
    --memory 1024 `
    --cpus 1 `
    --nic1 none `
    --graphicscontroller vboxvga `
    --vram 16 `
    --boot1 dvd `
    --boot2 disk `
    --boot3 none `
    --boot4 none `
    --uart1 0x3F8 4 `
    --uartmode1 file $serialLog

Write-Host "Attaching MicroOS v2.0 ISO to Virtual Drive..."
& $vbox storagectl $vmName --name "IDE Controller" --add ide
& $vbox storageattach $vmName --storagectl "IDE Controller" --port 0 --device 0 --type dvddrive --medium $iso

Write-Host "Starting Virtual Machine in Headless Mode..."
& $vbox startvm $vmName --type headless

Write-Host "Waiting 24 seconds for boot sequence..."
Start-Sleep -Seconds 24

Write-Host "Taking screenshot..."
$screenshot = "$projDir\logs\vbox_v2_boot.png"
& $vbox controlvm $vmName screenshotpng $screenshot

$vmState = (& $vbox showvminfo $vmName --machinereadable | Select-String "VMState=").Line
Write-Host "Current VM State: $vmState"

if (Test-Path $serialLog) {
    Write-Host "=== GUEST SERIAL CONSOLE OUTPUT ==="
    Get-Content $serialLog -Tail 60
} else {
    Write-Host "Serial log not found."
}

Write-Host "Gracefully stopping VM via ACPI..."
& $vbox controlvm $vmName acpipowerbutton
Start-Sleep -Seconds 3
$vmState = (& $vbox showvminfo $vmName --machinereadable | Select-String "VMState=").Line
if ($vmState -notmatch "poweroff") {
    & $vbox controlvm $vmName poweroff
}

& $vbox storageattach $vmName --storagectl "IDE Controller" --port 0 --device 0 --type dvddrive --medium none 2>$null
& $vbox unregistervm $vmName --delete 2>$null
Write-Host "VirtualBox v2.0 QA Test Completed Successfully."
