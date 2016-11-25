program parstest1;

var
    arr : array[10] of integer;
    i : integer;
    y : integer;

function echo(i:integer) : integer;
begin;
    return i;
end;

begin
    i := echo(10289);
    y := 1;
    y := y div y;
    arr[1] := i;
end.