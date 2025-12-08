import Random;
use LinearAlgebra;

proc main(){
    const  size : int = 2976;
    var  M0: [0..size, 0..size] real;
    var  M1: [0..size, 0..size] real;

    // whole matrices have these values now
    M0 = 6.987;
    M1 = 4.210;

    var M3 = dot(M0, M1);
    writeln(M3);
}