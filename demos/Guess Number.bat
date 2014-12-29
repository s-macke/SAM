cd ..
@echo off
rem color 0a

:MAIN
cls
set number=%random:~0,2%
rem goto WINNER
ping localhost -n 1 >nul
echo Sam's NUMBER GUESSING GAME!
SAM "Sam's NUMBER GUESSING GAME!"
ping localhost -n 1 >nul
echo.
ping localhost -n 1 >nul
echo.
ping localhost -n 1 >nul
echo.
ping localhost -n 1 >nul
echo It's easy to play!
SAM "It's easy to play!"
ping localhost -n 1 >nul
echo.
ping localhost -n 1 >nul
echo The computer generates a number between 1 and 100, and you try to guess it!
SAM "The computer generates a number between 1 and one hundred, and you try to guess it!"
ping localhost -n 1 >nul
echo.
ping localhost -n 1 >nul
echo.
ping localhost -n 1 >nul
echo.
echo "To begin the game, type your name and press Enter":
SAM "To begin the game, type your name and press Enter":
set /p web=
:HOME
cls
echo Well %web% Now GUESS THE NUMBER!
SAM Well
SAM %web%
SAM "Now GUESS THE NUMBER!"
echo.
echo.
set /p guess=
echo
if %guess% LSS %number% goto LOWER
if %guess% GTR %number% goto HIGHER
if %guess% EQU %number% goto WINNER
:LOWER
cls
echo Your guess is lower than the number!
SAM "Your guess is lower than the number!"
echo.
echo.
pause
goto HOME
:HIGHER
cls
echo Your guess is higher than the number!
SAM "Your guess is higher than the number!"
echo.
echo.
pause
goto HOME
:WINNER
cls
echo.
SAM -speed 55 -pitch 92 -throat 128 -mouth 128 "Ha ha ha ha ha ha ha? Ha ha ha ha ha ha ha!"
echo  Congratulations %web%! You guessed the number correctly!
SAM "Congratulations"
SAM %web%!
SAM "You guessed the number correctly!"
echo.
echo.
echo The number was %number%
SAM "The number was"
SAM %number%
echo Thanks for playing!
SAM "Thanks for playing?"
SAM %web%!
SAM "Thanks for playing."
ping localhost -n 2 >nul
exit
