@echo off
rem Libera a porta do modo Batalha no Firewall do Windows (TCP 5050, rede privada).
rem Rode no computador que vai HOSPEDAR a partida: clique com o botao direito > Executar como administrador.
set PORTA=5050
if not "%1"=="" set PORTA=%1

net session >nul 2>&1
if errorlevel 1 (
  echo Precisa rodar como administrador: botao direito ^> Executar como administrador.
  pause
  exit /b 1
)

netsh advfirewall firewall delete rule name="Bloons TD Battles" >nul 2>&1
netsh advfirewall firewall add rule name="Bloons TD Battles" dir=in action=allow protocol=TCP localport=%PORTA% profile=private,domain
if errorlevel 1 (
  echo Nao foi possivel criar a regra.
) else (
  echo Porta %PORTA% liberada. Para remover: netsh advfirewall firewall delete rule name="Bloons TD Battles"
)
pause
