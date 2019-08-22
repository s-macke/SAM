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
ADDRESS COMMAND; "/SAM" "The number was" getNumberString(Number); ADDRESS REXX
ECHO "Thanks for playing " Name ",thanks for playing!"
ADDRESS COMMAND; "/SAM" "Thanks for playing?" Name "Thanks for playing."; ADDRESS REXX
DELAY(100)
EXIT


cls: PROCEDURE
ECHO "1B"x"[0;0H" "1B"x"[J"
RETURN 0


getNumberString: PROCEDURE
  ARG number

  SELECT
    WHEN number < 10 THEN return number

    WHEN number = 10 THEN return "ten"
    WHEN number = 11 THEN return "eleven"
    WHEN number = 12 THEN return "twelve"
    WHEN number = 13 THEN return "thirteen"
    WHEN number = 14 THEN return "fourteen"
    WHEN number = 15 THEN return "fifteen"
    WHEN number = 16 THEN return "sixteen"
    WHEN number = 17 THEN return "seventeen"
    WHEN number = 18 THEN return "eighteen"
    WHEN number = 19 THEN return "9teen"

    WHEN number = 20 THEN return "twenty"
    WHEN number < 30 THEN return "twenty" || number//20

    WHEN number = 30 THEN return "thirty"
    WHEN number < 40 THEN return "thirty" || number//30

    WHEN number = 40 THEN return "fourty"
    WHEN number < 50 THEN return "fourty" || number//40

    WHEN number = 50 THEN return "fifty"
    WHEN number < 60 THEN return "fifty"  || number//50

    WHEN number = 60 THEN return "sixty"
    WHEN number < 70 THEN return "sixty"  || number//60

    WHEN number = 70 THEN return "seventy"
    WHEN number < 80 THEN return "seventy"  || number//70

    WHEN number = 80 THEN return "eighty"
    WHEN number < 90 THEN return "eighty"  || number//80

    WHEN number = 90 THEN return "9tee"
    WHEN number <100 THEN return "9tee"  || number//90
  END


