param(
    [string]$MsysRoot = $env:GENERALSX_MSYS2_ROOT,
    [switch]$PersistUserRoot,
    [switch]$ShowSummary,
    [switch]$InstallMinGW64,  # Instalar MinGW-w64 x86_64 toolchain
    [switch]$InstallCMake     # Instalar CMake e dependências
)

$ErrorActionPreference = 'Stop'

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "GeneralsX Windows64 MinGW Setup" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if ([string]::IsNullOrWhiteSpace($MsysRoot)) {
    Write-Host "⚠️  MSYS2_ROOT não definido. Usando padrão: C:\msys64" -ForegroundColor Yellow
    $MsysRoot = 'C:\msys64'
}

# Verificar se MSYS2 está instalado
Write-Host "[1/4] Verificando MSYS2..." -ForegroundColor Yellow
$msys2Path = Join-Path $MsysRoot 'usr\bin\pacman.exe'
if (Test-Path $msys2Path) {
    Write-Host "  ✅ MSYS2 encontrado em: $MsysRoot" -ForegroundColor Green
    $msys2Version = & $msys2Path --version 2>&1
    Write-Host "  Version: $msys2Version" -ForegroundColor Gray
} else {
    Write-Host "  ❌ MSYS2 não encontrado. Instalando..." -ForegroundColor Red
    Write-Host "  Instalando MSYS2 via winget..." -ForegroundColor Gray
    winget install MSYS2.MSYS2 -e -i -s winget --silent 2>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  ⚠️  winget falhou. Tente instalar manualmente:" -ForegroundColor Yellow
        Write-Host "     winget install MSYS2.MSYS2" -ForegroundColor Yellow
        Write-Host "     ou baixe de https://www.msys2.org/" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "  Pressione ENTER para continuar após instalar MSYS2 manualmente" -ForegroundColor Yellow
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        exit 1
    }
}

# Definir paths para MinGW-w64 x86_64
$mingw64Root = Join-Path $MsysRoot 'mingw64'
$mingw64Bin = Join-Path $mingw64Root 'bin'
$msysBin = Join-Path $MsysRoot 'usr\bin'

# Instalar MinGW-w64 x86_64 se solicitado ou se não existir
if ($InstallMinGW64 -or -not (Test-Path $mingw64Root)) {
    Write-Host ""
    Write-Host "[2/4] Instalando MinGW-w64 x86_64 toolchain..." -ForegroundColor Yellow
    Write-Host "  Executando: pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-make mingw-w64-x86_64-ninja..." -ForegroundColor Gray
    & $msys2Path -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-make mingw-w64-x86_64-ninja 2>&1 | Select-Object -Last 10
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  ✅ MinGW-w64 x86_64 toolchain instalado" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️  Instalação do MinGW-w64 x86_64 falhou" -ForegroundColor Yellow
    }
}

# Instalar CMake e dependências se solicitado ou se não existir
if ($InstallCMake -or -not (Test-Path (Join-Path $mingw64Root 'bin\cmake.exe'))) {
    Write-Host ""
    Write-Host "[3/4] Instalando CMake e dependências..." -ForegroundColor Yellow
    Write-Host "  Executando: pacman -S --needed cmake ninja git curl wget unzip..." -ForegroundColor Gray
    & $msys2Path -S --needed cmake ninja git curl wget unzip 2>&1 | Select-Object -Last 5
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  ✅ CMake e dependências instalados" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️  Instalação de CMake/Ninja falhou" -ForegroundColor Yellow
    }
}

# Configurar environment
$env:MSYSTEM = 'MINGW64'
$env:MSYSTEM_PREFIX = ($mingw64Root -replace '\\', '/')
$env:CHERE_INVOKING = '1'

$pathEntries = @($mingw64Bin, $msysBin)
$currentPathEntries = @($env:PATH -split ';' | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })

$newPathEntries = New-Object System.Collections.Generic.List[string]
foreach ($entry in $pathEntries) {
    $newPathEntries.Add($entry)
}

foreach ($entry in $currentPathEntries) {
    if ($pathEntries -notcontains $entry) {
        $newPathEntries.Add($entry)
    }
}

$env:PATH = ($newPathEntries | Select-Object -Unique) -join ';'

if ($PersistUserRoot) {
    [System.Environment]::SetEnvironmentVariable('GENERALSX_MSYS2_ROOT', $MsysRoot, 'User')
}

# Validar compiladores
Write-Host ""
Write-Host "[4/4] Validando compiladores..." -ForegroundColor Yellow
Write-Host "  x86_64-w64-mingw32-gcc: $(x86_64-w64-mingw32-gcc --version 2>&1)" -ForegroundColor Gray
Write-Host "  x86_64-w64-mingw32-g++: $(x86_64-w64-mingw32-g++ --version 2>&1)" -ForegroundColor Gray
Write-Host "  ninja: $(ninja --version 2>&1)" -ForegroundColor Gray
Write-Host "  ✅ Compiladores validados" -ForegroundColor Green

if ($ShowSummary) {
    Write-Host ""
    Write-Host "MSYS2 root: $MsysRoot"
    Write-Host "MSYSTEM: $($env:MSYSTEM)"
    Write-Host "MSYSTEM_PREFIX: $($env:MSYSTEM_PREFIX)"
    Write-Host "gcc: $((Get-Command x86_64-w64-mingw32-gcc).Source)"
    Write-Host "g++: $((Get-Command x86_64-w64-mingw32-g++).Source)"
    Write-Host "make: $((Get-Command make).Source)"
    Write-Host "ninja: $((Get-Command ninja).Source)"
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Setup concluído!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Próximos passos:" -ForegroundColor Yellow
Write-Host "  1. Configure: cmake --preset windows64-deploy" -ForegroundColor White
Write-Host "  2. Build: cmake --build build/windows64-deploy --target z_generals" -ForegroundColor White
Write-Host "  3. Run: .\build\windows64-deploy\GeneralsMD\GeneralsXZH -win" -ForegroundColor White
Write-Host ""