#R --slave --args filename lastEpoch nbEpochBackwards < ../scripts/accumMeanEnd.r
#R --slave --args filename < ../scripts/accumMeanEnd.r
library('matrixStats',quietly=TRUE);
# read arguments 
args <- commandArgs(trailingOnly = TRUE);

if(length(args) < 2){
    write("Error in arguments, (needs a file name and a percentage of time to average");
	#, total number of epochs and number of  backwards epochs)", "");
    q();
}
lastEpoch <- 500;
backEpochs <- 40;

data <- read.table(args[1]);

#if(!is.na(as.numeric(args[2])))
#{
#	lastEpoch <- as.numeric(args[2]);
#}

#if(!is.na(as.numeric(args[3])))
#{
#	backEpochs <- as.numeric(args[3]);
#}



if(!is.na(as.numeric(args[2])))
{
	timeBack <- as.numeric(args[2]);
}

lastEpoch <- max(data[1])

backEpochs <- lastEpoch * timeBack

considered <- (lastEpoch-backEpochs+1):lastEpoch

meanItemEpoch <- matrix(0.0,max(data[,1]),1)

for(i in 0:data[dim(data)[1],1])
	{
	 if(length(data[which(data[,1] == i, arr.ind=TRUE),3]) > 0)
		{
			 meanItemEpoch[i,1] <- sum(data[which(data[,1]==i,arr.ind=TRUE),3])
		}
	 else
		 meanItemEpoch[i,1] <- 0
 	}


result <- mean(meanItemEpoch[considered,1])
sResult <-toString(result)
cat(sResult)
cat('\n')



