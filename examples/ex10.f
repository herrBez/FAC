
if(true){
    skip;
} else {
    skip;
}


fract f;
f = [1|3];
if(false -> false){
    f = f + f;
    print(f);
} else {
    f = f - f;
    print(f);
}

if(false || [1|3] < [2|3]){
    f = [1|3];
} else {
    skip;
}

if((f < [1|3]) && (f > [1|3])){
    f = [1|3];
    print(f);
    print(f);
} else {
    print(f);
}
