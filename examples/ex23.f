/* example: nested if */
fract f = [0|1];
if([1|3]<[2|3]){
    f = [1|3];
	if(true){
	    f= f - [1|3];
	    if([1|3]<=[2|3]){
	    	f= f + [1|3];
    	}
    }
}

print(f);