$env:PATH = "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin;" + $env:PATH
$env:PATH += ";C:\Program Files\qemu"

Get-Process qemu-system-arm -ErrorAction SilentlyContinue | Stop-Process -Force

Write-Host "Starting QEMU (TCP server)..."
$q = Start-Process "qemu-system-arm" -ArgumentList "-machine","mps2-an386","-cpu","cortex-m4","-kernel","build\rtos-task-manager.elf","-display","none","-serial","tcp::4444,server","-no-reboot" -NoNewWindow -PassThru

Start-Sleep 1

Write-Host "Connecting TCP client..."
$client = New-Object System.Net.Sockets.TcpClient
$client.Connect("127.0.0.1", 4444)
$stream = $client.GetStream()

Write-Host "Waiting 8s for 256 samples..."
Start-Sleep 8

Write-Host "Sending latency command..."
[byte[]](0x6c,0x61,0x74,0x65,0x6e,0x63,0x79,0x0d,0x0a) | ForEach-Object {
    $stream.WriteByte($_)
    Start-Sleep -Milliseconds 20
}

Write-Host "Collecting output for 6s..."
$ms = New-Object System.IO.MemoryStream
$endTime = (Get-Date).AddSeconds(6)
$buf = New-Object byte[] 4096
do {
    if ($stream.DataAvailable) {
        $n = $stream.Read($buf,0,$buf.Length)
        if ($n -gt 0) { $ms.Write($buf,0,$n) }
    } else { Start-Sleep -Milliseconds 10 }
} while ((Get-Date) -lt $endTime)

$client.Close()
Stop-Process -Id $q.Id -Force -ErrorAction SilentlyContinue

$bytes = $ms.ToArray()
$text = [System.Text.Encoding]::UTF8.GetString($bytes)
$text | Set-Content uart_session.txt -Encoding UTF8
Write-Host "Saved uart_session.txt - $($bytes.Length) bytes"
$text -split [char]10
