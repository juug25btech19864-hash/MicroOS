$vbox = "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe"
$projDir = "C:\Users\M SRIVAS\.gemini\antigravity\scratch\MicroOS"
$iso = "$projDir\release\microos.iso"
$serialLog = "$projDir\logs\vbox_uefi_serial.log"
$vmName = "MicroOS-UEFI-Test"

Write-Host "=== Oracle VirtualBox UEFI QA Test ==="
& $vbox controlvm $vmName poweroff 2>$null
Start-Sleep -Seconds 2
& $vbox unregistervm $vmName --delete 2>$null

if (Test-Path $serialLog) { Remove-Item -Force $serialLog }

& $vbox createvm --name $vmName --ostype "Linux_64" --register
& $vbox modifyvm $vmName `
    --firmware efi `
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

& $vbox storagectl $vmName --name "IDE Controller" --add ide
& $vbox storageattach $vmName --storagectl "IDE Controller" --port 0 --device 0 --type dvddrive --medium $iso

Write-Host "Starting VM in UEFI mode (headless)..."
& $vbox startvm $vmName --type headless

Write-Host "Waiting 22 seconds for UEFI boot..."
Start-Sleep -Seconds 22

$vmState = (& $vbox showvminfo $vmName --machinereadable | Select-String "VMState=").Line
Write-Host "VM State: $vmState"

if (Test-Path $serialLog) {
    Write-Host "=== UEFI GUEST SERIAL CONSOLE OUTPUT ==="
    Get-Content $serialLog -Tail 50
} else {
    Write-Host "Serial log not found."
}

Write-Host "Powering off UEFI VM..."
& $vbox controlvm $vmName poweroff
& $vbox storageattach $vmName --storagectl "IDE Controller" --port 0 --device 0 --type dvddrive --medium none 2>$null
& $vbox unregistervm $vmName --delete 2>$null
Write-Host "=== UEFI Test Completed ==="
