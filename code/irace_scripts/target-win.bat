@echo off
::##############################################################################
:: BAT version of target-runner for Windows.
:: Contributed by Andre de Souza Andrade <andre.andrade@uniriotec.br>.
:: Check other examples in examples/
::
:: This script is run in the execution directory (execDir, --exec-dir).
::
:: PARAMETERS:
:: %%1 is the candidate configuration number
:: %%2 is the instance ID
:: %%3 is the seed
:: %%4 is the instance name
:: The rest are parameters to the target-algorithm
::
:: RETURN VALUE:
:: This script should print one numerical value: the cost that must be minimized.
:: Exit with 0 if no error, with 1 in case of error
::##############################################################################

:: Please change the EXE and FIXED_PARAMS to the correct ones
SED "workdir=C:\Users\clf1v16\docs_offline\01.HHCRSP\Code\HHCRSP\workbench\"
SET "exe=python C:\Users\clf1v16\docs_offline\01.HHCRSP\Code\HHCRSP\hhcrsp_launcher.py"
SET "exereader=python C:\Users\clf1v16\docs_offline\01.HHCRSP\Code\HHCRSP\read_quality.py"
rem SET "fixed_params=-f=filename.txt"

FOR /f "tokens=1-4*" %%a IN ("%*") DO (
	SET candidate=%%a
	SET instance_id=%%b
	SET seed=%%c
	SET instance=%%d
	SET candidate_parameters=%%e
)

SET "stdres=c%candidate%-%instance_id%-%seed%.stdres"
SET "stdout=%workdir%c%candidate%-%instance_id%-%seed%.stdout"
SET "stderr=%workdir%c%candidate%-%instance_id%-%seed%.stderr"

:: :: FIXME: How to implement this in BAT ?
::if not exist %exe% error "%exe%: not found or not executable (pwd: %(pwd))"

:: Save  the output to a file, and parse the result from it.

%exe% --workdir=%workdir% -f=%stdres% -iid=%instance_id% -cn=%candidate% -i=%instance% -seed=%seed% %candidate_parameters% 1>%stdout% 2>%stderr%

%exereader% %stdres%


:: :: This may be used to introduce a delay if there are filesystem
:: :: issues.
:: setlocal EnableDelayedExpansion
:: :loop
:: for /f %%i in ("%stdout%") do set size=%%~zi
:: if "%size%" NEQ "0" (goto endloop)
:: waitfor false /t 10 >nul
:: goto loop
:: :endloop

:: This is an example of reading a number from the output.
:: It assumes that the objective value is the first number in
:: the first column of the last line of the output.
rem setlocal EnableDelayedExpansion
rem set "cmd=findstr /R /N "^^" %stdout% | find /C ":""
rem for /f %%a in ('!cmd!') do set numberlines=%%a
rem set /a lastline=%numberlines%-1
rem for /f "tokens=3" %%F in ('more +%lastline% %stdout%') do set COST=%%F
rem echo %COST%

:: Un-comment this if you want to delete temporary files.
del %stdout% %stderr% %stdres%
exit 0