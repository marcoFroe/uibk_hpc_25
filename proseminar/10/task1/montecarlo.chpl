use Random;


proc main(){
    const n = 1_000_000_000;
    var inside_counter:int = 0;


    forall i in 1..n with (+ reduce inside_counter){
        var rand = new Random.randomStream(real);
        
        var x:real = rand.next();
        var y:real = rand.next();
    
        if ((x*x)+(y*y)<=1.0){
            inside_counter += 1;
        }
    }

    var pi: real = (inside_counter:real / n) * 4.0;
    writeln(pi);
}