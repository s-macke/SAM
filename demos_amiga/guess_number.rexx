/* /start with "rx guess_number.rexx" */ 
/* Adapted by AF, 14.Dec.2018 */

ADDLIB("rexxsupport.library",0, -30, 0)

/* RANDOM() always delivers the same sequence, seems a varying seed is needed. */
Seed=TIME(SECONDS)
SAY "Seed="Seed

MAIN: /*:MAIN*/
cls()
number=RANDOM(1,100,Seed)
/* rem goto WINNER */
DELAY(50)
ECHO "Sam's NUMBER GUESSING GAME!"
ADDRESS COMMAND; "/SAM" "Sam's NUMBER GUESSING GAME!"; ADDRESS REXX
DELAY(50)
ECHO 
DELAY(50)
ECHO 
DELAY(50)
ECHO 
DELAY(50)
ECHO "It's easy to play!"
ADDRESS COMMAND; "/SAM" "It's easy to play!"; ADDRESS REXX
DELAY(50)
ECHO
DELAY(50)
ECHO "The computer generates a number between 1 and 100, and you try to guess it!"
ADDRESS COMMAND; "/SAM" "The computer generates a number between 1 and 1 hundred, and you try to guess it!"; ADDRESS REXX
DELAY(50)
ECHO
DELAY(50)
ECHO
DELAY(50)
ECHO
ECHO "To begin the game, type your name and press Enter"
ADDRESS COMMAND; "/SAM" "To begin the game, type your name and press Enter"; ADDRESS REXX
PULL Name
HOME:
cls()
ECHO "Well" Name "Now GUESS THE NUMBER!"
ADDRESS COMMAND; "/SAM" "Well " Name ", now GUESS THE NUMBER!"; ADDRESS REXX
ECHO
ECHO
PULL guess
ECHO

SELECT
   WHEN guess < number THEN SIGNAL LOWER
   WHEN guess > number THEN SIGNAL HIGHER
   WHEN guess = number THEN SIGNAL WINNER
END

LOWER:
cls()
ECHO "Your guess is lower than the number!"
ADDRESS COMMAND; "/SAM" "Your guess is lower than the number!"; ADDRESS REXX
ECHO
ECHO
pause
SIGNAL HOME

HIGHER:
cls()
ECHO "Your guess is higher than the number!"
ADDRESS COMMAND; "/SAM" "Your guess is higher than the number!"; ADDRESS REXX
ECHO
ECHO
SIGNAL HOME

WINNER:
cls()
ECHO
ADDRESS COMMAND; "/SAM" "-speed 55 -pitch 92 -throat 128 -mouth 128 " "Ha ha ha ha ha ha ha? Ha ha ha ha ha ha ha!"; ADDRESS REXX
ECHO "Congratulations " Name "! You guessed the number correctly!"
ADDRESS COMMAND; "/SAM" "Congratulations" Name "!"; ADDRESS REXX
ADDRESS COMMAND; "/SAM" "You guessed the number correctly!"; ADDRESS REXX
ECHO
ECHO
ECHO "The number was" number
ADDRESS COMMAND; "/SAM" "The number was" Number; ADDRESS REXX
ECHO "Thanks for playing " Name ",thanks for playing!"
ADDRESS COMMAND; "/SAM" "Thanks for playing?" Name "Thanks for playing."; ADDRESS REXX
DELAY(100)
EXIT


cls: PROCEDURE
ECHO "1B"x"[0;0H" "1B"x"[J"
RETURN 0

