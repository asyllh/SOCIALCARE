
@ECHO OFF
setlocal enableextensions enabledelayedexpansion
rem FOR %%G IN (a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z) DO (echo %%G)

SET /a count=0
SET IDIR=\\filestore.soton.ac.uk\\users\\clf1v16\\mydocuments\\temp\\Instances\\Ait_Haddadene\\adapted\\
rem SET IDIR=C:\
echo %IDIR%

SET /a TIME=15
FOR %%F IN (18-4-s-2a, 18-4-s-2b, 18-4-s-2c, 18-4-s-2d, 18-4-m-2a, 18-4-m-2b, 18-4-m-2c, 18-4-m-2d, 18-4-m-2e, 18-4-l-2a, 18-4-l-2b, 18-4-l-2c, 18-4-l-2d, 18-4-l-2e) DO (
	SET /a count+=1
	echo solver_bat.bat !count! !count! -1 "%IDIR%%%F" -do_twopt=0 -no_change_ls=1 -no_change_grasp=1 -pr_strategy=1 -pr_direction=1 -sols_in_pool=19 -grasp_dl=0.25 -grasp_dr=0.81 -rcl_strategy=2 -t %TIME%
)
endlocal
	rem SET /a count += 1
	rem SET instance=%instance_dir%%inst_name%
	rem echo solver_bat.bat %count% %count% -1 %instance% -do_twopt=0 -no_change_ls=1 -no_change_grasp=1 -pr_strategy=1 -pr_direction=1 -sols_in_pool=19 -grasp_dl=0.25 -grasp_dr=0.81 -rcl_strategy=2

rem for name in 45-10-s-3a, 45-10-s-2a, 45-10-s-3b, 45-10-s-2b, 45-10-s-3c, 45-10-m-4, 45-10-m-2a, 45-10-m-2b, 45-10-m-3, 45-10-l-2a, 45-10-l-2b, 45-10-l-3, 45-10-l-4;
rem do
rem 	SET instance=%%d
rem done
rem for name in 73-16-s-2a, 73-16-s-3, 73-16-s-2b, 73-16-m-3a, 73-16-m-2, 73-16-m-3b, 73-16-l-2, 73-16-l-3, 73-16-l-4, 73-16-l-5;
rem do

rem done
