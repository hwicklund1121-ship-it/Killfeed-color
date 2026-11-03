$ErrorActionPreference = 'Stop'
$ownerPath = Join-Path $PSScriptRoot 'admin-owner.local.txt'
if (Test-Path -LiteralPath $ownerPath) {
    $adminSteamId = (Get-Content -LiteralPath $ownerPath -Raw).Trim()
} else {
    $adminSteamId = (Read-Host 'Enter your 17-digit SteamID64 for VPP Super Admin access').Trim()
}
if ($adminSteamId -notmatch '^\d{17}$') {
    throw 'SteamID64 must contain exactly 17 digits. No admin permissions were changed.'
}
[System.IO.File]::WriteAllText($ownerPath, $adminSteamId, [System.Text.Encoding]::ASCII)
$permissionDir = Join-Path $PSScriptRoot 'profiles\VPPAdminTools\Permissions\SuperAdmins'
[System.IO.Directory]::CreateDirectory($permissionDir) | Out-Null
$permissionFile = Join-Path $permissionDir 'SuperAdmins.txt'
# VPP reads while FGets() > 0: an empty line stops the entire admin-list load.
# Keep every valid existing admin, remove blank/comment lines, and avoid a BOM.
$existingIds = @()
if (Test-Path -LiteralPath $permissionFile) {
    $existingIds = @(Get-Content -LiteralPath $permissionFile | ForEach-Object { $_.Trim().TrimStart([char]0xFEFF).Replace(' ', '').Trim() } | Where-Object { $_ -match '^\d{17}$' } | Select-Object -Unique)
}
if ($existingIds -notcontains $adminSteamId) {
    $existingIds += $adminSteamId
}
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllLines($permissionFile, [string[]]$existingIds, $utf8NoBom)
Write-Host 'VPP Super Admin list repaired: no blank lines or BOM; existing valid admins preserved.'
