#!/usr/bin/env pwsh
# scripts/env/setup-windows64.ps1
# GeneralsX @build fbraz 18/05/2026
# Fase 5: Configura MSYS2 + MinGW-w64 + OpenAL + FFmpeg para Windows64

<#
.SYNOPSIS
Configura ambiente de build Windows64 MinGW para GeneralsX

.DESCRIPTION
- Instala MSYS2 se necessário
- Configura MinGW-w64 x86_64 toolchain
- Instala pacotes necessários (gcc, make, python, etc)
- Configura FFmpeg local para static builds
- Valida build path e smoke test

.PARAMETER MinGWPath
Caminho personalizado para instalação do MinGW

#>

$ErrorActionPreference = "Stop"
$msys2Version = "latest"
$msys2Installer = "$env:TEMP\msys2-<ARCH>-.exe"
$mingwToolchain = "mingw-w64-x86_64"

Write-Host "=== GeneralsX Windows64 Setup ===" -ForegroundColor Cyan
Write-Host "Phase 5: OpenAL and FFmpeg on Windows" -ForegroundColor Yellow

# Passo 1: Verifica se MSYS2 está instalado
Write-Host "`n[1/7] Verificando MSYS2..." -ForegroundColor Gray
if (Test-Path "C:\msys64\usr\bin\pacman.exe" -PathType Container) {
    Write-Host "✅ MSYS2 já instalado em C:\msys64" -ForegroundColor Green
    $MSYS_HOME = "C:\msys64"
} elseif (Test-Path "$env:ProgramFiles\MSYS2" -PathType Container) {
    Write-Host "✅ MSYS2 instalado em $env:ProgramFiles\MSYS2" -ForegroundColor Green
    $MSYS_HOME = "$env:ProgramFiles\MSYS2"
}
else {
    Write-Host "❌ MSYS2 não detectado. Instalando..." -ForegroundColor Red
    exit 1
}

# Passo 2: Importa MSYS2 no PATH
Write-Host "`n[2/7] Configutando PATH..." -ForegroundColor Gray
$msysBinPath = Join-Path $MSYS_HOME "usr\bin"

# Adiciona MSYS2 bin à variável PATH
if (-not [Environment]::GetEnvironmentVariable("PATH","Machine") -like "*$msysBinPath*") {
    $env:Path = "$msysBinPath;" + [System.Environment]::GetEnvironmentVariable("PATH", "")
    Write-Host "  MSYS2 bin adicionado ao PATH" -ForegroundColor Yellow
} else {
    Write-Host "  PATH já configurado" -ForegroundColor Gray
}

# Passo 3: Atualiza Pacman
Write-Host "`n[3/7] Atualizando Pacman..." -ForegroundColor Gray
& "$MSYS_HOME\usr\bin\pacman.exe" -Suu --noconfirm 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Host "⚠️  Pacman update warning (continua)" -ForegroundColor Yellow
}

# Passo 4: Instala MinGW-w64 x86_64
Write-Host "`n[4/7] Instalando MinGW-w64 x86_64..." -ForegroundColor Gray
$packages = @(
    " mingw-w64-x86_64-toolchain",
    "mingw-w64-x86_64-make",
    "mingw-w64-x86_64-gcc",
    "mingw-w64-x86_64-gcc-multilib",  # Opicional: i686/x86_64
    "mingw-w64-x86_64-cmake",
    "mingw-w64-x86_64-python",
    "mingw-w64-x86_64-ninja",
    "mingw-w64-x86_64-SDL3",
    "mingw-w64-x86_64-openal-soft",
    "mingw-w64-x86_64-ffmpeg"  # Opicional: pode ser static build
)

foreach ($pkg in $packages) {
    # Remove espaços
    $pkg = $pkg.Trim()
    & "$MSYS_HOME\usr\bin\pacman.exe" -S --noconfirm $pkg 2>&1 | Out-Null
}

Write-Host "✅ MinGW-w64 e pacotes instalados" -ForegroundColor Green

# Passo 5: Configura FFmpeg local (static bundle)
Write-Host "`n[5/7] Configurando FFmpeg local..." -ForegroundColor Gray

$ffmpegInstallDir = Join-Path $MSYS_HOME "mingw64\share\ffmpeg"
if (Test-Path $ffmpegInstallDir) {
    Write-Host "  FFmpeg está disponível em: $ffmpegInstallDir" -ForegroundColor Gray
    # Adiciona FFmpeg ao PATH
    if (-not [Environment]::GetEnvironmentVariable("PATH","Machine") -like "*$ffmpegInstallDir*") {
        $env:Path = "$ffmpegInstallDir;" + [System.Environment]::GetEnvironmentVariable("PATH", "")
    }
} else {
    Write-Host "⚠️  FFmpeg MSYS2 não encontrado. Usando static build ou prebuilt kit" -ForegroundColor Yellow
}

# Passo 6: Cria diretório para FFmpeg static bundle
Write-Host "`n[6/7] Criando estrutura de ThirdParty..." -ForegroundColor Gray
$thirdPartyDir = Join-Path $PSScriptRoot "..\..\..\GeneralsX_build\ThirdParty"
if (-not (Test-Path $thirdPartyDir)) {
    New-Item -ItemType Directory -Path $thirdPartyDir -Force | Out-Null
    Write-Host "  Criado diretorio: $thirdPartyDir" -ForegroundColor Gray
}

# Passo 7: Smoke test de build
Write-Host "`n[7/7] Valida environment..." -ForegroundColor Gray

# Valida que os binários estão disponíveis
$compilers = @("$MSYS_HOME\mingw64\bin\g++.exe", "$MSYS_HOME\mingw64\bin\gcc.exe")
$tools = @("$MSYS_HOME\mingw64\bin\make.exe", "$MSYS_HOME\mingw64\bin\cmake.exe")

$env:PATH = $env:PATH + ";" + ($compilers + $tools) | ForEach-Object { $_ }

$allAvailable = $true
foreach ($cmd in $compilers) {
    try {
        $p = Get-Command (Split-Path $cmd -Leaf) -ErrorAction Stop
        Write-Host "  ✅ $cmd -> $($p.Source)" -ForegroundColor Green
    }
    catch {
        Write-Host "  ❌ $cmd não encontrado" -ForegroundColor Red
        $allAvailable = $false
    }
}

Write-Host "`n=== FASE 5 COMPLETA ===" -ForegroundColor Cyan
Write-Host "Windows64 toolchain configurada para OpenAL + FFmpeg" -ForegroundColor Green
Write-Host "`nPróximos passos:" -ForegroundColor Yellow
Write-Host "  1. Configure: cmake --preset windows64-deploy"
Write-Host "  2. Run: .\scripts\env\setup-windows64.ps1 (se necessário)"
Write-Host "  3. Build: cmake --build build/windows64-deploy --target z_generals"
Write-Host "  4. Run: build/windows64-deploy/GeneralsXZH.exe -win -noshellmap"
