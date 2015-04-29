nbR <- 64
exp <- "gcNav"
maxIt <- 0
for(i in 1:nbR)
{
 name <- sprintf(paste(paste("%03d-evo-",exp,sep=""),"Tune.log",sep=""),i)
 aux <- read.table(name)
 if(max(aux[,1]) > maxIt)
 {
   maxIt <- max(aux[,1])
 }
}
temp = list.files(pattern=paste(paste("*-evo-",exp,sep=""),"*",sep=""))
named.list <- lapply(temp, read.table)
library(data.table)
data <-rbindlist(named.list)


measurePeriod <- 100 #min(data[which(data[,1] !=  min(data[,1])),1]) - min(data[,1])

timeback <- 0.08 #TODO Read

nbRob <- 100#length(which(data[,1] == measurePeriod, arr.ind=TRUE))

nbMeasures <- floor(maxIt/measurePeriod)

backNbMeasures <- floor(nbMeasures * timeback)

considered <- (nbMeasures-backNbMeasures+1):nbMeasures


#dev.new()
pdf("plots.pdf")
for(i in 1:nbR)
{
dataAux <- named.list[[i]][8] + named.list[[i]][9]
meanFitness <- matrix(0.0,nbMeasures,1)
j <- 0
while(j < nbMeasures)
{
	#or median?
	meanFitness[j+1,1] <- mean(dataAux[(j * nbRob + 1) :(j * nbRob + nbRob),1])
	j <- j +1
}
if(i == 1)
{
	plot(meanFitness, type ='l', ylim=c(0,200))
}
else
	lines(meanFitness)
	
row.names(meanFitness) <- rep('[sizeNN:',length(meanFitness))
	write.table(meanFitness,paste(sprintf(paste("%03d-meanFitPerRobot-",exp,sep=""),i),".sizeNN",sep=""),row.names= TRUE,col.names=FALSE,sep="",quote=FALSE)
}

dev.off()

q()

