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


if(!is.na(as.numeric(args[2])))
{
	timeback <- as.numeric(args[2]);
}

lastIter <- max(data[1])

backIters <- floor(lastIter * timeback)

considered <- (lastIter-backIters+1):lastIter

meanFitness <- matrix(0.0,max(data[,1]),1)

nbR <- length(which(data[,1] == 1, arr.ind=TRUE))
for(i in (lastIter-backIters+1):lastIter)
     meanFitness[i,1] <- mean(data[(i * 100 + 1) :(i * 100 + 100),5])


result <- mean(meanFitness[considered,1])

sResult <-toString(result)
cat(sResult)
cat('\n')



