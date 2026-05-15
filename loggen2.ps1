[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

$logPath = "C:\Users\getto\протеже папки рабочего стола\учёба\TcpServer\Logs"
$filePath = "$logPath\example.csv"
New-Item -ItemType Directory -Force -Path $logPath | Out-Null

$iterations = 3000
$sources = "AppModule", "NetService", "DataProcessor", "UIController"

Write-Host "Начинаю генерацию $iterations тестовых записей в $logPath"

$sw = [System.IO.StreamWriter]::new($filePath, $true, [System.Text.Encoding]::UTF8)
$sw.AutoFlush = $true

try{
    for ($i = 1; $i -le $iterations; $i++) {
        $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss.fff"
        $chance = Get-Random -Minimum 1 -Maximum 101
        if($chance -le 5) {$level = "Info"}
        elseif ($chance -le 10) { $level = "Warning"}
        else {$level = "Error"}
        $source = $sources | Get-Random
        $code = Get-Random -Minimum 1000 -Maximum 9999
        $message = "Test event #{$i}: operation completed with status $level"
        
        $line = "$timestamp, $level, $source, Code:$code, $message"
        $sw.WriteLine($line)
        Start-Sleep -Milliseconds 5
    }
}
finally{
    $sw.Close()
    Write-Host "Сгенерировано $iterations тестовых записей в $logPath"
}