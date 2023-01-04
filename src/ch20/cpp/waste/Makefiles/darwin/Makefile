OBJS = asyncdns.o config.o connection.o filedb.o listen.o m_chat.o m_file.o m_keydist.o m_ping.o m_search.o m_upload.o mqueue.o mqueuelist.o netkern.o sha.o util.o xfers.o xferwnd.o srchwnd.o srvmain.o blowfish.o m_lcaps.o


RSAOBJS = md5c.o nn.o prime.o r_random.o rsa.o 

DEBUGFLAG = 
CXXFLAGS = -O2 $(DEBUGFLAG) -pipe  -Irsa -DBIG_ENDIAN
CC=gcc
CFLAGS = -O2 $(DEBUGFLAG) -pipe  -Irsa -DBIG_ENDIAN

default: wastesrv

md5c.o: rsa/md5c.c
	$(CC) $(CFLAGS) -c -o md5c.o rsa/md5c.c
nn.o: rsa/nn.c
	$(CC) $(CFLAGS) -c -o nn.o rsa/nn.c
prime.o: rsa/prime.c
	$(CC) $(CFLAGS) -c -o prime.o rsa/prime.c
r_random.o: rsa/r_random.c
	$(CC) $(CFLAGS) -c -o r_random.o rsa/r_random.c
rsa.o: rsa/rsa.c
	$(CC) $(CFLAGS) -c -o rsa.o rsa/rsa.c

wastesrv: $(OBJS) $(RSAOBJS)
	$(CXX) $(DEBUGFLAG) -o wastesrv $(OBJS) $(RSAOBJS)


clean:
	-rm *.o wastesrv
