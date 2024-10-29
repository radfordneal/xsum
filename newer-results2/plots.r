# FUNCTIONS FOR READING TIMING DATA AND CREATING PLOTS.
#
# Run with:
#
#   R --vanilla <plots.r

plot_key <- function ()
{
  pdf("key.pdf",height=0.7,width=9.5,pointsize=11)
  par(mar=c(0.35,0.35,0.35,0.35))
  v <- 0.5
  plot(0,xlim=c(0,10.4),ylim=c(-0.1,v+0.1),
       type="n",xaxt="n",yaxt="n",bty="n",xlab="",ylab="")

  lines(c(0,1),c(v,v),type="b",pch=20,col="red",lty=1)
  text(1.2,v,adj=c(0,0.5),cex=1.25,"Small superaccumulator")

  lines(c(0,1),c(0,0),type="b",pch=20,col="red",lty=2)
  text(1.2,0,adj=c(0,0.5),cex=1.25,"Large superaccumulator")

  lines(c(4.1,5.1),c(v,v),type="b",pch=20,col="blue",lty=1)
  text(5.3,v,adj=c(0,0.5),cex=1.25,"iFastSum")

  lines(c(4.1,5.1),c(0,0),type="b",pch=20,col="blue",lty=2)
  text(5.3,0,adj=c(0,0.5),cex=1.25,"OnlineExact")

  lines(c(7,8),c(v,v),type="b",pch=20,col="black",lty=1)
  text(8.2,v,adj=c(0,0.5),cex=1.25,"Simple sum, ordered")

  lines(c(7,8),c(0,0),type="b",pch=20,col="black",lty=2)
  text(8.2,0,adj=c(0,0.5),cex=1.25,"Simple sum, not ordered")

  dev.off()
}

plot_times <- function (y, what, cache, 
                        col=c("red","red","blue","blue","black","black"),
                        lty=c(1,2,1,2,1,2), add=FALSE)
{
  logy <- log(y,2)

  if (!add)
  { #pdf(paste0("./",what,".pdf"),height=4.28,width=5,pointsize=11)
    par(mar=c(4.0,3.3,1.2,0.2),mgp=c(1.4,0.6,0),lab=c(7,7,7),cex=0.7)
    plot(1,type="n",xlab="number of terms",ylab="nanoseconds per term",
         xlim=c(1,7),ylim=c(-2.1,6.5),xaxt="n",yaxt="n")
    abline(h=seq(-1,7),col="gray")
    mtext (2^((-2):6), side=2, line=0.3, at=(-2):6, cex=0.5)
    mtext (1:7, side=1, line=0.05, at=(1:7)+0.1, cex=0.4)
    mtext (rep(10,7), side=1, line=0.55, at=(1:7)-0.04, cex=0.5)
    abline(col="gray",v=log10(cache))
  }

  for (j in 1:ncol(y))
  { lines (1:7, logy[,j], col=col[j], lty=lty[j])
    points (1:7, logy[,j], col=col[j], pch=20)
  }

  mtext(paste("         ",system_name),outer=TRUE,line=-1.2,cex=0.8)
}

read_times <- function (file)
{
  c <- pipe (paste (
   'fgrep -v Kahan <', file, '| fgrep -v "Float 128"', '| fgrep ns/term',
   '| sed "s/.*s,//" | sed "s/ns.*//"'
  ))
  d <- scan(c)
  close(c)
  stopifnot(length(d)==98)
  if (file != "results-pi-gcc-4.6.3")
  { d[d<0] <- d[d<0] + 2^32/1e6  # clock wrapped around
  }
  else
  { d[d<0] <- d[d<0] + 2^32/1e5  # did fewer repeats on the pi
  }
  m <- matrix(d,ncol=7)
  n <- matrix(NA,ncol=7,nrow=2)
  m <- rbind(m[1:8,],n,m[9:12,],n,m[13:14,])
  m <- t(m)
  dim(m) <- c(7,6,3)
  dimnames(m) <- list(NULL,c("sacc","lacc","ifast","online","dbl","udbl"),
                           c("sum","norm","dot"))
  m
}

