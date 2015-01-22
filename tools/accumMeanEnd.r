#R --slave --args filename < ../scripts/accumMeanEnd.r
library('matrixStats',quietly=TRUE);
# read arguments 
args <- commandArgs(trailingOnly = TRUE);

if(length(args) < 2){
    write("Error in arguments, (needs a file name and a percentage of time to average");
	#, total number of epochs and number of  backwards epochs)", "");
    q();
}
lastIter <- 500;
backIter <- 40;

data <- read.table(args[1]);

timeback <- 0.8
if(!is.na(as.numeric(args[2])))
{
	timeback <- as.numeric(args[2]);
}

lastIter <- max(data[1]) 


measurePeriod <- min(data[which(data[,1] !=  min(data[,1])),1]) - min(data[,1])


nbR <- length(which(data[,1] == measurePeriod, arr.ind=TRUE))

nbMeasures <- floor(lastIter/measurePeriod)

backNbMeasures <- floor(nbMeasures * timeback)

considered <- (nbMeasures-backNbMeasures+1):nbMeasures


meanFitness <- matrix(0.0,nbMeasures,1)

i <- 0
while(i < nbMeasures)
{
	#or median?
	meanFitness[i+1,1] <- mean(data[(i * nbR + 1) :(i * nbR + nbR),5])
	i <- i +1
}


result <- mean(meanFitness[considered,1])

sResult <-toString(result)
cat(sResult)
cat('\n')



