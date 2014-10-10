gaussian <- function(n){
	result <- vector();	
	for (i in 1:n) 
	{
		v1 <- 2 * runif(1);
		v2 <- 2 * runif(1);
		rsq <- v1*v1 + v2*v2;
		while((rsq >= 1.0) | (rsq == 0.0)) {
			v1 <- 2 * runif(1);
			v2 <- 2 * runif(1);
		 	rsq <- v1*v1 + v2*v2
	 	}
	 fac<-sqrt(-2.0*log(rsq)/rsq);
	result <- c(result, (v2*fac))
	}	
	return(result)
}

vectorDensity <- function(x,discSteps=20){
res <- vector()
maximal <-max(x)
minimal <- 0.0
for(i in 0:(discSteps-1))
	res <- c(res, sum((x >= (i*	(maximal/discSteps))) & (x < ((i+1)	*(maximal/discSteps)))))

return(res)
}