find_min_times <- function (...,
                    ref = c(sacc=3, lacc=5, ifast=3, online=5, dbl=4, udbl=4))
{
  times <- list(...)
  min_times <- times[[1]]
  wh <- matrix (names(times)[[1]], 6, 3)
  rownames(wh) <- dimnames(min_times)[[2]]
  colnames(wh) <- dimnames(min_times)[[3]]

  for (ni in 2:length(times))
  { nxt <- times[[ni]]
    for (k in 1:dim(nxt)[3])
    { w <- !is.na(nxt[1,,k])
      dn <- dimnames(nxt)[[2]]
      for (j in (1:dim(nxt)[2])[w])
      { if (nxt[ref[dn[j]],j,k] < min_times[ref[dn[j]],j,k])
        { min_times[,j,k] <- nxt[,j,k]
          wh[j,k] <- names(times)[[ni]]
        }
      }
    }
  }

  cat("Compilations with minimum for 1000 terms:\n")
  print (wh)

  min_times
}


# CREATE PLOTS FOR VARIOUS NEWER SYSTEMS.

plot_key()


# AMD THREADRIPPER PRO 3945WX.

system_name <- "thinkstation"
pdf(paste0(system_name,".pdf"),height=9,width=6)
par(mfcol=c(3,2))

cache = c(32*1024, 512*1024, 16*1024*1024)

thinkstation_results <- list (
  thinkstation_gcc_9_5_0 = read_times ("results-thinkstation-gcc-9.5.0"),
  thinkstation_gcc_10_5_0 = read_times ("results-thinkstation-gcc-10.5.0"),
  thinkstation_gcc_11_4_0 = read_times ("results-thinkstation-gcc-11.4.0"),
  thinkstation_gcc_12_3_0 = read_times ("results-thinkstation-gcc-12.3.0"),
  thinkstation_clang_11_1_0 = read_times ("results-thinkstation-clang-11.1.0"),
  thinkstation_clang_12_0_1 = read_times ("results-thinkstation-clang-12.0.1"),
  thinkstation_clang_13_0_1 = read_times ("results-thinkstation-clang-13.0.1"),
  thinkstation_clang_14_0_0 = read_times ("results-thinkstation-clang-14.0.0"),
  thinkstation_clang_15_0_7 = read_times ("results-thinkstation-clang-15.0.7")
)

thinkstation <- eval (as.call (c (find_min_times, thinkstation_results)))

for (task in c("sum","norm","dot"))
{
  plot_times (thinkstation[,,task],  
              paste0("thinkstation-",task), 
              cache / c(sum=8,norm=8,dot=16)[task])
  title (task)
  for (dat in thinkstation_results)
  { plot_times(dat[,,task], add=TRUE,
               col=c("pink","pink","lightblue","lightblue","gray","gray"))
  }
  plot_times(thinkstation[,,task], add=TRUE)
  # dev.off()
}

thinkstation_no_opt_results <- list (
  thinkstation_gcc_9_5_0 =
     read_times ("results-thinkstation-gcc-9.5.0-no-opt"),
  thinkstation_gcc_10_5_0 =
     read_times ("results-thinkstation-gcc-10.5.0-no-opt"),
  thinkstation_gcc_11_4_0 =
     read_times ("results-thinkstation-gcc-11.4.0-no-opt"),
  thinkstation_gcc_12_3_0 =
     read_times ("results-thinkstation-gcc-12.3.0-no-opt"),
  thinkstation_clang_11_1_0 =
     read_times ("results-thinkstation-clang-11.1.0-no-opt"),
  thinkstation_clang_12_0_1 =
     read_times ("results-thinkstation-clang-12.0.1-no-opt"),
  thinkstation_clang_13_0_1 =
     read_times ("results-thinkstation-clang-13.0.1-no-opt"),
  thinkstation_clang_14_0_0 =
     read_times ("results-thinkstation-clang-14.0.0-no-opt"),
  thinkstation_clang_15_0_7 =
     read_times ("results-thinkstation-clang-15.0.7-no-opt")
)

