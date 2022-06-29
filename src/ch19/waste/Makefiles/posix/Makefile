OBJS = asyncdns.o config.o connection.o filedb.o listen.o m_chat.o m_file.o m_keydist.o m_ping.o m_search.o m_upload.o mqueue.o mqueuelist.o netkern.o sha.o util.o xfers.o xferwnd.o srchwnd.o srvmain.o blowfish.o m_lcaps.o


RSAOBJS = md5c.o nn.o prime.o r_random.o rsa.o 

DEBUGFLAG = -s
CXXFLAGS = -O2 $(DEBUGFLAG) -pipe -march=pentiumpro
CFLAGS = -O2 $(DEBUGFLAG) -pipe -march=pentiumpro


wastesrv: $(OBJS) $(RSAOBJS)
	$(CC) $(DEBUGFLAG) -pthread -o wastesrv $(OBJS) $(RSAOBJS)

md5c.o: rsa/md5c.c
nn.o: rsa/nn.c
prime.o: rsa/prime.c
r_random.o: rsa/r_random.c
rsa.o: rsa/rsa.c

clean:
	-rm *.o wastesrv
