program test;

var
    x : real;

#include "stdio.d"
#include "math.d"

begin
    x := read_real();
    write_real(sqrt(x));
    write_real(x);
    newline();
end.
    