thinkstation_no_opt <- 
  eval (as.call (c (find_min_times, thinkstation_no_opt_results)))

for (task in c("sum","norm","dot"))
{
  plot_times (thinkstation_no_opt[,,task],  
              paste0("thinkstation-no-opt-",task), 
              cache / c(sum=8,norm=8,dot=16)[task])
  title (paste(task,"no-opt"))
  for (dat in thinkstation_no_opt_results)
  { plot_times(dat[,,task], add=TRUE,
               col=c("pink","pink","lightblue","lightblue","gray","gray"))
  }
  plot_times(thinkstation_no_opt[,,task], add=TRUE)
  # dev.off()
}

dev.off()


# INTEL W-2275.

system_name <- "T5820"
pdf(paste0(system_name,".pdf"),height=9,width=6)
par(mfcol=c(3,2))

cache = c(32*1024, 1024*1024, 19*1024*1024)

T5820_results <- list (
  T5820_gcc_9_5_0 = read_times ("results-T5820-gcc-9.5.0"),
  T5820_gcc_10_5_0 = read_times ("results-T5820-gcc-10.5.0"),
  T5820_gcc_11_4_0 = read_times ("results-T5820-gcc-11.4.0"),
  T5820_gcc_12_3_0 = read_times ("results-T5820-gcc-12.3.0"),
  T5820_clang_11_1_0 = read_times ("results-T5820-clang-11.1.0"),
  T5820_clang_12_0_1 = read_times ("results-T5820-clang-12.0.1"),
  T5820_clang_13_0_1 = read_times ("results-T5820-clang-13.0.1"),
  T5820_clang_14_0_0 = read_times ("results-T5820-clang-14.0.0"),
  T5820_clang_15_0_7 = read_times ("results-T5820-clang-15.0.7")
)

T5820 <- eval (as.call (c (find_min_times, T5820_results)))

for (task in c("sum","norm","dot"))
{
  plot_times (T5820[,,task],  
              paste0("T5820-",task), 
              cache / c(sum=8,norm=8,dot=16)[task])
  title (task)
  for (dat in T5820_results)
  { plot_times(dat[,,task], add=TRUE,
               col=c("pink","pink","lightblue","lightblue","gray","gray"))
  }
  plot_times(T5820[,,task], add=TRUE)
  # dev.off()
}

T5820_no_opt_results <- list (
  T5820_gcc_9_5_0 =
     read_times ("results-T5820-gcc-9.5.0-no-opt"),
  T5820_gcc_10_5_0 =
     read_times ("results-T5820-gcc-10.5.0-no-opt"),
  T5820_gcc_11_4_0 =
     read_times ("results-T5820-gcc-11.4.0-no-opt"),
  T5820_gcc_12_3_0 =
     read_times ("results-T5820-gcc-12.3.0-no-opt"),
  T5820_clang_11_1_0 =
     read_times ("results-T5820-clang-11.1.0-no-opt"),
  T5820_clang_12_0_1 =
     read_times ("results-T5820-clang-12.0.1-no-opt"),
  T5820_clang_13_0_1 =
     read_times ("results-T5820-clang-13.0.1-no-opt"),
  T5820_clang_14_0_0 =
     read_times ("results-T5820-clang-14.0.0-no-opt"),
  T5820_clang_15_0_7 =
     read_times ("results-T5820-clang-15.0.7-no-opt")
)

T5820_no_opt <- 
  eval (as.call (c (find_min_times, T5820_no_opt_results)))

for (task in c("sum","norm","dot"))
{
  plot_times (T5820_no_opt[,,task],  
              paste0("T5820-no-opt-",task), 
              cache / c(sum=8,norm=8,dot=16)[task])
  title (paste(task,"no-opt"))
  for (dat in T5820_no_opt_results)
  { plot_times(dat[,,task], add=TRUE,
               col=c("pink","pink","lightblue","lightblue","gray","gray"))
  }
  plot_times(T5820_no_opt[,,task], add=TRUE)
  # dev.off()
}

dev.off()
