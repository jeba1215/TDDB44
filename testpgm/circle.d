PROGRAM Cirkel;

  CONST Pi = 3.14159;

  VAR o : REAL; 
      r : REAL;

#include "stdio.d"      

  PROCEDURE Init;
    BEGIN
      r := 17;
    END;

  FUNCTION Omkrets(Radie : REAL) : REAL;

    FUNCTION Diameter: REAL;    
      BEGIN
      //write_real(RADIE);
	RETURN 2 * Radie;
      END;

  BEGIN
    RETURN Diameter() * Pi;
  END;

BEGIN
  Init();
  o := Omkrets(r);
  write_real(o);
END.
